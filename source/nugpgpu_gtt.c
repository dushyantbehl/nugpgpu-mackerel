#include <linux/printk.h>
#include <linux/pci.h>
#include <linux/gfp.h>
#include <asm/cacheflush.h>
#include <linux/io-mapping.h>

#include "nugpgpu_gtt.h"
#include "nugpgpu_drv.h"
#include "nugpgpu_dbg.h"
#include "../generated/nugpgpu_dev.h"

static unsigned int gtt_entry = 0;

static int scratchpage_setup(struct nugpgpu_private *gpu_priv);

static inline nugpgpu_gtt_pte_t nugpgpu_pte_encode(dma_addr_t addr,
                                                   uint32_t level,
                                                   bool valid)
{
  nugpgpu_gtt_pte_t pte = valid ? PTE_VALID : 0;
  pte |= PTE_ADDR_ENCODE(addr,valid);

  if (level != 0)
    pte |= HSW_CACHEABILITY_CONTROL(0x2);

  return pte;
}

int nugpgpu_gtt_init(struct nugpgpu_private *gpu_priv)
{
  struct gpu_gtt *gtt = &gpu_priv->gtt;
  int ret;
  uint32_t apperture_size;

  printk(LOG_INFO "nugpgpu_gtt_init\n" LOG_END);
  TRACE_IN

  ret = nugpgpu_gtt_probe(gpu_priv, &gtt->total_size, &gtt->stolen_size,
          &gtt->mappable_base, &gtt->mappable_end);
  if(ret)
  {
    printk(LOG_ERR "Failed to probe GTT\n" LOG_END);
  }
  
  apperture_size = gpu_priv->gtt.mappable_end;

  gpu_priv->gtt.mappable = 
                io_mapping_create_wc(gpu_priv->gtt.mappable_base, apperture_size);
  if (gpu_priv->gtt.mappable == NULL)
      printk(LOG_ERR "gtt mappable still null\n" LOG_END);

  TRACE_OUT
  return ret;
}

int nugpgpu_gtt_probe(struct nugpgpu_private *gpu_priv, size_t *total_size,
                  size_t *stolen_size, phys_addr_t *mappable_base,
                  unsigned long *mappable_end)
{
  int ret;
  unsigned int gtt_size;
  phys_addr_t gtt_phys_addr;
  u16 stolen_gmch, total_gmch, hsw_gmch_ctl;

  printk(LOG_INFO "nugpgpu_gtt_probe\n" LOG_END);
  TRACE_IN

  *mappable_base = pci_resource_start(gpu_priv->pdev, 2);
  printk(LOG_INFO "*mappable_base = 0x%lx\n" LOG_END, (unsigned long) *mappable_base);

  *mappable_end = pci_resource_len(gpu_priv->pdev, 2);
  printk(LOG_INFO "*mappable_end = 0x%lx\n" LOG_END, *mappable_end);

  if( (*mappable_end < (64<<20) || (*mappable_end > (512<<20)))) {
      printk(LOG_INFO "sanity check in gtt_probe failed\n" LOG_END);
  }

  // TODO : This fails. Also in the driver.
  if(!pci_set_dma_mask(gpu_priv->pdev, DMA_BIT_MASK(40)))
  {
    printk(LOG_INFO "pci_set_dma_mask failed, trying consistent\n" LOG_END);
    pci_set_consistent_dma_mask(gpu_priv->pdev, DMA_BIT_MASK(40));
  }

  // Read size. GSA_CR_MGGC0_0_2_0_PCI (0x50)
  pci_read_config_word(gpu_priv->pdev, nugpgpu_HSW_GMCH_CTRL, &hsw_gmch_ctl);
  printk(LOG_INFO "GSA_CR_MGGC0_2_0_PCI = 0x%hx\n" LOG_END, hsw_gmch_ctl);

  // Stolen Size / GTT Graphics Memory Size / GGMS (9:8)
  stolen_gmch = hsw_gmch_ctl >> nugpgpu_HSW_GMCH_GGMS_SHIFT;
  stolen_gmch &= nugpgpu_HSW_GMCH_GGMS_MASK;
  *stolen_size = stolen_gmch << nugpgpu_HSW_GMCH_GGMS_REVSHIFT;
  printk(LOG_INFO "*stolen_size = 0x%x\n" LOG_END, (unsigned int) *stolen_size);

  // Total GTT Size / Graphics Mode Select / GMS (7:3)
  // Amount of main memory that is pre-allocated to support the graphics device
  total_gmch = hsw_gmch_ctl >> nugpgpu_HSW_GMCH_GGMS_SHIFT;
  total_gmch &= nugpgpu_HSW_GMCH_GGMS_MASK;
  gtt_size = total_gmch << nugpgpu_HSW_GMCH_GGMS_REVSHIFT;

  printk(LOG_INFO "gtt_size = 0x%x\n" LOG_END, gtt_size);
  
  *total_size = (gtt_size / sizeof(nugpgpu_gtt_pte_t)) << PAGE_SHIFT;
  printk(LOG_INFO "*total_size = 0x%x\n" LOG_END, (unsigned int) *total_size);

  // 2MB MMIO + 2MB GTT aperture
  gtt_phys_addr = pci_resource_start(gpu_priv->pdev, 0) +
                                    (pci_resource_len(gpu_priv->pdev, 0) / 2);
  printk(LOG_INFO "*gtt_phys_addr = %x\n" LOG_END, (unsigned int) gtt_phys_addr);

  // Write-combined mapping. Faster sequential update & stuff.
  // This is a BAR. We go through it so that GPU's TLBs get synchronized.
  gpu_priv->gtt.gsm = ioremap_wc(gtt_phys_addr, gtt_size);
  printk(LOG_INFO "gsm = %lx\n" LOG_END, (unsigned long) gpu_priv->gtt.gsm);

  ret = scratchpage_setup(gpu_priv);
  if(ret)
  {
    printk(LOG_ERR "scratchpage setup failed\n" LOG_END);
    iounmap(gpu_priv->gtt.gsm);
  }

  TRACE_OUT
  return ret;
}

/* If needed improve this function*/
uint32_t nugpgpu_gtt_insert(struct nugpgpu_private *gpu_priv,
                            struct sg_table *st,
                            uint32_t level)
{
  uint32_t gfx_addr = gtt_entry * PAGE_SIZE;
  printk(LOG_INFO "nugpgpu_gtt_insert\n" LOG_END);
  TRACE_IN
  gtt_entry = nugpgpu_gtt_insert_entries(gpu_priv, st, &gtt_entry, level);
  TRACE_OUT
  return gfx_addr;
}

unsigned int nugpgpu_gtt_insert_entries(struct nugpgpu_private *gpu_priv,
                                        struct sg_table *st,
                                        unsigned int *entry,
                                        uint32_t level)
{
  unsigned int i = 0;
  struct sg_page_iter sg_iter;
  dma_addr_t addr;
  nugpgpu_gtt_pte_t pte;
  nugpgpu_gtt_pte_t __iomem *gtt_entries =
    (nugpgpu_gtt_pte_t __iomem *)gpu_priv->gtt.gsm + *entry;

  printk(LOG_INFO "nugpgpu_gtt_insert_entries\n" LOG_END);
  TRACE_IN

  for_each_sg_page(st->sgl, &sg_iter, st->nents, 0) {
    addr = sg_page_iter_dma_address(&sg_iter);
    pte = nugpgpu_pte_encode(addr, level, true);

    printk(LOG_INFO "gtt addr : 0x%lx, gtt pte : 0x%x, gtt writing at %u\n" LOG_END,
          (long unsigned int) addr, (unsigned int) pte, (unsigned int) i+*entry);

    iowrite32(pte, &gtt_entries[i]);
    i++;
  }

  printk(LOG_INFO "gtt write done. verifying.\n" LOG_END);

  if (i != 0)
    WARN_ON(readl(&gtt_entries[i-1]) !=
      nugpgpu_pte_encode(addr, level, true));

  //Flush the GPU TLB.
  NUGPGPU_WRITE(nugpgpu_GFX_FLSH_CNTL_HSW, nugpgpu_GFX_FLSH_CNTL_EN);
  NUGPGPU_READ(nugpgpu_GFX_FLSH_CNTL_HSW);
  
  TRACE_OUT
  return (i+(*entry));
}

static int scratchpage_setup(struct nugpgpu_private *gpu_priv)
{
  printk(LOG_INFO "scratchpage_setup\n" LOG_END);
  TRACE_IN

  gpu_priv->gtt.scratchpage.page = alloc_page(GFP_KERNEL | GFP_DMA32 | __GFP_ZERO);
  if(!gpu_priv->gtt.scratchpage.page)
  {
    printk(LOG_ERR "Failed to alloc_page.\n" LOG_END);
    TRACE_OUT
    return -ENOMEM;
  }

  get_page(gpu_priv->gtt.scratchpage.page);
  set_pages_uc(gpu_priv->gtt.scratchpage.page, 1);

  gpu_priv->gtt.scratchpage.addr = pci_map_page(gpu_priv->pdev,
                                                gpu_priv->gtt.scratchpage.page,
                                                0, PAGE_SIZE,
                                                PCI_DMA_BIDIRECTIONAL);
  if(pci_dma_mapping_error(gpu_priv->pdev, gpu_priv->gtt.scratchpage.addr))
  {
    printk(LOG_ERR "Failed to map DMA page.\n" LOG_END);
    TRACE_OUT
    return -EINVAL;
  }

  printk(LOG_INFO "PCI map page, scratch page successful\n" LOG_END);

  TRACE_OUT
  return 0;
}

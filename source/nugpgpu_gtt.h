#ifndef __NUGPGPU_GTT_H__
#define __NUGPGPU_GTT_H__

/* GRAPHICS TRANSLATION TABLE
 * https://bwidask.net/blog/index.php/2014/06/the-global-gtt-part-1/
 */

#include <linux/types.h>
#include <linux/scatterlist.h>

struct nugpgpu_private;

#define PTE_VALID 1

/* PTE Structure
 * 31:12  Physical Page Address 31:12
 * 11     Cacheability Control (WT/WB LLC/eLLC(eDRAM)) ([3] of [3..0])
 * 10:4   Physical Page Address 38:32
 * 2:0    Cacheability Control (WT/WB LLC/eLLC(eDRAM)) ([2..0] of [3..0])
 * 0      Valid
 */
#define PTE_ADDR_ENCODE(addr, valid) \
          ((valid ? PTE_VALID : 0) | ((addr) | (((addr) >> 28) & 0x7f0)))

/* Cacheability Control
 * 3:1    Three lower bits
 * 11     Fourth bit
 */
#define HSW_CACHEABILITY_CONTROL(bits) \
          ((((bits) & 0x7) << 1) | \
           (((bits) & 0x8) << (11 - 3)))

/*
 * GTT : GPU's VA -> PA
 * GTT logic in GPU, and not iommu.
 * Each element in GTT is a PTE.
 * Most of the initialization done by firmware.
 * The required information from initialization process can be obtained via PCI
 * config space, or MMIO. (We do it by PCI config space).
 * GPU mapping is direct, hardware based.
 * CPU needs to get the mapping, and then dereference & write.
 *
 * GSM : Graphics Stolen Memory, holds global PTEs
 * Located in system memory, allocated by BIOS, or boot firmware.
 * Base address from MPGFXTRK_CR_MBGSM_0_2_0_GTTMMADR (0x108100) (31:20)
 *
 * DSM : Stolen memory for everything that is not GTT.
 *
 * UNCORE_CR_GTTMMADR_0_2_0_PCI (0x10) (38:22)
 * Requests allocation for the combined GTT aperture and memory mapped range.
 * 4MB = 2MB MMIO + 2MB GTT aperture.
 * Base address of GTTADR := 2MB +  GTTMMADR
 * Base address of MMIO := GTTMMADR
 * Entire 4MB range is defined as a memory BAR in graphics device config space.
 * Alias into which the driver is required to write PTEs.
 * TODO : VERIFY: WHY: HOW: Driver can read PTE from GTT, but cannot write
 * directly to GTT. (To enable GPU TLB update?)
 *
 * Some region is mapped by the CPU too - GMADR (=aperture?)
 *
 * start : Start offset (0)
 *
 * total : Size of address space mapping (2GB)
 *
 * scratchpage : All unused PTEs point to this blank page
 */
struct gpu_gtt {
  unsigned long mappable_end;
  struct io_mapping *mappable;
  phys_addr_t mappable_base;

  unsigned long start;

  size_t stolen_size;
  size_t total_size;

  void __iomem *gsm;

  struct {
    dma_addr_t addr;
    struct page *page;
  } scratchpage;
};

int nugpgpu_gtt_init(struct nugpgpu_private *gpu_priv);
int nugpgpu_gtt_probe(struct nugpgpu_private *gpu_priv, size_t *total_size,
                      size_t *stolen_size, phys_addr_t *mappable_base,
                      unsigned long *mappable_end);
unsigned int nugpgpu_gtt_insert_entries(struct nugpgpu_private *gpu_priv,
                                        struct sg_table *st,
                                        unsigned int *entry,
                                        uint32_t level);
uint32_t nugpgpu_gtt_insert(struct nugpgpu_private *gpu_priv,
                            struct sg_table *st,
                            uint32_t level);


typedef uint32_t nugpgpu_gtt_pte_t;

#endif

#include <linux/pci.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/highmem.h>

#include "nugpgpu_tests.h"
#include "nugpgpu_dbg.h"

static void check_page_status(void *page)
{
  unsigned i;
  unsigned char *loc = (unsigned char *)page;
  printk(LOG_INFO "checking status page 0x%lx\n" LOG_END, (unsigned long) loc);
  for(i = 0; i < PAGE_SIZE; i++){
    if(loc[i]){
      printk(LOG_INFO "status[0x%x] = 0x%x\n" LOG_END, i, loc[i]);
    }
  }
}

void check_dword_hws(struct nugpgpu_private *gpu_priv, struct nugpgpu_ring *ring )
{
  u32 shift = 0x30;
  u32 emit_shift = shift << MI_STORE_DWORD_INDEX_SHIFT;
  u32 old_value = 0;
  int i=0;

  ring->page_status[shift] = 0x0a0a0a0a;

  old_value = ring->page_status[shift];
  printk(LOG_INFO "checking status page 0x%lx = 0x%x,0x%x\n" LOG_END, 
                  (unsigned long) ring->page_status, old_value,
                  (unsigned) ring->page_status[shift+1]);

  nugpgpu_dump(gpu_priv);

  check_page_status(ring->page_status);

  nugpgpu_ring_begin(ring, 4);
  printk(LOG_INFO "ring_begin done\n" LOG_END);
  ring_emit(ring, MI_STORE_DWORD_INDEX);
  ring_emit(ring, emit_shift);
  ring_emit(ring, 0xdeadbeef);
  ring_emit(ring, MI_NOOP);
  nugpgpu_ring_advance(gpu_priv, ring);
  printk(LOG_INFO "after ring advance\n" LOG_END);

  barrier();

  while ( i<100000 && ring->page_status[shift] == old_value ){
    if (i%10 == 0)
        printk(LOG_INFO "Still old value\n" LOG_END);
      i++;
  }

  old_value = ring->page_status[shift];
  printk(LOG_INFO "value after while loop is 0x%x\n" LOG_END, old_value);

  barrier();

  if (i<100000)
    printk(LOG_INFO "Thanks God\n" LOG_END);
  else
    printk(LOG_INFO "Help GOD\n" LOG_END);

  printk(LOG_INFO "checking status page 0x%lx = 0x%x,0x%x\n" LOG_END,
                   (unsigned long) ring->page_status,
                   (unsigned) ring->page_status[shift],
                   (unsigned) ring->page_status[shift+1]);

  nugpgpu_dump(gpu_priv);
  return;
}

void check_dword_temp(struct nugpgpu_private *gpu_priv, struct nugpgpu_ring *ring)
{
  u32 shift = 0x30;
  u32 emit_shift = shift << MI_STORE_DWORD_INDEX_SHIFT;
  u32 old_value = 0;
  int i=0,ret;

  struct nugpgpu_obj obj;
  uint32_t gfx_addr;
  uint32_t *virt_addr;

  TRACE_IN

  printk(LOG_INFO "check_dword_temp\n" LOG_END);

  /* Allocate a temp page. */
  ret = allocate_object(gpu_priv, &obj, 1);
  if (ret){
    printk(LOG_ERR "Failed to allocate temp page\n" LOG_END);
    return;
  }

  gfx_addr = nugpgpu_gtt_insert(gpu_priv, obj.pg_list, 
                                 nugpgpu_cache_llc);
  if ( gfx_addr == -1 ){
    printk(LOG_ERR "Failed to insert the temp page in gtt\n" LOG_END);
    return;
  }

  virt_addr = kmap(sg_page(obj.pg_list->sgl));
  if (virt_addr == NULL) {
    printk(LOG_ERR "Failed to map temp page\n" LOG_END);
    return;
  }
  memset(virt_addr, 0, PAGE_SIZE);
  printk(LOG_INFO "temp page : (gfx - 0x%x, virt_addr-0x%lx\n" LOG_END,
                   (unsigned) gfx_addr, (unsigned long) virt_addr);

  virt_addr[256] = MI_BATCH_BUFFER_END;


  ring->page_status[shift] = 0x0a0a0a0a;

  old_value = ring->page_status[shift];
  printk(LOG_INFO "checking status page 0x%lx = 0x%x,0x%x\n" LOG_END, 
                  (unsigned long) ring->page_status, old_value,
                  (unsigned) ring->page_status[shift+1]);
  nugpgpu_dump(gpu_priv);

  check_page_status(ring->page_status);

  nugpgpu_ring_begin(ring, 8);
  printk(LOG_INFO "ring_begin done\n" LOG_END);
  ring_emit(ring, MI_NOOP);
  ring_emit(ring, MI_NOOP);
  ring_emit(ring, MI_BATCH_BUFFER_START);
  ring_emit(ring, gfx_addr);
  ring_emit(ring, MI_NOOP);
  ring_emit(ring, MI_STORE_DWORD_INDEX);
  ring_emit(ring, emit_shift);
  ring_emit(ring, 0xdeadbeef);
  ring_emit(ring, MI_NOOP);
  nugpgpu_ring_advance(gpu_priv, ring);
  printk(LOG_INFO "after ring advance\n" LOG_END);

  barrier();

  while ( i<100000 && ring->page_status[shift] == old_value ){
    if (i%10 == 0)
        printk(LOG_INFO "Still old value\n" LOG_END);
      i++;
  }

  old_value = ring->page_status[shift];
  printk(LOG_INFO "value after while loop is 0x%x\n" LOG_END, old_value);

  barrier();

  if (i<100000)
    printk(LOG_INFO "Thanks God\n" LOG_END);
  else
    printk(LOG_INFO "Help GOD\n" LOG_END);

  printk(LOG_INFO "checking status page 0x%lx = 0x%x,0x%x\n" LOG_END,
                   (unsigned long) ring->page_status,
                   (unsigned) ring->page_status[shift],
                   (unsigned) ring->page_status[shift+1]);

  nugpgpu_dump(gpu_priv);
  return;
}

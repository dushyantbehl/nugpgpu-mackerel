#include <linux/pci.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/highmem.h>

#include "nugpgpu_ringbuffer.h"
#include "nugpgpu_gtt.h"
#include "nugpgpu_dbg.h"

#include "nugpgpu_tests.h"

#define RING        (&nugpgpu_render_ring)
#define RING_PAGES  32

struct nugpgpu_ring nugpgpu_render_ring;

static inline void nugpgpu_ring_render_stop(struct nugpgpu_private *gpu_priv)
{
  NUGPGPU_WRITE(nugpgpu_render_mi_mode, _MASKED_BIT_ENABLE(nugpgpu_stop_ring));

  if(wait_for(( NUGPGPU_READ(nugpgpu_render_mi_mode) & nugpgpu_mode_idle) != 0, 10000))
    printk(LOG_ERR "MI_MODE failed to stop\n" LOG_END);
  else
    printk(LOG_INFO "MI_MODE ring stopped successfully\n" LOG_END);
}

static inline void flushtlb(struct nugpgpu_private *gpu_priv)
{
  NUGPGPU_WRITE( nugpgpu_render_instpm, 
                _MASKED_BIT_ENABLE( nugpgpu_instpm_tlb_invalidate 
                                    | nugpgpu_instpm_sync_flush));

  if( wait_for((NUGPGPU_READ(nugpgpu_render_instpm) 
               & nugpgpu_instpm_sync_flush) == 0, 100000))
    printk(LOG_ERR "Wait for SyncFlush timedout\n" LOG_END);
  else
    printk(LOG_INFO "TLB invalidated successfully\n" LOG_END);
}

//TODO: no wraping logic.
int nugpgpu_ring_begin(struct nugpgpu_ring *ring, int num_dwords)
{
  printk(LOG_INFO "nugpgpu_ring_begin\n" LOG_END);
  TRACE_IN
  ring->space -= num_dwords * sizeof(uint32_t);
  TRACE_OUT
  return 0;
}

inline void ring_emit(struct nugpgpu_ring *ring, u32 data)
{
    iowrite32(data, ring->virtual_start + ring->tail);
    ring->tail += 4;
}

void nugpgpu_ring_advance(struct nugpgpu_private *gpu_priv,
                          struct nugpgpu_ring *ring)
{
    ring->tail &= ring->size -1;
    NUGPGPU_WRITE(nugpgpu_render_tail, ring->tail);
}

inline int ring_space(struct nugpgpu_ring *ring)
{
  int space =  (ring->head & nugpgpu_RING_HEAD_ADDR) - (ring->tail + nugpgpu_RING_FREE_SPACE);
  if(space < 0)
    space += ring->size;
  return space;
}

int ring_dispatch(struct nugpgpu_private *gpu_priv ,struct nugpgpu_ring *ring, u32 offset)
{
    int ret;
    ret = nugpgpu_ring_begin(ring, 2);
    if (ret)
        return ret;

    ring_emit(ring, MI_BATCH_BUFFER_START);
    ring_emit(ring, offset);
    nugpgpu_ring_advance(gpu_priv, ring);
    return 0;
}


int nugpgpu_ringbuffer_render_init(struct nugpgpu_private *gpu_priv)
{
  int ret;
  u32 head;

  printk(LOG_INFO "nugpgpu_ringbuffer_render_init\n" LOG_END);
  TRACE_IN

  RING->mmio_base = nugpgpu_RENDER_RING_BASE;
  RING->size = PAGE_SIZE * RING_PAGES;

  /* Allocate the status page. */
  ret = allocate_object(gpu_priv, &RING->status_obj, 1);
  if (ret){
    printk(LOG_ERR "Failed to allocate the status page\n" LOG_END);
    return 1;
  }

  RING->gva_status = nugpgpu_gtt_insert(gpu_priv, RING->status_obj.pg_list, 
                                        nugpgpu_cache_llc);
  if (RING->gva_status == (unsigned int)-1){
    printk(LOG_ERR "Failed to insert the status page in gtt\n" LOG_END);
    return 1;
  }

  printk(LOG_INFO "RING->gva_status : 0x%x\n" LOG_END, (unsigned int) RING->gva_status);

  RING->page_status = kmap(sg_page(RING->status_obj.pg_list->sgl));
  if (RING->page_status == NULL) {
    printk(LOG_ERR "Failed to map page_status\n" LOG_END);
    return 1;
  }
  memset(RING->page_status, 0, PAGE_SIZE);
  printk(LOG_INFO "RING->page_status : 0x%lx\n" LOG_END, (unsigned long) RING->page_status);

  /* Allocate the ringbuffer object */
  ret = allocate_object(gpu_priv, &RING->ringbuf_obj, RING_PAGES);
  if (ret){
    printk(LOG_ERR "Failed to allocate the status page\n" LOG_END);
    return 1;
  }

  RING->gva_ringbuffer = nugpgpu_gtt_insert(gpu_priv, RING->ringbuf_obj.pg_list, 
                                            nugpgpu_cache_llc);
  if (RING->gva_ringbuffer == (unsigned int)-1){
    printk(LOG_ERR "Failed to insert the status page in gtt\n" LOG_END);
    return 1;
  }

  printk(LOG_INFO "RING->gva_ringbuffer : 0x%x\n" LOG_END, (unsigned int) RING->gva_ringbuffer);

  RING->page_ringbuffer = kmap(sg_page(RING->ringbuf_obj.pg_list->sgl));
  if (RING->page_ringbuffer == NULL) {
    printk(LOG_ERR "Failed to map page_ringbuffer\n" LOG_END);
    return 1;
  }

  RING->virtual_start = ioremap_wc(gpu_priv->gtt.mappable_base + PAGE_SIZE, RING->size);
  if (RING->virtual_start == NULL) {
    printk(LOG_ERR "Problem while mapping virtual start ioremap_wc\n" LOG_END);
    return 1;
  }

  printk(LOG_INFO "Allocated the ringbuffer\n" LOG_END);

  /* Initialize the ring now.*/

  gpu_forcewake_get(gpu_priv);

  // Write status page register
  printk(LOG_INFO "writing status page register\n" LOG_END);

  NUGPGPU_WRITE(nugpgpu_RENDER_HWS_PGA_GEN7, RING->gva_status);
  NUGPGPU_READ(nugpgpu_RENDER_HWS_PGA_GEN7);

  flushtlb(gpu_priv);

  // Stop ring
  printk(LOG_INFO "stopping ring\n" LOG_END);

  NUGPGPU_WRITE(nugpgpu_render_ctl, 0);
  NUGPGPU_WRITE(nugpgpu_render_head, 0);
  NUGPGPU_WRITE(nugpgpu_render_tail, 0);

  // The doc says this enforces ordering between multiple writes
  head = NUGPGPU_READ(nugpgpu_render_head) & nugpgpu_RING_HEAD_ADDR;
  if ( head !=0 ){
    printk(LOG_ERR "failed to set head to zero\n" LOG_END);
    NUGPGPU_WRITE(nugpgpu_render_head, 0);

    if (NUGPGPU_READ(nugpgpu_render_head) & nugpgpu_RING_HEAD_ADDR) {
      printk(LOG_ERR "failed to set ring head to zero "
                     "ctl %08x head %08x tail %08x start %08x\n"
             LOG_END,
             NUGPGPU_READ(nugpgpu_render_ctl),
             NUGPGPU_READ(nugpgpu_render_head),
             NUGPGPU_READ(nugpgpu_render_tail),
             NUGPGPU_READ(nugpgpu_render_start));
    }
  }

  /* i915 driver says the below line...?? */
  /* Enforce ordering by reading HEAD register back */
  NUGPGPU_READ(nugpgpu_render_head);

  NUGPGPU_WRITE(nugpgpu_render_start, RING->gva_ringbuffer);
  NUGPGPU_WRITE(nugpgpu_render_ctl, (((RING->size - PAGE_SIZE) &
                             nugpgpu_RING_NR_PAGES) |
                             nugpgpu_is_valid));

  /* If the head is still not zero, the ring is dead */
  if(wait_for((NUGPGPU_READ(nugpgpu_render_ctl) & nugpgpu_is_valid) != 0 &&
          NUGPGPU_READ(nugpgpu_render_start) == RING->gva_ringbuffer &&
          (NUGPGPU_READ(nugpgpu_render_head) & nugpgpu_RING_HEAD_ADDR) == 0, 50)) {
    printk(LOG_ERR "ring failed to start\n" LOG_END);
    return -EIO;
  }

  RING->head = NUGPGPU_READ(nugpgpu_render_head);
  RING->tail = NUGPGPU_READ(nugpgpu_render_tail);
  RING->space = ring_space(RING);

  printk(LOG_INFO "ring->space = %d\n" LOG_END, RING->space);

  gpu_forcewake_put(gpu_priv);

  NUGPGPU_WRITE(nugpgpu_render_mi_mode, _MASKED_BIT_ENABLE(nugpgpu_vs_timer_dispatch));
  NUGPGPU_WRITE(nugpgpu_render_mi_mode, _MASKED_BIT_ENABLE(nugpgpu_async_flip_pref_disble));
  NUGPGPU_WRITE(nugpgpu_render_mode_gen7, _MASKED_BIT_DISABLE(nugpgpu_GFX_TLB_INVALIDATE_ALWAYS) |
                       _MASKED_BIT_ENABLE(nugpgpu_GFX_REPLAY_MODE));
  NUGPGPU_WRITE(nugpgpu_render_instpm, _MASKED_BIT_ENABLE(nugpgpu_instpm_force_ordering));

  dword_check(gpu_priv, RING, temp);

  TRACE_OUT
  return 0;
}

#ifndef __NUGPGPU_RINGBUFFER_H__
#define __NUGPGPU_RINGBUFFER_H__

#include "nugpgpu_drv.h"
#include "nugpgpu_misc.h"
#include "nugpgpu_memmgr.h"

#define RING_ACTUAL_BASE(regs, ring_base)   (regs + ring_base)

#define RING_NR_PAGES   0x001FF000
#define RING_HEAD_ADDR  0x001FFFFC
#define RING_TAIL_ADDR  0x001FFFF8
#define RING_FREE_SPACE 64

#define IDIHASHMSK(x)           (((x) & 0x3f) << 16)

/*
 * Memory interface instructions for ring.
 */
#define MI_INSTR(opcode, flags) (((opcode) << 23) | (flags))
#define MI_FLUSH    MI_INSTR(0x04, 0)
#define MI_STORE_DWORD_IMM MI_INSTR(0x20, 1)

#define MI_STORE_DWORD_INDEX MI_INSTR(0x21, 1)
#define MI_STORE_DWORD_INDEX_SHIFT 2
#define MI_USER_INTERRUPT MI_INSTR(0x02, 0)
#define MI_NOOP MI_INSTR(0, 0)

#define MI_BATCH_BUFFER_START MI_INSTR(0x31, 0)
#define MI_BATCH_BUFFER_END   MI_INSTR(0x0a, 0)

struct nugpgpu_ring {
  struct nugpgpu_private *gpu_priv;
  u32 mmio_base;
  int size;                     // Ring-buffer size
  void __iomem *virtual_start;
  u32 *page_ringbuffer;        // Kernel VA ring-buffer page
  u32 *page_status;             // Kernel VA status page
  u32 gva_ringbuffer;           // Graphics virtual address ring-buffer
  u32 gva_status;               // Graphics virtual address status page
  struct nugpgpu_obj status_obj;
  struct nugpgpu_obj ringbuf_obj;
  u32 head;
  u32 tail;
  int space;
  int effective_size;
};

int nugpgpu_ringbuffer_render_init(struct nugpgpu_private *gpu_priv);
inline int ring_space(struct nugpgpu_ring *ring);
void nugpgpu_ring_advance(struct nugpgpu_private *gpu_priv,
                          struct nugpgpu_ring *ring);
inline void ring_emit(struct nugpgpu_ring *ring, u32 data);
int nugpgpu_ring_begin(struct nugpgpu_ring *ring, int num_dwords);

#endif

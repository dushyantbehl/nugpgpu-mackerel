#ifndef __NUGPGPU_TEST_H_
#define __NUGPGPU_TEST_H_

#include "nugpgpu_drv.h"
#include "nugpgpu_memmgr.h"
#include "nugpgpu_ringbuffer.h"
#include "nugpgpu_gtt.h"
#include "nugpgpu_dbg.h"

#define dword_check(dev, ring, pos)   \
    do {                \
      printk(LOG_INFO "checking dword at %s,%d\n" LOG_END,__func__,__LINE__);  \
      check_dword_##pos(dev, ring); \
    }while(0);

void check_dword_hws(struct nugpgpu_private *gpu_priv, struct nugpgpu_ring *ring);
void check_dword_temp(struct nugpgpu_private *gpu_priv, struct nugpgpu_ring *ring);

#endif

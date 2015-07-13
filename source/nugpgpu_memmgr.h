#ifndef __NUGPGPU_MEMMGR_H_
#define __NUGPGPU_MEMMGR_H_

#include "nugpgpu_drv.h"
#include "nugpgpu_gtt.h"
#include <linux/scatterlist.h>
#include <linux/pci.h>

struct nugpgpu_obj {
  struct sg_table *pg_list;
  uint32_t cache_level;
  struct nugpgpu_private *gpu_priv;
};

int allocate_object(struct nugpgpu_private *gpu_priv, 
                    struct nugpgpu_obj *obj, u32 num);


#endif

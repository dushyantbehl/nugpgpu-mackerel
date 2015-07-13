#ifndef __NUGPGPU_DRV_H__
#define __NUGPGPU_DRV_H__

#include <linux/types.h>

#include "../generated/nugpgpu_dev.h"
#include "nugpgpu_regrw.h"
#include "nugpgpu_gtt.h"
#include "nugpgpu_dbg.h"

#define IS_HASWELL(dev) 1 

#define IS_HSW_GT3(dev) (IS_HASWELL(dev) && \
                        ((dev)->pdev->device & 0x00F0) == 0x0020)

// Stripped down version of `struct drm_i915_private`
struct nugpgpu_private {
  struct pci_dev *pdev;
  dev_t dev;
  struct device *dev_device;
  void __iomem *regs;
  struct gpu_gtt gtt;
  struct nugpgpu_t nugpgpu_dev;
  unsigned forcewake_count;
  struct gpu_mmio_funcs mmio_funcs;
};

#endif

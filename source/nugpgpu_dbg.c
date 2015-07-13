#include "nugpgpu_dbg.h"
#include <linux/pci.h>
#include <linux/device.h>

#include "nugpgpu_ringbuffer.h"

int depth = 0;
int debug = 0;

static void my915_mmio_dump(struct nugpgpu_private *gpu_priv)
{
  struct pci_dev *dev = gpu_priv->pdev;

  dev_info(&dev->dev, "\n\nnugpgpu DUMP\n\n");

  dev_info(&dev->dev, "nugpgpu_ECOBUS\t\t0x%x\n", NUGPGPU_READ(nugpgpu_ECOBUS));
  dev_info(&dev->dev, "nugpgpu_FORCEWAKE\t\t0x%x\n", NUGPGPU_READ(nugpgpu_FORCEWAKE));
  dev_info(&dev->dev, "nugpgpu_FORCEWAKE_MT\t\t0x%x\n", NUGPGPU_READ(nugpgpu_FORCEWAKE_MT));
  dev_info(&dev->dev, "nugpgpu_MI_PREDICATE_RESULT_2\t\t0x%x\n", NUGPGPU_READ(nugpgpu_MI_PREDICATE_RESULT_2));
  dev_info(&dev->dev, "nugpgpu_HSW_IDICR\t\t0x%x\n", NUGPGPU_READ(nugpgpu_HSW_IDICR));
  dev_info(&dev->dev, "RENDER_RING_nugpgpu\n");
  dev_info(&dev->dev, "RING_TAIL\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_tail));
  dev_info(&dev->dev, "RING_HEAD\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_head));
  dev_info(&dev->dev, "RING_START\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_start));
  dev_info(&dev->dev, "RING_CTL\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_ctl));
  dev_info(&dev->dev, "RING_MODE\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_mi_mode));
  dev_info(&dev->dev, "RING_ACTHD\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_acthd));
  dev_info(&dev->dev, "RING_INSTDONE\t\t\t0x%x\n", NUGPGPU_READ(nugpgpu_render_instpm));
}

void nugpgpu_dump(struct nugpgpu_private *gpu_priv)
{
  debug = 0;
  dev_info(&gpu_priv->pdev->dev, "\n");
  my915_mmio_dump(gpu_priv);
  dev_info(&gpu_priv->pdev->dev, "\n");
  debug = 1;
}

/*
 * nugpgpu: Driver initialization and exit routines
 * Based on i915 driver source code.
 */

#include <linux/module.h>
#include <linux/pci.h>

#include "nugpgpu_drv.h" /* Driver core structs */
#include "nugpgpu_vendor.h" /* Device specific details. */
#include "nugpgpu_misc.h"
#include "nugpgpu_dbg.h"
#include "nugpgpu_gtt.h"
#include "nugpgpu_ringbuffer.h"

#define DRIVER_LICENSE      "GPL and additional rights"
#define DRIVER_NAME         "nugpgpu"
#define DRIVER_DESCRIPTION  "New GPGPU Driver for Intel Haswell Chipsets"
#define DRIVER_AUTHOR       "Dushyant Behl <myselfdushyantbehl@gmail.com, "\
                            "Utkarsh <utkarshsins@gmail.com>"

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);

static int __init nugpgpu_init (void);

/* Array of pci device id's we support. */
static const struct pci_device_id nugpgpu_id_list[] = {
  NUGPGPU_PCI_IDLIST,
  {0, 0, 0}      /* No Idea currently what these zeros are, padding maybe */
};
MODULE_DEVICE_TABLE(pci, nugpgpu_id_list);

static void nugpgpu_hw_init(struct nugpgpu_private *gpu_priv)
{
  uint32_t gt_mode = IS_HSW_GT3(gpu_priv) ? nugpgpu_lower_slice_enable : nugpgpu_lower_slice_disable;

  printk(LOG_INFO "nugpgpu_hw_init\n" LOG_END);
  TRACE_IN

  printk(LOG_INFO "MI_PREDICATE_RESULT_2 0x%x\n" LOG_END,
          NUGPGPU_READ( nugpgpu_MI_PREDICATE_RESULT_2 ));
  NUGPGPU_WRITE( nugpgpu_MI_PREDICATE_RESULT_2, gt_mode );
  printk(LOG_INFO "MI_PREDICATE_RESULT_2 0x%x\n" LOG_END,
          NUGPGPU_READ(nugpgpu_MI_PREDICATE_RESULT_2));

  TRACE_OUT
}

static int nugpgpu_mmio_map(struct pci_dev *pdev,
                            struct nugpgpu_private * dev_priv)
{
  printk(LOG_INFO "nugpgpu_mmio_map\n" LOG_END);
  TRACE_IN

  dev_priv->regs = pci_iomap(pdev, nugpgpu_bar, nugpgpu_size);
  if (!dev_priv->regs)
  {
    printk(LOG_ERR "Failed to map registers\n" LOG_END);
    return -EIO;
  }

  printk(LOG_INFO "MMIO Bar : %d\n" LOG_END, nugpgpu_bar);
  printk(LOG_INFO "MMIO Size : 0x%x\n" LOG_END, nugpgpu_size);
  printk(LOG_INFO "MMIO registers successfully mapped to 0x%lx\n" LOG_END,
        (unsigned long) dev_priv->regs);

  TRACE_OUT
  return 0;
}

static int nugpgpu_pci_probe(struct pci_dev *pdev,
                              const struct pci_device_id *ent)
{
  int ret;
  struct nugpgpu_private * dev_priv;
  struct gpu_gtt *gtt;

  printk(LOG_INFO "nugpgpu_pci_probe\n" LOG_END);
  TRACE_IN

  printk(LOG_INFO "Got information for a device - %s \n" LOG_END,
          pci_name(pdev));

  dev_priv = kzalloc(sizeof(*dev_priv), GFP_KERNEL);
  if (dev_priv == NULL)
  {
    printk(LOG_ERR "Failed to allocate memory for dev_priv\n" LOG_END);
    TRACE_OUT
    return -ENOMEM;
  }

  ret = pci_enable_device(pdev);
  if (ret)
  {
    printk(LOG_ERR "pci_enable_device returned %d \n" LOG_END, ret);
    // TODO : free dev_priv
    TRACE_OUT
    return ret;
  }

  dev_priv->pdev = pdev;
  pci_set_drvdata(pdev, dev_priv);

  nugpgpu_mmio_map(pdev, dev_priv);

  /* Initialize the mackerel device */
  nugpgpu_initialize( &dev_priv->nugpgpu_dev, dev_priv->regs ); 

  nugpgpu_regrw_init( dev_priv );

  gtt = &dev_priv->gtt;
  ret = nugpgpu_gtt_init(dev_priv);
  if(ret)
  {
    printk(LOG_ERR "Failed to probe GTT\n" LOG_END);
  }

  pci_set_master(pdev);
  nugpgpu_hw_init( dev_priv );

  ret = nugpgpu_ringbuffer_render_init(dev_priv);
  if (ret)
  {
    printk(LOG_ERR "Problem while initializing the ring\n" LOG_END);
  }

  TRACE_OUT
  return 0;
}

static void nugpgpu_pci_remove(struct pci_dev *pdev)
{
  struct nugpgpu_private * dev_priv = pci_get_drvdata(pdev);

  printk(LOG_INFO "nugpgpu_pci_remove\n" LOG_END);
  TRACE_IN

  if (dev_priv->regs != NULL)
    pci_iounmap(pdev, dev_priv->regs);

  pci_disable_device(pdev);

  TRACE_OUT
  return;
}

/* struct PCI_DRIVER to register the pci driver with the kernel. */
static struct pci_driver nugpgpu_pci_driver = {
  name: DRIVER_NAME,
  id_table: nugpgpu_id_list,
  probe: nugpgpu_pci_probe,
  remove: nugpgpu_pci_remove,
};

/* Driver Initialization Routine. */
static int __init nugpgpu_init (void)
{
  int rc;

  TRACE_INIT
  printk(LOG_INFO "nugpgpu_init\n" LOG_END);
  TRACE_IN

  printk(LOG_INFO "Initializing module\n" LOG_END);

  rc = pci_register_driver( &nugpgpu_pci_driver );
  if ( rc == 0 )
      printk(LOG_INFO "Driver registered successfully\n" LOG_END);
  else
      printk(LOG_ERR "Driver registeration failed\n" LOG_END);

  TRACE_OUT
  return rc;
}

/* Driver Exit Routine. */
static void __exit nugpgpu_exit (void)
{
  printk(LOG_INFO "nugpgpu_exit\n" LOG_END);
  TRACE_IN

  printk(LOG_INFO "Exiting module\n" LOG_END);

  pci_unregister_driver(&nugpgpu_pci_driver);
  printk(LOG_INFO "Driver unregistered successfully\n" LOG_END);
  TRACE_OUT
}

module_init(nugpgpu_init);
module_exit(nugpgpu_exit);

/* nugpgpu_init.c */

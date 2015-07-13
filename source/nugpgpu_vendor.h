#ifndef __NUGPGPU_VENDOR_H__
#define __NUGPGPU_VENDOR_H__

/*
 * nugpgpu: Vendor / Device Specific Information.
 * Taken directly from i915 driver source code.
 */
#include "nugpgpu_pciids.h"
#include "../generated/nugpgpu_dev.h"

#define DEV_INFO_FOR_EACH_FLAG(func, sep) \
    func(is_mobile) sep \
    func(need_gfx_hws) sep \
    func(is_haswell) sep \
    func(has_fbc) sep \
    func(has_hotplug) sep \
    func(has_llc) sep \
    func(has_ddi) sep \
    func(has_fpga_dbg)

#define DEFINE_FLAG(name) u8 name:1
#define SEP_SEMICOLON ;

struct intel_device_info {
    u32 display_mmio_offset;
    u8 gen;
    u8 ring_mask; /* Rings supported by the HW */
    DEV_INFO_FOR_EACH_FLAG(DEFINE_FLAG, SEP_SEMICOLON);
};

#define GEN7_FEATURES                      \
        .gen = 7,                          \
        .need_gfx_hws = 1,                 \
        .has_hotplug = 1,                  \
        .has_fbc = 1,                      \
        .has_llc = 1,                      \
        .has_fpga_dbg = 1,                 \
        .has_ddi = 1,                      \
        .is_haswell = 1,                   \
        .ring_mask = nugpgpu_RENDER_RING | \
                     nugpgpu_BSD_RING    | \
                     nugpgpu_VEBOX_RING  | \
                     nugpgpu_BLT_RING,     \

/* Device info for desktop based chipsets */
static const struct intel_device_info intel_haswell_d_info = {
    GEN7_FEATURES
};

/* Device info for mobile chipsets */
static const struct intel_device_info intel_haswell_m_info = {
    GEN7_FEATURES
    .is_mobile = 1,
};

#define NUGPGPU_PCI_IDLIST          \
    INTEL_HSW_D_IDS(&intel_haswell_d_info), \
    INTEL_HSW_M_IDS(&intel_haswell_m_info)

/* nugpgpu_vendor.h */

#endif

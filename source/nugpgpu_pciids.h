#ifndef __NUGPGPU_PCIIDS_H_
#define __NUGPGPU_PCIIDS_H_

/* Taken from include/drm/i915_pciids.h */

/*
 * A pci_device_id struct {
 *  __u32 vendor, device;
 *      __u32 subvendor, subdevice;
 *  __u32 class, class_mask;
 *  kernel_ulong_t driver_data;
 * };
 * Don't use C99 here because "class" is reserved and we want to
 * give userspace flexibility.
 */
#define INTEL_VGA_DEVICE(id, info) {        \
    0x8086, id,             \
    ~0, ~0,                 \
    0x030000, 0xff0000,         \
    (unsigned long) info }

#define INTEL_HSW_D_IDS(info) \
    INTEL_VGA_DEVICE(0x0402, info), /* GT1 desktop */ \
    INTEL_VGA_DEVICE(0x0412, info), /* GT2 desktop */ \
    INTEL_VGA_DEVICE(0x0422, info), /* GT3 desktop */ \
    INTEL_VGA_DEVICE(0x040a, info), /* GT1 server */ \
    INTEL_VGA_DEVICE(0x041a, info), /* GT2 server */ \
    INTEL_VGA_DEVICE(0x042a, info), /* GT3 server */ \
    INTEL_VGA_DEVICE(0x040B, info), /* GT1 reserved */ \
    INTEL_VGA_DEVICE(0x041B, info), /* GT2 reserved */ \
    INTEL_VGA_DEVICE(0x042B, info), /* GT3 reserved */ \
    INTEL_VGA_DEVICE(0x040E, info), /* GT1 reserved */ \
    INTEL_VGA_DEVICE(0x041E, info), /* GT2 reserved */ \
    INTEL_VGA_DEVICE(0x042E, info), /* GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0C02, info), /* SDV GT1 desktop */ \
    INTEL_VGA_DEVICE(0x0C12, info), /* SDV GT2 desktop */ \
    INTEL_VGA_DEVICE(0x0C22, info), /* SDV GT3 desktop */ \
    INTEL_VGA_DEVICE(0x0C0A, info), /* SDV GT1 server */ \
    INTEL_VGA_DEVICE(0x0C1A, info), /* SDV GT2 server */ \
    INTEL_VGA_DEVICE(0x0C2A, info), /* SDV GT3 server */ \
    INTEL_VGA_DEVICE(0x0C0B, info), /* SDV GT1 reserved */ \
    INTEL_VGA_DEVICE(0x0C1B, info), /* SDV GT2 reserved */ \
    INTEL_VGA_DEVICE(0x0C2B, info), /* SDV GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0C0E, info), /* SDV GT1 reserved */ \
    INTEL_VGA_DEVICE(0x0C1E, info), /* SDV GT2 reserved */ \
    INTEL_VGA_DEVICE(0x0C2E, info), /* SDV GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0A02, info), /* ULT GT1 desktop */ \
    INTEL_VGA_DEVICE(0x0A12, info), /* ULT GT2 desktop */ \
    INTEL_VGA_DEVICE(0x0A22, info), /* ULT GT3 desktop */ \
    INTEL_VGA_DEVICE(0x0A0A, info), /* ULT GT1 server */ \
    INTEL_VGA_DEVICE(0x0A1A, info), /* ULT GT2 server */ \
    INTEL_VGA_DEVICE(0x0A2A, info), /* ULT GT3 server */ \
    INTEL_VGA_DEVICE(0x0A0B, info), /* ULT GT1 reserved */ \
    INTEL_VGA_DEVICE(0x0A1B, info), /* ULT GT2 reserved */ \
    INTEL_VGA_DEVICE(0x0A2B, info), /* ULT GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0D02, info), /* CRW GT1 desktop */ \
    INTEL_VGA_DEVICE(0x0D12, info), /* CRW GT2 desktop */ \
    INTEL_VGA_DEVICE(0x0D22, info), /* CRW GT3 desktop */ \
    INTEL_VGA_DEVICE(0x0D0A, info), /* CRW GT1 server */ \
    INTEL_VGA_DEVICE(0x0D1A, info), /* CRW GT2 server */ \
    INTEL_VGA_DEVICE(0x0D2A, info), /* CRW GT3 server */ \
    INTEL_VGA_DEVICE(0x0D0B, info), /* CRW GT1 reserved */ \
    INTEL_VGA_DEVICE(0x0D1B, info), /* CRW GT2 reserved */ \
    INTEL_VGA_DEVICE(0x0D2B, info), /* CRW GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0D0E, info), /* CRW GT1 reserved */ \
    INTEL_VGA_DEVICE(0x0D1E, info), /* CRW GT2 reserved */ \
    INTEL_VGA_DEVICE(0x0D2E, info)  /* CRW GT3 reserved */ \

#define INTEL_HSW_M_IDS(info) \
    INTEL_VGA_DEVICE(0x0406, info), /* GT1 mobile */ \
    INTEL_VGA_DEVICE(0x0416, info), /* GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0426, info), /* GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0C06, info), /* SDV GT1 mobile */ \
    INTEL_VGA_DEVICE(0x0C16, info), /* SDV GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0C26, info), /* SDV GT3 mobile */ \
    INTEL_VGA_DEVICE(0x0A06, info), /* ULT GT1 mobile */ \
    INTEL_VGA_DEVICE(0x0A16, info), /* ULT GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0A26, info), /* ULT GT3 mobile */ \
    INTEL_VGA_DEVICE(0x0A0E, info), /* ULX GT1 mobile */ \
    INTEL_VGA_DEVICE(0x0A1E, info), /* ULX GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0A2E, info), /* ULT GT3 reserved */ \
    INTEL_VGA_DEVICE(0x0D06, info), /* CRW GT1 mobile */ \
    INTEL_VGA_DEVICE(0x0D16, info), /* CRW GT2 mobile */ \
    INTEL_VGA_DEVICE(0x0D26, info)  /* CRW GT3 mobile */

#endif

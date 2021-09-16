/** @file

  TD_VMM_DATA library include TD_VMM_DATA types and query interfaces.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _CH_VMM_DATA_LIB_H_
#define _CH_VMM_DATA_LIB_H_


#define VMM_DATA_TYPE_SIGNATURE                         0x0000
#define VMM_DATA_TYPE_INTERFACE_VERSION                 0x0001
#define VMM_DATA_TYPE_SYSTEM_UUID                       0x0002
#define VMM_DATA_TYPE_RAM_SIZE                          0x0003
#define VMM_DATA_TYPE_GRAPHICS_ENABLED                  0x0004
#define VMM_DATA_TYPE_SMP_CPU_COUNT                     0x0005
#define VMM_DATA_TYPE_MACHINE_ID                        0x0006
#define VMM_DATA_TYPE_KERNEL_ADDRESS                    0x0007
#define VMM_DATA_TYPE_KERNEL_SIZE                       0x0008
#define VMM_DATA_TYPE_KERNEL_COMMAND_LINE               0x0009
#define VMM_DATA_TYPE_INITRD_ADDRESS                    0x000A
#define VMM_DATA_TYPE_INITRD_SIZE                       0x000B
#define VMM_DATA_TYPE_BOOT_DEVICE                       0x000C
#define VMM_DATA_TYPE_NUMA_DATA                         0x000D
#define VMM_DATA_TYPE_BOOT_MENU                         0x000E
#define VMM_DATA_TYPE_MAX_CPU_COUNT                     0x000F
#define VMM_DATA_TYPE_KERNEL_ENTRY                      0x0010
#define VMM_DATA_TYPE_KERNEL_DATA                       0x0011
#define VMM_DATA_TYPE_INITRD_DATA                       0x0012
#define VMM_DATA_TYPE_COMMAND_LINE_ADDRESS              0x0013
#define VMM_DATA_TYPE_COMMAND_LINE_SIZE                 0x0014
#define VMM_DATA_TYPE_COMMAND_LINE_DATA                 0x0015
#define VMM_DATA_TYPE_KERNEL_SETUP_ADDRESS              0x0016
#define VMM_DATA_TYPE_KERNEL_SETUP_SIZE                 0x0017
#define VMM_DATA_TYPE_KERNEL_SETUP_DATA                 0x0018
#define VMM_DATA_TYPE_FILE_DIR                          0x0019
#define VMM_DATA_TYPE_ACPI_TABLES                       0x8000
#define VMM_DATA_TYPE_SMBIOS_TABLES                     0x8001
#define VMM_DATA_TYPE_IRQ0_OVERRIDE                     0x8002
#define VMM_DATA_TYPE_E820_TABLE                        0x8003
#define VMM_DATA_TYPE_HPET_DATA                         0x8004


typedef struct {
  UINT64                            StartAddress;
  UINT64                            Length;
  UINT16                            VmmDataType;
} TD_VMM_DATA;

/**
  Query TdVmmData of specific type from HOB

  @param[in]     VmmDataType - The type of VmmData need to be queried.

  @retval    The Pointer to the TD_VMM_DATA instance.
  @retval    NULL - The type of VmmDataType is not found in HOB

**/
TD_VMM_DATA*
EFIAPI
ChGetVmmDataItem (
  IN  UINT16       VmmDataType
  );

#endif


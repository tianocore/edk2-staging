/** @file

  Copyright (c) 2017, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Glossary:
    - ACPI   - Advanced Configuration and Power Interface
    - SMBIOS - System Management BIOS
    - DT     - Device Tree
**/

#ifndef DYNAMIC_TABLE_FACTORY_PROTOCOL_H_
#define DYNAMIC_TABLE_FACTORY_PROTOCOL_H_

#include <AcpiTableGenerator.h>
#include <SmbiosTableGenerator.h>
#include <DeviceTreeTableGenerator.h>

/** This macro defines the Dynamic Table Factory Protocol GUID.

  GUID: {91D1E327-FE5A-49B8-AB65-0ECE2DDB45EC}
*/
#define EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL_GUID         \
  { 0x91d1e327, 0xfe5a, 0x49b8,                         \
    { 0xab, 0x65, 0xe, 0xce, 0x2d, 0xdb, 0x45, 0xec }   \
  };

/** This macro defines the Configuration Manager Protocol Revision.
*/
#define EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL_REVISION  CREATE_REVISION (1, 0)

#pragma pack(1)

/**
  Forward declarations:
*/
typedef struct DynamicTableFactoryProtocol EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL;
typedef struct DynamicTableFactoryInfo EFI_DYNAMIC_TABLE_FACTORY_INFO;

/** Return a pointer to the ACPI table generator.

  @param [in]  This       Pointer to the Dynamic Table Factory Protocol.
  @param [in]  TableId    The ACPI table generator ID for the
                          requested generator.
  @param [out] Generator  Pointer to the requested ACPI table
                          generator.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The requested generator is not found
                                in the list of registered generators.
*/
typedef
EFI_STATUS
EFIAPI
(EFIAPI * EFI_DYNAMIC_TABLE_FACTORY_GET_ACPI_TABLE_GENERATOR) (
  IN  CONST EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL  * CONST This,
  IN  CONST ACPI_TABLE_GENERATOR_ID                     GeneratorId,
  OUT CONST ACPI_TABLE_GENERATOR               ** CONST Generator
  );

/** Return a pointer to the SMBIOS table generator.

  @param [in]  This       Pointer to the Dynamic Table Factory Protocol.
  @param [in]  TableId    The SMBIOS table generator ID for the
                          requested generator.
  @param [out] Generator  Pointer to the requested SMBIOS table
                          generator.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The requested generator is not found
                                in the list of registered generators.
*/
typedef
EFI_STATUS
EFIAPI
(EFIAPI * EFI_DYNAMIC_TABLE_FACTORY_GET_SMBIOS_TABLE_GENERATOR) (
  IN  CONST EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL  * CONST This,
  IN  CONST SMBIOS_TABLE_GENERATOR_ID                   GeneratorId,
  OUT CONST SMBIOS_TABLE_GENERATOR             ** CONST Generator
  );

/** Return a pointer to the Device Tree table generator.

  @param [in]  This       Pointer to the Dynamic Table Factory Protocol.
  @param [in]  TableId    The Device Tree table generator ID for the
                          requested generator.
  @param [out] Generator  Pointer to the requested Device Tree table
                          generator.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The requested generator is not found
                                in the list of registered generators.
*/
typedef
EFI_STATUS
EFIAPI
(EFIAPI * EFI_DYNAMIC_TABLE_FACTORY_GET_DT_TABLE_GENERATOR) (
  IN  CONST EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL  * CONST This,
  IN  CONST DT_TABLE_GENERATOR_ID                       GeneratorId,
  OUT CONST DT_TABLE_GENERATOR                 ** CONST Generator
  );

/** A structure describing the Dynamic Table Factory Protocol interface.
*/
typedef struct DynamicTableFactoryProtocol {
  /// The Dynamic Table Factory Protocol revision.
  UINT32                                               Revision;

  /// The interface used to request an ACPI Table Generator.
  EFI_DYNAMIC_TABLE_FACTORY_GET_ACPI_TABLE_GENERATOR   GetAcpiTableGenerator;

  /// The interface used to request a SMBIOS Table Generator.
  EFI_DYNAMIC_TABLE_FACTORY_GET_SMBIOS_TABLE_GENERATOR GetSmbiosTableGenerator;

  /// The interface used to request a Device Tree Table Generator.
  EFI_DYNAMIC_TABLE_FACTORY_GET_DT_TABLE_GENERATOR     GetDtTableGenerator;

  /** Pointer to the data structure that holds the
      list of registered table generators
  */
  EFI_DYNAMIC_TABLE_FACTORY_INFO          * TableFactoryInfo;
} EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL;

/** The Dynamic Table Factory Protocol GUID.
*/
extern EFI_GUID gEfiDynamicTableFactoryProtocolGuid;

#pragma pack()

#endif // DYNAMIC_TABLE_FACTORY_PROTOCOL_H_

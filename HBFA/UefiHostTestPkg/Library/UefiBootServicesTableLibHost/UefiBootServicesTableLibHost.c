/** @file

Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

#include "DxeMain.h"

extern EFI_BOOT_SERVICES mBootServices;
extern EFI_SYSTEM_TABLE mEfiSystemTableTemplate;

EFI_SYSTEM_TABLE   *gST         = &mEfiSystemTableTemplate;
EFI_BOOT_SERVICES  *gBS         = &mBootServices;
EFI_HANDLE         gImageHandle = NULL;

EFI_HANDLE            gDxeCoreImageHandle = NULL;
EFI_SECURITY2_ARCH_PROTOCOL              *gSecurity2;

EFI_STATUS
EFIAPI
CoreAllocatePages (
  IN  EFI_ALLOCATE_TYPE     Type,
  IN  EFI_MEMORY_TYPE       MemoryType,
  IN  UINTN                 NumberOfPages,
  OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  VOID *Buffer;

  Buffer = malloc (EFI_PAGES_TO_SIZE(NumberOfPages));
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  *Memory = (UINTN)Buffer;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CoreFreePages (
  IN EFI_PHYSICAL_ADDRESS  Memory,
  IN UINTN                 NumberOfPages
  )
{
  free ((VOID *)(UINTN)Memory);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CoreAllocatePool (
  IN EFI_MEMORY_TYPE  PoolType,
  IN UINTN            Size,
  OUT VOID            **Buffer
  )
{
  *Buffer = malloc (Size);
  if (*Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CoreFreePool (
  IN VOID  *Buffer
  )
{
  free (Buffer);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Stall (
  IN UINTN            Microseconds
  ) 
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg0 (
  VOID
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg1 (
  UINTN Arg1
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg2 (
  UINTN Arg1,
  UINTN Arg2
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg3 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg4 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg5 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4,
  UINTN Arg5
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg6 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4,
  UINTN Arg5,
  UINTN Arg6
  )
{
  ASSERT(FALSE);
  return EFI_UNSUPPORTED;
}

EFI_BOOT_SERVICES mBootServices = {
  {
    EFI_BOOT_SERVICES_SIGNATURE,                                                          // Signature
    EFI_BOOT_SERVICES_REVISION,                                                           // Revision
    sizeof (EFI_BOOT_SERVICES),                                                           // HeaderSize
    0,                                                                                    // CRC32
    0                                                                                     // Reserved
  },
  (EFI_RAISE_TPL)                               CoreRaiseTpl,                             // RaiseTPL
  (EFI_RESTORE_TPL)                             CoreRestoreTpl,                           // RestoreTPL
  (EFI_ALLOCATE_PAGES)                          CoreAllocatePages,                        // AllocatePages
  (EFI_FREE_PAGES)                              CoreFreePages,                            // FreePages
  (EFI_GET_MEMORY_MAP)                          CoreEfiNotAvailableYetArg5,               // GetMemoryMap
  (EFI_ALLOCATE_POOL)                           CoreAllocatePool,                         // AllocatePool
  (EFI_FREE_POOL)                               CoreFreePool,                             // FreePool
  (EFI_CREATE_EVENT)                            CoreEfiNotAvailableYetArg5,               // CreateEvent
  (EFI_SET_TIMER)                               CoreEfiNotAvailableYetArg3,               // SetTimer
  (EFI_WAIT_FOR_EVENT)                          CoreEfiNotAvailableYetArg3,               // WaitForEvent
  (EFI_SIGNAL_EVENT)                            CoreEfiNotAvailableYetArg1,               // SignalEvent
  (EFI_CLOSE_EVENT)                             CoreEfiNotAvailableYetArg1,               // CloseEvent
  (EFI_CHECK_EVENT)                             CoreEfiNotAvailableYetArg1,               // CheckEvent
  (EFI_INSTALL_PROTOCOL_INTERFACE)              CoreInstallProtocolInterface,             // InstallProtocolInterface
  (EFI_REINSTALL_PROTOCOL_INTERFACE)            CoreReinstallProtocolInterface,           // ReinstallProtocolInterface
  (EFI_UNINSTALL_PROTOCOL_INTERFACE)            CoreUninstallProtocolInterface,           // UninstallProtocolInterface
  (EFI_HANDLE_PROTOCOL)                         CoreHandleProtocol,                       // HandleProtocol
  (VOID *)                                      NULL,                                     // Reserved
  (EFI_REGISTER_PROTOCOL_NOTIFY)                CoreRegisterProtocolNotify,               // RegisterProtocolNotify
  (EFI_LOCATE_HANDLE)                           CoreLocateHandle,                         // LocateHandle
  (EFI_LOCATE_DEVICE_PATH)                      CoreLocateDevicePath,                     // LocateDevicePath
  (EFI_INSTALL_CONFIGURATION_TABLE)             CoreEfiNotAvailableYetArg2,               // InstallConfigurationTable
  (EFI_IMAGE_LOAD)                              CoreEfiNotAvailableYetArg6,               // LoadImage
  (EFI_IMAGE_START)                             CoreEfiNotAvailableYetArg3,               // StartImage
  (EFI_EXIT)                                    CoreEfiNotAvailableYetArg4,               // Exit
  (EFI_IMAGE_UNLOAD)                            CoreEfiNotAvailableYetArg1,               // UnloadImage
  (EFI_EXIT_BOOT_SERVICES)                      CoreEfiNotAvailableYetArg2,               // ExitBootServices
  (EFI_GET_NEXT_MONOTONIC_COUNT)                CoreEfiNotAvailableYetArg1,               // GetNextMonotonicCount
  (EFI_STALL)                                   Stall,               // Stall
  (EFI_SET_WATCHDOG_TIMER)                      CoreEfiNotAvailableYetArg4,               // SetWatchdogTimer
  (EFI_CONNECT_CONTROLLER)                      CoreEfiNotAvailableYetArg4,               // ConnectController
  (EFI_DISCONNECT_CONTROLLER)                   CoreEfiNotAvailableYetArg3,               // DisconnectController
  (EFI_OPEN_PROTOCOL)                           CoreOpenProtocol,                         // OpenProtocol
  (EFI_CLOSE_PROTOCOL)                          CoreCloseProtocol,                        // CloseProtocol
  (EFI_OPEN_PROTOCOL_INFORMATION)               CoreOpenProtocolInformation,              // OpenProtocolInformation
  (EFI_PROTOCOLS_PER_HANDLE)                    CoreProtocolsPerHandle,                   // ProtocolsPerHandle
  (EFI_LOCATE_HANDLE_BUFFER)                    CoreLocateHandleBuffer,                   // LocateHandleBuffer
  (EFI_LOCATE_PROTOCOL)                         CoreLocateProtocol,                       // LocateProtocol
  (EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES)    CoreInstallMultipleProtocolInterfaces,    // InstallMultipleProtocolInterfaces
  (EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES)  CoreUninstallMultipleProtocolInterfaces,  // UninstallMultipleProtocolInterfaces
  (EFI_CALCULATE_CRC32)                         CoreEfiNotAvailableYetArg3,               // CalculateCrc32
  (EFI_COPY_MEM)                                CopyMem,                                  // CopyMem
  (EFI_SET_MEM)                                 SetMem,                                   // SetMem
  (EFI_CREATE_EVENT_EX)                         CoreEfiNotAvailableYetArg6                // CreateEventEx
};

EFI_STATUS
EFIAPI
OutputString (
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
  IN CHAR16                                 *String
  )
{
  DEBUG ((DEBUG_INFO, "%s", String));
  return EFI_SUCCESS;
}

EFI_SIMPLE_TEXT_OUTPUT_MODE mMode = {
  1, // MaxMode
  0, // Mode
  0, // Attribute
  80, // CursorColumn
  25, // CursorRow
  FALSE, // CursorVisible
};

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL mConOut = {
  (EFI_TEXT_RESET)               CoreEfiNotAvailableYetArg2, // Reset
  OutputString,
  (EFI_TEXT_TEST_STRING)         CoreEfiNotAvailableYetArg2, // TestString
  (EFI_TEXT_QUERY_MODE)          CoreEfiNotAvailableYetArg4, // QueryMode
  (EFI_TEXT_SET_MODE)            CoreEfiNotAvailableYetArg2, // SetMode
  (EFI_TEXT_SET_ATTRIBUTE)       CoreEfiNotAvailableYetArg2, // SetAttribute
  (EFI_TEXT_CLEAR_SCREEN)        CoreEfiNotAvailableYetArg1, // ClearScreen
  (EFI_TEXT_SET_CURSOR_POSITION) CoreEfiNotAvailableYetArg3, // SetCursorPosition
  (EFI_TEXT_ENABLE_CURSOR)       CoreEfiNotAvailableYetArg2, // EnableCursor
  NULL,                       // Mode
};

EFI_SYSTEM_TABLE mEfiSystemTableTemplate = {
  {
    EFI_SYSTEM_TABLE_SIGNATURE,                                           // Signature
    EFI_SYSTEM_TABLE_REVISION,                                            // Revision
    sizeof (EFI_SYSTEM_TABLE),                                            // HeaderSize
    0,                                                                    // CRC32
    0                                                                     // Reserved
  },
  NULL,                                                                   // FirmwareVendor
  0,                                                                      // FirmwareRevision
  NULL,                                                                   // ConsoleInHandle
  NULL,                                                                   // ConIn
  NULL,                                                                   // ConsoleOutHandle
  &mConOut,                                                               // ConOut
  NULL,                                                                   // StandardErrorHandle
  NULL,                                                                   // StdErr
  NULL,                                                                   // RuntimeServices
  &mBootServices,                                                         // BootServices
  0,                                                                      // NumberOfConfigurationTableEntries
  NULL                                                                    // ConfigurationTable
};

EFI_STATUS
EFIAPI
UefiBootServicesTableLibConstructor (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = gBS->InstallProtocolInterface (
                  &gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );

  return Status;
}

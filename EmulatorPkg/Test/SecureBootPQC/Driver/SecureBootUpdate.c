/** @file
  UEFI Application to update Secure Boot variables (PK, KEK, DB) in Emulator environment.

  Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Library/FileHandleLib.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/GlobalVariable.h>

#define SECURE_BOOT_MODE_ENABLE   1
#define SECURE_BOOT_MODE_DISABLE  0

//
// Custom Mode GUID and Variable Name
//
#define EFI_CUSTOM_MODE_NAME  L"CustomMode"

/**
  Read file content into buffer.

  @param[in]  FileName      File name to read.
  @param[out] FileSize      Size of the file.
  @param[out] FileBuffer    Buffer containing file content.

  @retval EFI_SUCCESS       File read successfully.
  @retval Others            Failed to read file.

**/
EFI_STATUS
ReadFileToBuffer (
  IN  CHAR16  *FileName,
  OUT UINTN   *FileSize,
  OUT VOID    **FileBuffer
  )
{
  EFI_STATUS          Status;
  SHELL_FILE_HANDLE   FileHandle;
  UINT64              Size;
  UINTN               ReadSize;
  VOID                *Buffer;

  if (FileName == NULL || FileSize == NULL || FileBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *FileSize = 0;
  *FileBuffer = NULL;

  Status = ShellOpenFileByName (FileName, &FileHandle, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ReadFileToBuffer: Cannot open file '%s': %r\n", FileName, Status));
    Print (L"Error: Cannot open file '%s'\n", FileName);
    return Status;
  }

  Status = FileHandleGetSize (FileHandle, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ReadFileToBuffer: Cannot get file size: %r\n", Status));
    Print (L"Error: Cannot get file size\n");
    ShellCloseFile (&FileHandle);
    return Status;
  }

  if (Size == 0) {
    DEBUG ((DEBUG_ERROR, "ReadFileToBuffer: File is empty\n"));
    Print (L"Error: File is empty\n");
    ShellCloseFile (&FileHandle);
    return EFI_NOT_FOUND;
  }

  Buffer = AllocatePool ((UINTN)Size);
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "ReadFileToBuffer: Cannot allocate %lu bytes\n", (UINT64)Size));
    Print (L"Error: Cannot allocate memory for file\n");
    ShellCloseFile (&FileHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  ReadSize = (UINTN)Size;
  Status = FileHandleRead (FileHandle, &ReadSize, Buffer);
  ShellCloseFile (&FileHandle);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ReadFileToBuffer: Cannot read file: %r\n", Status));
    Print (L"Error: Cannot read file\n");
    FreePool (Buffer);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "ReadFileToBuffer: Successfully loaded %u bytes from file\n", ReadSize));
  *FileSize = ReadSize;
  *FileBuffer = Buffer;
  return EFI_SUCCESS;
}

/**
  Create EFI_SIGNATURE_LIST from certificate data.

  @param[in]  CertData      Certificate data (DER format).
  @param[in]  CertSize      Size of certificate data.
  @param[in]  SignatureType Signature type GUID.
  @param[out] SigListSize   Size of created signature list.
  @param[out] SigList       Created signature list.

  @retval EFI_SUCCESS       Signature list created successfully.
  @retval Others            Failed to create signature list.

**/
EFI_STATUS
CreateSignatureList (
  IN  VOID      *CertData,
  IN  UINTN     CertSize,
  IN  EFI_GUID  *SignatureType,
  OUT UINTN     *SigListSize,
  OUT VOID      **SigList
  )
{
  EFI_SIGNATURE_LIST  *SigListHdr;
  EFI_SIGNATURE_DATA  *SigData;
  UINTN               TotalSize;
  UINT8               *Buffer;

  if (CertData == NULL || SignatureType == NULL || SigListSize == NULL || SigList == NULL) {
    DEBUG ((DEBUG_ERROR, "CreateSignatureList: Invalid parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate total size: EFI_SIGNATURE_LIST + EFI_SIGNATURE_DATA + CertSize
  //
  TotalSize = sizeof (EFI_SIGNATURE_LIST) + sizeof (EFI_GUID) + CertSize;
  DEBUG ((DEBUG_INFO, "CreateSignatureList: Total size = %u bytes\n", TotalSize));

  Buffer = AllocateZeroPool (TotalSize);
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "CreateSignatureList: Cannot allocate %u bytes\n", TotalSize));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Fill EFI_SIGNATURE_LIST header
  //
  SigListHdr = (EFI_SIGNATURE_LIST *)Buffer;
  CopyGuid (&SigListHdr->SignatureType, SignatureType);
  SigListHdr->SignatureListSize = (UINT32)TotalSize;
  SigListHdr->SignatureHeaderSize = 0;
  SigListHdr->SignatureSize = (UINT32)(sizeof (EFI_GUID) + CertSize);

  //
  // Fill EFI_SIGNATURE_DATA
  //
  SigData = (EFI_SIGNATURE_DATA *)(Buffer + sizeof (EFI_SIGNATURE_LIST));
  ZeroMem (&SigData->SignatureOwner, sizeof (EFI_GUID));  // Use zero GUID as owner
  CopyMem (SigData->SignatureData, CertData, CertSize);

  *SigListSize = TotalSize;
  *SigList = Buffer;

  DEBUG ((DEBUG_INFO, "CreateSignatureList: Signature list created successfully\n"));
  return EFI_SUCCESS;
}

/**
  Wrap signature list with EFI_VARIABLE_AUTHENTICATION_2 header.
  This is needed even in SETUP_MODE because ProcessVarWithKek expects this format.

  @param[in]  SigListData      Signature list data.
  @param[in]  SigListSize      Size of signature list.
  @param[out] AuthDataSize     Size of wrapped data.
  @param[out] AuthData         Wrapped data with authentication header.

  @retval EFI_SUCCESS          Data wrapped successfully.
  @retval Others               Failed to wrap data.

**/
EFI_STATUS
WrapWithAuthHeader (
  IN  VOID   *SigListData,
  IN  UINTN  SigListSize,
  OUT UINTN  *AuthDataSize,
  OUT VOID   **AuthData
  )
{
  EFI_VARIABLE_AUTHENTICATION_2  *AuthHdr;
  UINTN                          TotalSize;
  UINT8                          *Buffer;
  EFI_TIME                       Time;

  if (SigListData == NULL || AuthDataSize == NULL || AuthData == NULL) {
    DEBUG ((DEBUG_ERROR, "WrapWithAuthHeader: Invalid parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate total size: EFI_VARIABLE_AUTHENTICATION_2 (with minimal PKCS#7) + SigList
  // For SETUP_MODE, we use a minimal authentication header with empty signature
  //
  TotalSize = OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo) +
              OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData) +
              SigListSize;

  DEBUG ((DEBUG_INFO, "WrapWithAuthHeader: Total size = %u bytes (auth header + payload)\n", TotalSize));

  Buffer = AllocateZeroPool (TotalSize);
  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "WrapWithAuthHeader: Cannot allocate %u bytes\n", TotalSize));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Fill EFI_VARIABLE_AUTHENTICATION_2 header
  //
  AuthHdr = (EFI_VARIABLE_AUTHENTICATION_2 *)Buffer;
  
  //
  // Set timestamp to current time (zeros are fine for SETUP_MODE)
  //
  ZeroMem (&Time, sizeof (EFI_TIME));
  Time.Year = 2025;
  Time.Month = 1;
  Time.Day = 1;
  CopyMem (&AuthHdr->TimeStamp, &Time, sizeof (EFI_TIME));

  //
  // Fill WIN_CERTIFICATE_UEFI_GUID
  //
  AuthHdr->AuthInfo.Hdr.dwLength = OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData);
  AuthHdr->AuthInfo.Hdr.wRevision = 0x0200;
  AuthHdr->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
  CopyGuid (&AuthHdr->AuthInfo.CertType, &gEfiCertPkcs7Guid);

  //
  // Copy signature list after authentication header
  //
  CopyMem (
    Buffer + OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo) + 
             OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData),
    SigListData,
    SigListSize
    );

  *AuthDataSize = TotalSize;
  *AuthData = Buffer;

  DEBUG ((DEBUG_INFO, "WrapWithAuthHeader: Data wrapped successfully\n"));
  return EFI_SUCCESS;
}

/**
  Update Secure Boot variable.

  @param[in]  VariableName  Variable name to update.
  @param[in]  VendorGuid    Vendor GUID.
  @param[in]  Data          Variable data.
  @param[in]  DataSize      Size of variable data.
  @param[in]  UseAppend     Use APPEND_WRITE mode.

  @retval EFI_SUCCESS       Variable updated successfully.
  @retval Others            Failed to update variable.

**/
EFI_STATUS
UpdateSecureBootVariable (
  IN CHAR16    *VariableName,
  IN EFI_GUID  *VendorGuid,
  IN VOID      *Data,
  IN UINTN     DataSize,
  IN BOOLEAN   UseAppend
  )
{
  EFI_STATUS  Status;
  UINT32      Attributes;

  if (VariableName == NULL || VendorGuid == NULL) {
    DEBUG ((DEBUG_ERROR, "UpdateSecureBootVariable: Invalid parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Set attributes for authenticated variables
  //
  Attributes = EFI_VARIABLE_NON_VOLATILE |
               EFI_VARIABLE_BOOTSERVICE_ACCESS |
               EFI_VARIABLE_RUNTIME_ACCESS |
               EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;

  //
  // Add APPEND_WRITE attribute if requested
  //
  if (UseAppend) {
    Attributes |= EFI_VARIABLE_APPEND_WRITE;
    DEBUG ((DEBUG_INFO, "UpdateSecureBootVariable: Using APPEND mode for variable '%s'\n", VariableName));
  } else {
    DEBUG ((DEBUG_INFO, "UpdateSecureBootVariable: Using REPLACE mode for variable '%s'\n", VariableName));
  }

  DEBUG ((DEBUG_INFO, "UpdateSecureBootVariable: Updating variable '%s', size=%u, attributes=0x%x\n", 
          VariableName, DataSize, Attributes));
  Print (L"Updating variable '%s'...\n", VariableName);

  Status = gRT->SetVariable (
                  VariableName,
                  VendorGuid,
                  Attributes,
                  DataSize,
                  Data
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UpdateSecureBootVariable: Failed to update '%s': %r\n", VariableName, Status));
    Print (L"Error: Failed to update variable '%s': %r\n", VariableName, Status);
  } else {
    DEBUG ((DEBUG_INFO, "UpdateSecureBootVariable: Successfully updated '%s'\n", VariableName));
    Print (L"Success: Variable '%s' updated\n", VariableName);
  }

  return Status;
}

/**
  Clear Secure Boot variable (delete it).
  
  This function handles clearing both in Setup Mode and User Mode:
  - Setup Mode: Direct deletion (no auth required)
  - User Mode: Use empty EFI_VARIABLE_AUTHENTICATION_2 header

  @param[in]  VariableName  Variable name to clear.
  @param[in]  VendorGuid    Vendor GUID.

  @retval EFI_SUCCESS       Variable cleared successfully.
  @retval Others            Failed to clear variable.

**/
EFI_STATUS
ClearSecureBootVariable (
  IN CHAR16    *VariableName,
  IN EFI_GUID  *VendorGuid
  )
{
  EFI_STATUS  Status;
  UINT8       SetupMode;
  UINTN       DataSize;

  DEBUG ((DEBUG_INFO, "ClearSecureBootVariable: Clearing variable '%s'\n", VariableName));
  Print (L"Clearing variable '%s'...\n", VariableName);

  //
  // Check if we're in Setup Mode
  //
  DataSize = sizeof (SetupMode);
  Status = gRT->GetVariable (
                  EFI_SETUP_MODE_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &SetupMode
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "ClearSecureBootVariable: Cannot read SetupMode, assuming Setup Mode\n"));
    SetupMode = 1;  // Assume Setup Mode if can't read
  }

  EFI_VARIABLE_AUTHENTICATION_2  AuthHeader;
  EFI_TIME                       Time;
  UINTN                          AuthSize;
  
  //
  // Fill timestamp
  //
  ZeroMem (&Time, sizeof (EFI_TIME));
  Time.Year = 2025;
  Time.Month = 1;
  Time.Day = 1;
  CopyMem (&AuthHeader.TimeStamp, &Time, sizeof (EFI_TIME));
  
  //
  // Fill WIN_CERTIFICATE_UEFI_GUID with minimal size (no CertData)
  //
  AuthHeader.AuthInfo.Hdr.dwLength = OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData);
  AuthHeader.AuthInfo.Hdr.wRevision = 0x0200;
  AuthHeader.AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
  CopyGuid (&AuthHeader.AuthInfo.CertType, &gEfiCertPkcs7Guid);
  
  //
  // Calculate size: timestamp + WIN_CERTIFICATE header (no payload)
  //
  AuthSize = OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo) +
              OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData);
  
  Status = gRT->SetVariable (
                  VariableName,
                  VendorGuid,
                  EFI_VARIABLE_NON_VOLATILE | 
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                  EFI_VARIABLE_RUNTIME_ACCESS |
                  EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS,
                  AuthSize,
                  &AuthHeader
                  );

  if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
    DEBUG ((DEBUG_ERROR, "ClearSecureBootVariable: Failed to clear '%s': %r\n", VariableName, Status));
    Print (L"Error: Failed to clear variable '%s': %r\n", VariableName, Status);
  } else {
    DEBUG ((DEBUG_INFO, "ClearSecureBootVariable: Successfully cleared '%s'\n", VariableName));
    Print (L"Success: Variable '%s' cleared\n", VariableName);
    Status = EFI_SUCCESS;
  }

  return Status;
}

/**
  Display current Secure Boot status.

  @retval EFI_SUCCESS       Status displayed successfully.

**/
EFI_STATUS
DisplaySecureBootStatus (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT8       SecureBoot;
  UINT8       SetupMode;
  UINTN       DataSize;

  Print (L"\n=== Current Secure Boot Status ===\n");

  //
  // Check SecureBoot variable
  //
  DataSize = sizeof (SecureBoot);
  Status = gRT->GetVariable (
                  EFI_SECURE_BOOT_MODE_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &SecureBoot
                  );

  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SecureBoot variable: %d (%a)\n", SecureBoot, SecureBoot ? "Enabled" : "Disabled"));
    Print (L"SecureBoot: %d (%s)\n", 
           SecureBoot, 
           SecureBoot ? L"Enabled" : L"Disabled");
  } else {
    DEBUG ((DEBUG_WARN, "SecureBoot variable not found: %r\n", Status));
    Print (L"SecureBoot: Not found\n");
  }

  //
  // Check SetupMode variable
  //
  DataSize = sizeof (SetupMode);
  Status = gRT->GetVariable (
                  EFI_SETUP_MODE_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &SetupMode
                  );

  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SetupMode variable: %d (%a)\n", SetupMode, SetupMode ? "Setup Mode" : "User Mode"));
    Print (L"SetupMode: %d (%s)\n", 
           SetupMode, 
           SetupMode ? L"Setup Mode" : L"User Mode");
  } else {
    DEBUG ((DEBUG_WARN, "SetupMode variable not found: %r\n", Status));
    Print (L"SetupMode: Not found\n");
  }

  Print (L"==================================\n\n");

  return EFI_SUCCESS;
}

/**
  Print usage information.

**/
VOID
PrintUsage (
  VOID
  )
{
  Print (L"\n");
  Print (L"Usage: SecureBootUpdate.efi <command> [options]\n");
  Print (L"\n");
  Print (L"Commands:\n");
  Print (L"  status                     - Display current Secure Boot status\n");
  Print (L"  clear                      - Clear all Secure Boot variables (PK, KEK, DB, DBX)\n");
  Print (L"  update-pk <cert_file>      - Update PK from certificate file\n");
  Print (L"  update-kek <cert_file>     - Update KEK from certificate file\n");
  Print (L"  update-db <cert_file>      - Update DB from certificate file\n");
  Print (L"  update-dbx <cert_file>     - Update DBX from certificate file\n");
  Print (L"\n");
  Print (L"Examples:\n");
  Print (L"  SecureBootUpdate.efi status\n");
  Print (L"  SecureBootUpdate.efi clear\n");
  Print (L"  SecureBootUpdate.efi update-pk fs0:\\PK.cer\n");
  Print (L"  SecureBootUpdate.efi update-kek fs0:\\KEK.cer\n");
  Print (L"  SecureBootUpdate.efi update-db fs0:\\db.cer\n");
  Print (L"\n");
}

/**
  Main entry point for the application.

  @param[in] ImageHandle    Image handle.
  @param[in] SystemTable    System table.

  @retval EFI_SUCCESS       Application executed successfully.
  @retval Others            Error occurred.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  LIST_ENTRY    *ParamPackage;
  CHAR16        **Argv;
  UINTN         Argc;
  CHAR16        *Command;
  CHAR16        *FileName;
  VOID          *CertData;
  UINTN         CertSize;
  VOID          *SigList;
  UINTN         SigListSize;
  EFI_GUID      *SignatureType;
  UINTN         Index;
  CONST CHAR16  *Param;
  
  //
  // Local parameter check list (no named parameters, only positional)
  // Note: Use a different name to avoid conflict with global EmptyParamList
  //
  STATIC CONST SHELL_PARAM_ITEM LocalParamList[] = {
    {NULL, TypeMax}
  };

  DEBUG ((DEBUG_INFO, "SecureBootUpdate: Application started\n"));

  //
  // Parse command line arguments using Shell protocol
  // Must provide a valid parameter list (even if empty)
  //
  Status = ShellCommandLineParse (LocalParamList, &ParamPackage, NULL, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Failed to parse command line: %r\n", Status));
    Print (L"Error: Failed to parse command line\n");
    return Status;
  }

  //
  // Get raw parameters
  //
  Argv = NULL;
  Argc = 0;
  
  //
  // Count parameters (skip index 0 which is the program name)
  //
  if (ParamPackage != NULL) {
    Index = 1;  // Start from 1 to skip program name
    
    // Count how many raw parameters we have (excluding program name)
    while ((Param = ShellCommandLineGetRawValue (ParamPackage, Index)) != NULL) {
      Argc++;
      Index++;
    }
    
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Found %u parameters (excluding program name)\n", Argc));
    
    // Allocate array for parameters
    if (Argc > 0) {
      Argv = AllocateZeroPool (Argc * sizeof(CHAR16 *));
      if (Argv != NULL) {
        for (Index = 0; Index < Argc; Index++) {
          // Get parameter at Index+1 (skip program name at index 0)
          Argv[Index] = (CHAR16 *)ShellCommandLineGetRawValue (ParamPackage, Index + 1);
          DEBUG ((DEBUG_VERBOSE, "  Argv[%u] = '%s'\n", Index, Argv[Index]));
        }
      } else {
        DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Failed to allocate Argv array\n"));
        ShellCommandLineFreeVarList (ParamPackage);
        return EFI_OUT_OF_RESOURCES;
      }
    }
  }

  DEBUG ((DEBUG_INFO, "SecureBootUpdate: Argc=%u\n", Argc));

  if (Argc < 1) {
    DEBUG ((DEBUG_WARN, "SecureBootUpdate: No command specified\n"));
    PrintUsage ();
    if (ParamPackage != NULL) {
      ShellCommandLineFreeVarList (ParamPackage);
    }
    if (Argv != NULL) {
      FreePool (Argv);
    }
    return EFI_INVALID_PARAMETER;
  }

  Command = Argv[0];
  DEBUG ((DEBUG_INFO, "SecureBootUpdate: Command='%s'\n", Command));

  //
  // Handle 'status' command
  //
  if (StrCmp (Command, L"status") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Executing status command\n"));
    Status = DisplaySecureBootStatus ();
    goto Done;
  }

  //
  // Handle 'clear' command
  //
  if (StrCmp (Command, L"clear") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Executing clear command\n"));
    Print (L"========================================\n");
    Print (L"  Clear All Secure Boot Variables\n");
    Print (L"========================================\n");
    Print (L"\n");
    
    UINT8  SetupMode;
    UINT8  CustomMode;
    UINTN  DataSize;
    BOOLEAN EnteredCustomMode = FALSE;
    
    //
    // Check if we're in User Mode
    //
    DataSize = sizeof (SetupMode);
    Status = gRT->GetVariable (
                    EFI_SETUP_MODE_NAME,
                    &gEfiGlobalVariableGuid,
                    NULL,
                    &DataSize,
                    &SetupMode
                    );
    
    if (!EFI_ERROR (Status)) {
      //
      // In User Mode: Enter Custom Mode first
      //
      CustomMode = 1;
      Status = gRT->SetVariable (
                      EFI_CUSTOM_MODE_NAME,
                      &gEfiCustomModeEnableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                      sizeof (CustomMode),
                      &CustomMode
                      );
      
      if (EFI_ERROR (Status)) {
        Print (L"Warning: Failed to enter Custom Mode: %r\n", Status);
        Print (L"Attempting to clear variables anyway...\n\n");
      } else {
        Print (L"Successfully entered Custom Mode\n\n");
        EnteredCustomMode = TRUE;
      }
    }
    
    //
    // Clear all variables
    //
    Print (L"Clearing all Secure Boot variables...\n");
    ClearSecureBootVariable (EFI_KEY_EXCHANGE_KEY_NAME, &gEfiGlobalVariableGuid);
    ClearSecureBootVariable (EFI_IMAGE_SECURITY_DATABASE, &gEfiImageSecurityDatabaseGuid);
    ClearSecureBootVariable (EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid);
    ClearSecureBootVariable (EFI_PLATFORM_KEY_NAME, &gEfiGlobalVariableGuid);

    //
    // Exit Custom Mode if we entered it
    //
    if (EnteredCustomMode) {
      CustomMode = 0;
      gRT->SetVariable (
             EFI_CUSTOM_MODE_NAME,
             &gEfiCustomModeEnableGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (CustomMode),
             &CustomMode
             );
      Print (L"Exited Custom Mode\n");
    }
    
    Print (L"\nAll variables cleared\n");
    goto Done;
  }

  //
  // Handle update commands
  //
  if (Argc < 2) {
    DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Missing certificate file name\n"));
    Print (L"Error: Missing certificate file name\n");
    PrintUsage ();
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  FileName = Argv[1];
  DEBUG ((DEBUG_INFO, "SecureBootUpdate: Certificate file='%s'\n", FileName));

  //
  // Read certificate file
  //
  Status = ReadFileToBuffer (FileName, &CertSize, &CertData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Failed to read certificate file: %r\n", Status));
    goto Done;
  }

  //
  // Check if file is .auth format (contains EFI_VARIABLE_AUTHENTICATION_2)
  // or plain certificate format (.cer/.der)
  //
  BOOLEAN IsAuthFile = FALSE;
  UINTN   FileNameLen = StrLen (FileName);
  
  // Check file extension
  if (FileNameLen > 5) {
    CHAR16 *Ext = FileName + FileNameLen - 5;
    if (StrCmp (Ext, L".auth") == 0) {
      IsAuthFile = TRUE;
      DEBUG ((DEBUG_INFO, "SecureBootUpdate: Using .auth file, will write directly\n"));
    }
  }

  //
  // Prepare data for SetVariable
  //
  VOID  *DataToWrite;
  UINTN DataSizeToWrite;
  
  if (IsAuthFile) {
    //
    // .auth file already contains EFI_VARIABLE_AUTHENTICATION_2 + EFI_SIGNATURE_LIST
    // Write directly without additional wrapping
    //
    DataToWrite = CertData;
    DataSizeToWrite = CertSize;
    
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Using .auth file directly, size=%u\n", DataSizeToWrite));
  } else {
    //
    // Plain certificate file (.cer/.der)
    // Need to wrap in EFI_SIGNATURE_LIST
    //
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Plain certificate, creating signature list\n"));
    
    SignatureType = &gEfiCertX509Guid;
    
    Status = CreateSignatureList (CertData, CertSize, SignatureType, &SigListSize, &SigList);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Failed to create signature list: %r\n", Status));
      Print (L"Error: Failed to create signature list\n");
      FreePool (CertData);
      goto Done;
    }
    
    VOID *AuthData;
    UINTN AuthDataSize;
    Status = WrapWithAuthHeader(SigList, SigListSize, &AuthDataSize, &AuthData);
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Failed to wrap with authentication header: %r\n", Status));
      Print (L"Error: Failed to wrap with authentication header\n");
      FreePool (CertData);
      FreePool (SigList);
      goto Done;
    }

    DataToWrite = AuthData;
    DataSizeToWrite = AuthDataSize;
  }

  //
  // Update appropriate variable based on command
  //
  if (StrCmp (Command, L"update-pk") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Updating PK\n"));
    Status = UpdateSecureBootVariable (
               EFI_PLATFORM_KEY_NAME,
               &gEfiGlobalVariableGuid,
               DataToWrite,
               DataSizeToWrite,
               TRUE
               );
  } else if (StrCmp (Command, L"update-kek") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Updating KEK\n"));
    Status = UpdateSecureBootVariable (
               EFI_KEY_EXCHANGE_KEY_NAME,
               &gEfiGlobalVariableGuid,
               DataToWrite,
               DataSizeToWrite,
               TRUE
               );
  } else if (StrCmp (Command, L"update-db") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Updating DB\n"));
    Status = UpdateSecureBootVariable (
               EFI_IMAGE_SECURITY_DATABASE,
               &gEfiImageSecurityDatabaseGuid,
               DataToWrite,
               DataSizeToWrite,
               TRUE
               );
  } else if (StrCmp (Command, L"update-dbx") == 0) {
    DEBUG ((DEBUG_INFO, "SecureBootUpdate: Updating DBX\n"));
    Status = UpdateSecureBootVariable (
               EFI_IMAGE_SECURITY_DATABASE1,
               &gEfiImageSecurityDatabaseGuid,
               DataToWrite,
               DataSizeToWrite,
               TRUE
               );
  } else {
    DEBUG ((DEBUG_ERROR, "SecureBootUpdate: Unknown command '%s'\n", Command));
    Print (L"Error: Unknown command '%s'\n", Command);
    PrintUsage ();
    Status = EFI_INVALID_PARAMETER;
  }

  //
  // Clean up
  //
  FreePool (CertData);
  if (!IsAuthFile && SigList != NULL) {
    FreePool (SigList);
  }

  //
  // Display updated status
  //
  if (!EFI_ERROR (Status)) {
    if (StrCmp (Command, L"update-pk") == 0) {
      DisplaySecureBootStatus ();
    }
  }

Done:
  //
  // Free command line package
  //
  if (ParamPackage != NULL) {
    ShellCommandLineFreeVarList (ParamPackage);
  }
  
  if (Argv != NULL) {
    FreePool (Argv);
  }

  DEBUG ((DEBUG_INFO, "SecureBootUpdate: Application completed with status %r\n", Status));
  return Status;
}

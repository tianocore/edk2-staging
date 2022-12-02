/** @file
  EDKII TestSpdm

  Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <library/spdm_requester_lib.h>
#include <Protocol/PciIo.h>
#include <Protocol/SpdmIo.h>
#include <Protocol/Spdm.h>
#include <Protocol/SpdmTest.h>
#include <Protocol/DeviceSecurity.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Test/TestConfig.h>

#define USE_PSK  0

VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;

  for (Index = 0; Index < Size; Index++) {
    Print (L"%02x ", (UINTN)Data[Index]);
  }
}

VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  UINTN  Count;
  UINTN  Left;

  #define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    Print (L"\n");
  }

  if (Left != 0) {
    Print (L"%04x: ", Index * COLUME_SIZE);
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    Print (L"\n");
  }
}

VOID
TestPci (
  VOID
  )
{
  EFI_STATUS                      Status;
  SPDM_IO_PROTOCOL                *SpdmIo;
  EFI_HANDLE                      Handle;
  UINTN                           BufferSize;
  EDKII_DEVICE_SECURITY_PROTOCOL  *DeviceSecurity;
  EDKII_DEVICE_IDENTIFIER         DeviceId;

  Status = gBS->LocateProtocol (&gSpdmIoProtocolGuid, NULL, (VOID **)&SpdmIo);
  ASSERT_EFI_ERROR (Status);

  BufferSize = sizeof (Handle);
  Status     = gBS->LocateHandle (
                      ByProtocol,
                      &gEdkiiDeviceIdentifierTypePciGuid,
                      NULL,
                      &BufferSize,
                      &Handle
                      );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gSpdmIoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  SpdmIo
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (&gEdkiiDeviceSecurityProtocolGuid, NULL, (VOID **)&DeviceSecurity);
  ASSERT_EFI_ERROR (Status);

  DeviceId.Version = EDKII_DEVICE_IDENTIFIER_REVISION;
  CopyGuid (&DeviceId.DeviceType, &gEdkiiDeviceIdentifierTypePciGuid);
  DeviceId.DeviceHandle = Handle;
  Status                = DeviceSecurity->DeviceAuthenticate (DeviceSecurity, &DeviceId);
}

typedef struct {
  SPDM_DATA_TYPE    DataType;
  CHAR8             *String;
} DATA_TYPE_STRING;

#pragma pack(1)
#define TEST_PAYLOAD_CLIENT  "Hello Client!"
#define TEST_PAYLOAD_SERVER  "Hello Server!"
#define TEST_PAYLOAD_LEN     (sizeof("Hello XXXXXX!"))
///
/// SPDM VENDOR_DEFINED request
///
typedef struct {
  SPDM_MESSAGE_HEADER    Header;
  // Param1 == RSVD
  // Param2 == RSVD
  UINT16                 StandardID;
  UINT8                  Len;
  UINT16                 VendorID;
  UINT16                 PayloadLength;
  UINT8                  VendorDefinedPayload[TEST_PAYLOAD_LEN];
} SPDM_VENDOR_DEFINED_REQUEST_MINE;

///
/// SPDM VENDOR_DEFINED response
///
typedef struct {
  SPDM_MESSAGE_HEADER    Header;
  // Param1 == RSVD
  // Param2 == RSVD
  UINT16                 StandardID;
  UINT8                  Len;
  UINT16                 VendorID;
  UINT16                 PayloadLength;
  UINT8                  VendorDefinedPayload[TEST_PAYLOAD_LEN];
} SPDM_VENDOR_DEFINED_RESPONSE_MINE;

#pragma pack()

SPDM_VENDOR_DEFINED_REQUEST_MINE  mVendorDefinedRequest = {
  {
    SPDM_MESSAGE_VERSION_10,
    SPDM_VENDOR_DEFINED_REQUEST,
    0, // Param1
    0, // Param2
  },
  SPDM_REGISTRY_ID_PCISIG, // StandardID
  2,                       // Len
  0x8086,                  // VendorID
  TEST_PAYLOAD_LEN,        // PayloadLength
  { TEST_PAYLOAD_CLIENT }
};

SPDM_VENDOR_DEFINED_RESPONSE_MINE  mVendorDefinedResponse = {
  {
    SPDM_MESSAGE_VERSION_10,
    SPDM_VENDOR_DEFINED_RESPONSE,
    0, // Param1
    0, // Param2
  },
  SPDM_REGISTRY_ID_PCISIG, // StandardID
  2,                       // Len
  0x8086,                  // VendorID
  TEST_PAYLOAD_LEN,        // PayloadLength
  { TEST_PAYLOAD_SERVER }
};

/**
  Process a packet in the current SPDM session.

  @param  This                         Indicates a pointer to the calling context.
  @param  SessionId                    ID of the session.
  @param  Request                      A pointer to the request data.
  @param  RequestSize                  Size of the request data.
  @param  Response                     A pointer to the response data.
  @param  ResponseSize                 Size of the response data. On input, it means the size of Data
                                       buffer. On output, it means the size of copied Data buffer if
                                       EFI_SUCCESS, and means the size of desired Data buffer if
                                       EFI_BUFFER_TOO_SMALL.

  @retval EFI_SUCCESS                  The SPDM request is set successfully.
  @retval EFI_INVALID_PARAMETER        The DataSize is NULL or the Data is NULL and *DataSize is not zero.
  @retval EFI_UNSUPPORTED              The DataType is unsupported.
  @retval EFI_NOT_FOUND                The DataType cannot be found.
  @retval EFI_NOT_READY                The DataType is not ready to return.
  @retval EFI_BUFFER_TOO_SMALL         The buffer is too small to hold the data.
  @retval EFI_TIMEOUT                  A timeout occurred while waiting for the SPDM request
                                       to execute.
**/
EFI_STATUS
EFIAPI
TestSpdmProcessPacketCallback (
  IN     VOID   *Request,
  IN     UINTN  RequestSize,
  OUT VOID      *Response,
  IN OUT UINTN  *ResponseSize
  )
{
  SPDM_VENDOR_DEFINED_REQUEST_MINE  *SpmdRequest;

  SpmdRequest = Request;
  ASSERT (RequestSize == sizeof (SPDM_VENDOR_DEFINED_REQUEST_MINE));
  ASSERT (SpmdRequest->Header.RequestResponseCode == SPDM_VENDOR_DEFINED_REQUEST);
  ASSERT (SpmdRequest->StandardID == SPDM_REGISTRY_ID_PCISIG);
  ASSERT (SpmdRequest->VendorID == 0x8086);
  ASSERT (SpmdRequest->PayloadLength == TEST_PAYLOAD_LEN);
  ASSERT (CompareMem (SpmdRequest->VendorDefinedPayload, TEST_PAYLOAD_CLIENT, TEST_PAYLOAD_LEN) == 0);

  CopyMem (Response, &mVendorDefinedResponse, sizeof (mVendorDefinedResponse));
  *ResponseSize = sizeof (mVendorDefinedResponse);
  return EFI_SUCCESS;
}

VOID
TestSpdmApplication (
  IN SPDM_PROTOCOL       *SpdmProtocol,
  IN SPDM_TEST_PROTOCOL  *SpdmTestProtocol,
  IN UINT32              SessionId
  )
{
  EFI_STATUS                         Status;
  SPDM_VENDOR_DEFINED_REQUEST_MINE   Request;
  UINTN                              RequestSize;
  SPDM_VENDOR_DEFINED_RESPONSE_MINE  Response;
  UINTN                              ResponseSize;

  Status = SpdmTestProtocol->RegisterProcessPacketCallback (SpdmTestProtocol, TestSpdmProcessPacketCallback);

  CopyMem (&Request, &mVendorDefinedRequest, sizeof (Request));

  RequestSize  = sizeof (Request);
  ResponseSize = sizeof (Response);
  Status       = SpdmProtocol->SendReceiveData (SpdmProtocol, &SessionId, FALSE, &Request, RequestSize, &Response, &ResponseSize);
  ASSERT_EFI_ERROR (Status);

  ASSERT (ResponseSize == sizeof (SPDM_VENDOR_DEFINED_RESPONSE_MINE));
  ASSERT (Response.Header.RequestResponseCode == SPDM_VENDOR_DEFINED_RESPONSE);
  ASSERT (Response.StandardID == SPDM_REGISTRY_ID_PCISIG);
  ASSERT (Response.VendorID == 0x8086);
  ASSERT (Response.PayloadLength == TEST_PAYLOAD_LEN);
  ASSERT (CompareMem (Response.VendorDefinedPayload, TEST_PAYLOAD_SERVER, TEST_PAYLOAD_LEN) == 0);
}

VOID
TestSpdm (
  VOID
  )
{
  EFI_STATUS          Status;
  SPDM_PROTOCOL       *SpdmProtocol;
  SPDM_TEST_PROTOCOL  *SpdmTestProtocol;
  UINT32              SessionId;
  UINT8               HeartbeatPeriod;
  UINT8               MeasurementHash[64];
  UINT8               SlotId;
  UINT8               TestConfig;
  UINTN               TestConfigSize;

  TestConfigSize = sizeof(UINT8);
  Status = gRT->GetVariable (
                  L"SpdmTestConfig",
                  &gEfiDeviceSecurityPkgTestConfig,
                  NULL,
                  &TestConfigSize,
                  &TestConfig
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  if (TestConfig == TEST_CONFIG_SPDM_MESSAGE_VERSION_10) {
    //SPDM 1.0 does not support KEY_EXCHANGE or PSK_EXCHANGE, so skip.
    return;
  }

  Status = gBS->LocateProtocol (&gSpdmProtocolGuid, NULL, (VOID **)&SpdmProtocol);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (&gSpdmTestProtocolGuid, NULL, (VOID **)&SpdmTestProtocol);
  ASSERT_EFI_ERROR (Status);

 #if USE_PSK
  Status = SpdmProtocol->SetData (SpdmProtocol, SpdmDataPsk, NULL, "TestPskData", sizeof ("TestPskData"));
  ASSERT_EFI_ERROR (Status);

  Status = SpdmTestProtocol->SetData (SpdmTestProtocol, SpdmDataPsk, NULL, "TestPskData", sizeof ("TestPskData"));
  ASSERT_EFI_ERROR (Status);
 #endif

  HeartbeatPeriod = 0;
  ZeroMem (MeasurementHash, sizeof (MeasurementHash));
  SlotId = 0;
  if (TestConfig == TEST_CONFIG_DIFF_CERT_IN_DIFF_SLOT) {
    //The valid certificate chain with trust anchor is in slot_1 of responder.
    SlotId = 1;
  }
  Status = SpdmProtocol->StartSession (
                           SpdmProtocol,
                           USE_PSK,
                           SPDM_CHALLENGE_REQUEST_TCB_COMPONENT_MEASUREMENT_HASH,
                           SlotId,
                           &SessionId,
                           &HeartbeatPeriod,
                           MeasurementHash
                           );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "StartSession - %r\n", Status));
    return;
  }

  TestSpdmApplication (SpdmProtocol, SpdmTestProtocol, SessionId);

  Status = SpdmProtocol->StopSession (SpdmProtocol, SessionId, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "StopSession - %r\n", Status));
    return;
  }
}

EFI_STATUS
EFIAPI
MainEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  // CpuBreakpoint();
  TestPci ();

  TestSpdm ();
  return EFI_SUCCESS;
}

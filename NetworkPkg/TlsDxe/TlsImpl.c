/** @file
  The Miscellaneous Routines for TlsDxe driver.

Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "TlsImpl.h"

/**
  Encrypt the message listed in fragment.

  @param[in]       TlsInstance    The pointer to the TLS instance.
  @param[in, out]  FragmentTable  Pointer to a list of fragment. 
                                  On input these fragments contain the TLS header and 
                                  plain text TLS payload; 
                                  On output these fragments contain the TLS header and 
                                  cypher text TLS payload.
  @param[in]       FragmentCount  Number of fragment.

  @retval EFI_SUCCESS             The operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES    Can't allocate memory resources.
  @retval EFI_ABORTED             TLS session state is incorrect.
  @retval Others                  Other errors as indicated.
**/
EFI_STATUS
TlsEcryptPacket (
  IN     TLS_INSTANCE                  *TlsInstance,
  IN OUT EFI_TLS_FRAGMENT_DATA         **FragmentTable,
  IN     UINT32                        *FragmentCount
  )
{
  UINTN           Index;
  UINT32          BytesCopied;
  UINT32          BufferInSize;
  UINT8           *BufferIn;
  UINT8           *BufferInPtr;
  TLSRecordHeader *RecordHeaderIn;
  UINT16          ThisPlainMessageSize;
  TLSRecordHeader *TempRecordHeader;
  UINT16          ThisMessageSize;
  UINT32          BufferOutSize;
  UINT8           *BufferOut;
  INTN            Ret;
  
  BytesCopied      = 0;
  BufferInSize     = 0;
  BufferIn         = NULL;
  BufferInPtr      = NULL;
  RecordHeaderIn   = NULL;
  TempRecordHeader = NULL;
  BufferOutSize    = 0;
  BufferOut        = NULL;
  Ret              = 0;

  //
  // Calculate the size accroding to the fragment table.
  //
  for (Index = 0; Index < *FragmentCount; Index++) {
    BufferInSize += (*FragmentTable)[Index].FragmentLength;
  }

  //
  // Allocate buffer for processing data.
  //
  BufferIn = AllocateZeroPool (BufferInSize);
  if (BufferIn == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy all TLS plain record header and payload into ProcessBuffer.
  //
  for (Index = 0; Index < *FragmentCount; Index++) {
    CopyMem (
      (BufferIn + BytesCopied),
      (*FragmentTable)[Index].FragmentBuffer,
      (*FragmentTable)[Index].FragmentLength
      );
    BytesCopied += (*FragmentTable)[Index].FragmentLength;
  }

  BufferOut = AllocateZeroPool (MAX_BUFFER_SIZE);
  if (BufferOut == NULL) {
    FreePool (BufferIn);
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // Parsing buffer. 
  //
  BufferInPtr = BufferIn;
  TempRecordHeader = (TLSRecordHeader *) BufferOut;
  while ((UINTN) BufferInPtr < (UINTN) BufferIn + BufferInSize) {
    RecordHeaderIn = (TLSRecordHeader *) BufferInPtr;
    ASSERT (RecordHeaderIn->ContentType == TLS_CONTENT_TYPE_APPLICATION_DATA);
    ThisPlainMessageSize = RecordHeaderIn->Length;

    TlsWrite (TlsInstance->TlsConn, (UINT8 *) (RecordHeaderIn + 1), ThisPlainMessageSize);
    if (TlsInStateError (TlsInstance->TlsConn)) {
      FreePool (BufferIn);
      TlsInstance->TlsSessionState = EfiTlsSessionError;
      return EFI_ABORTED;
    }
    
    Ret = TlsCtrlTrafficOut (TlsInstance->TlsConn, (UINT8 *)(TempRecordHeader), MAX_BUFFER_SIZE);
    
    if (Ret > 0) {
      ThisMessageSize = (UINT16) Ret;
    } else {
      ThisMessageSize = 0;
    }

    BufferOutSize += ThisMessageSize;
    
    BufferInPtr += RECORD_HEADER_LEN + ThisPlainMessageSize;
    TempRecordHeader += ThisMessageSize;
  }

  FreePool (BufferIn);
  
  //
  // The caller will take responsible to handle the original fragment table.
  //
  *FragmentTable = AllocateZeroPool (sizeof (EFI_TLS_FRAGMENT_DATA));
  if (*FragmentTable == NULL) {
    FreePool (BufferOut);
    return EFI_OUT_OF_RESOURCES;
  }
  
  (*FragmentTable)[0].FragmentBuffer  = BufferOut;
  (*FragmentTable)[0].FragmentLength  = BufferOutSize;
  *FragmentCount                      = 1;

  return EFI_SUCCESS;
}

/**
  Decrypt the message listed in fragment.

  @param[in]       TlsInstance    The pointer to the TLS instance.
  @param[in, out]  FragmentTable  Pointer to a list of fragment.
                                  On input these fragments contain the TLS header and 
                                  cypher text TLS payload; 
                                  On output these fragments contain the TLS header and 
                                  plain text TLS payload.
  @param[in]       FragmentCount  Number of fragment.

  @retval EFI_SUCCESS             The operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES    Can't allocate memory resources.
  @retval EFI_ABORTED             TLS session state is incorrect.
  @retval Others                  Other errors as indicated.
**/
EFI_STATUS
TlsDecryptPacket (
  IN     TLS_INSTANCE                  *TlsInstance,
  IN OUT EFI_TLS_FRAGMENT_DATA         **FragmentTable,
  IN     UINT32                        *FragmentCount
  )
{
  UINTN           Index;
  UINT32          BytesCopied;
  UINT8           *BufferIn;
  UINT32          BufferInSize;
  UINT8           *BufferInPtr;
  TLSRecordHeader *RecordHeaderIn;
  UINT16          ThisCipherMessageSize;
  TLSRecordHeader *TempRecordHeader;
  UINT16          ThisPlainMessageSize;
  UINT8           *BufferOut;
  UINT32          BufferOutSize;
  INTN            Ret;
  
  BytesCopied      = 0;
  BufferIn         = NULL; 
  BufferInSize     = 0;  
  BufferInPtr      = NULL;
  RecordHeaderIn   = NULL;
  TempRecordHeader = NULL;
  BufferOut        = NULL;
  BufferOutSize    = 0;
  Ret              = 0;

  //
  // Calculate the size accroding to the fragment table.
  //
  for (Index = 0; Index < *FragmentCount; Index++) {
    BufferInSize += (*FragmentTable)[Index].FragmentLength;
  }

  //
  // Allocate buffer for processing data
  //
  BufferIn = AllocateZeroPool (BufferInSize);
  if (BufferIn == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy all TLS plain record header and payload to ProcessBuffer
  //
  for (Index = 0; Index < *FragmentCount; Index++) {
    CopyMem (
      (BufferIn + BytesCopied),
      (*FragmentTable)[Index].FragmentBuffer,
      (*FragmentTable)[Index].FragmentLength
      );
    BytesCopied += (*FragmentTable)[Index].FragmentLength;
  }

  BufferOut = AllocateZeroPool (MAX_BUFFER_SIZE);
  if (BufferOut == NULL) {
    FreePool (BufferIn);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Parsing buffer. Received packet may have multiply TLS record message.
  //
  BufferInPtr = BufferIn;
  TempRecordHeader = (TLSRecordHeader *) BufferOut;
  while ((UINTN) BufferInPtr < (UINTN) BufferIn + BufferInSize) {
    RecordHeaderIn = (TLSRecordHeader *) BufferInPtr;
    ASSERT (RecordHeaderIn->ContentType == TLS_CONTENT_TYPE_APPLICATION_DATA);
    ThisCipherMessageSize = NTOHS (RecordHeaderIn->Length);

    Ret = TlsCtrlTrafficIn (TlsInstance->TlsConn, (UINT8 *) (RecordHeaderIn), RECORD_HEADER_LEN + ThisCipherMessageSize);
    if (Ret != RECORD_HEADER_LEN + ThisCipherMessageSize) {
      FreePool (BufferIn);
      TlsInstance->TlsSessionState = EfiTlsSessionError;
      return EFI_ABORTED;
    }

    Ret = 0;
    Ret = TlsRead (TlsInstance->TlsConn, (UINT8 *) (TempRecordHeader + 1), MAX_BUFFER_SIZE);
    if (TlsInStateError (TlsInstance->TlsConn)) {
      FreePool (BufferIn);
      TlsInstance->TlsSessionState = EfiTlsSessionError;
      return EFI_ABORTED;
    }

    if (Ret > 0) {
      ThisPlainMessageSize = (UINT16) Ret;
    } else {
      ThisPlainMessageSize = 0;
    }
    
    CopyMem (TempRecordHeader, RecordHeaderIn, RECORD_HEADER_LEN);
    TempRecordHeader->Length = ThisPlainMessageSize;
    BufferOutSize += RECORD_HEADER_LEN + ThisPlainMessageSize;
    
    BufferInPtr += RECORD_HEADER_LEN + ThisCipherMessageSize;
    TempRecordHeader += RECORD_HEADER_LEN + ThisPlainMessageSize;
  }

  FreePool (BufferIn);
  
  //
  // The caller will take responsible to handle the original fragment table
  //
  *FragmentTable = AllocateZeroPool (sizeof (EFI_TLS_FRAGMENT_DATA));
  if (*FragmentTable == NULL) {
    FreePool (BufferOut);
    return EFI_OUT_OF_RESOURCES;
  }

  (*FragmentTable)[0].FragmentBuffer  = BufferOut;
  (*FragmentTable)[0].FragmentLength  = BufferOutSize;
  *FragmentCount                      = 1;

  return EFI_SUCCESS;
}

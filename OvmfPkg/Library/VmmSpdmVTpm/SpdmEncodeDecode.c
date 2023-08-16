/** @file

  Copyright (c) 2022 - 2023, Intel Corporation. All rights reserved. <BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/VTpmTd.h>
#include <Stub/SpdmLibStub.h>
#include "library/spdm_crypt_lib.h"
#include "VmmSpdmInternal.h"
#include "VTpmTransport.h"

#pragma pack(1)

typedef struct {
  UINT32    SessionId;
  UINT8     SequenceNumber[VTPM_SEQUENCE_NUMBER_COUNT];
  UINT16    Length;
} SPDM_RECORD_HEADER;

typedef struct {
  UINT16    ApplicationDataLengh;
} SPDM_ENC_DEC_MESSAGE_HEADER;

typedef struct {
  VTPM_MESSAGE_HEADER            Header1; // message_len:version:type
  SPDM_RECORD_HEADER             Header2; // session_id:sequence_number:total_length
  SPDM_ENC_DEC_MESSAGE_HEADER    Header3; // application_data_length
  VTPM_APP_MESSAGE_HEADER        Header4; // type
  // UINT8                       Message[];
  // UINT8                       RandomData[];
  // UINT8                       Mac[];
} VTPM_SECURE_SPDM_APP_MESSAGE;

#pragma pack()

/**
 * Reads a 64-bit value from memory that may be unaligned.
 *
 * @param  Buffer  The pointer to a 64-bit value that may be unaligned.
 *
 * @return The 64-bit value read from buffer.
 **/
STATIC
UINT64
ReadUint64 (
  IN CONST UINT8  *Buffer
  )
{
  return (UINT64)(Buffer[0]) |
         ((UINT64)(Buffer[1]) << 8) |
         ((UINT64)(Buffer[2]) << 16) |
         ((UINT64)(Buffer[3]) << 24) |
         ((UINT64)(Buffer[4]) << 32) |
         ((UINT64)(Buffer[5]) << 40) |
         ((UINT64)(Buffer[6]) << 48) |
         ((UINT64)(Buffer[7]) << 56);
}

/**
 * Writes a 64-bit value to memory that may be unaligned.
 *
 * @param  Buffer  The pointer to a 64-bit value that may be unaligned.
 * @param  Value   64-bit value to write to buffer.
 **/
STATIC
VOID
WriteUint64 (
  IN UINT8   *Buffer,
  IN UINT64  Value
  )
{
  Buffer[0] = (UINT8)(Value & 0xFF);
  Buffer[1] = (UINT8)((Value >> 8) & 0xFF);
  Buffer[2] = (UINT8)((Value >> 16) & 0xFF);
  Buffer[3] = (UINT8)((Value >> 24) & 0xFF);
  Buffer[4] = (UINT8)((Value >> 32) & 0xFF);
  Buffer[5] = (UINT8)((Value >> 40) & 0xFF);
  Buffer[6] = (UINT8)((Value >> 48) & 0xFF);
  Buffer[7] = (UINT8)((Value >> 56) & 0xFF);
}

/**
 * Encode the secured spdm message, including encryp the app Message
 * and pack the ciphyer text into a Secure Spdm Packet.
 *
 * @param  AppMessage           The app message which is to be encryped and packed.
 * @param  AppMessageSize       Size of app message
 * @param  SecuredMessage       Pointer to the encrypted and packet Secured Spdm Packet
 * @param  SecuredMessageSize   Size of SecuredMessage
 * @param  InfoTable            The tables of vtpm secure session.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval Others                       The message is encoded failed.
*/
STATIC
SPDM_RETURN
EncodeSecuredMessage (
  IN UINT8                               *AppMessage,
  IN UINTN                               AppMessageSize,
  IN OUT UINT8                           *SecuredMessage,
  IN OUT UINTN                           *SecuredMessageSize,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable
  )
{
  UINTN  AeadTagSize;
  UINTN  AeadKeySize;
  UINTN  AeadIvSize;

  UINT8   *Key;
  UINT8   Salt[AEAD_AES_256_GCM_IV_LEN] = { 0 };
  UINT64  SequenceNumber;
  UINT64  SequenceNumberInHeader;
  UINT64  Data64;

  UINT32  MaxRandCount;
  UINT32  RandCount;

  UINTN  PlainTextSize;
  UINTN  CipherTextSize;
  UINTN  RecordHeaderSize;
  UINTN  TotalSecuredMessageSize;

  UINT8  *Adata;
  UINT8  *EncMsg;
  UINT8  *DecMsg;
  UINT8  *Tag;

  BOOLEAN  Result;

  SPDM_AEAD_AES_256_GCM_KEY_IV_INFO  *KeyIvInfo;
  SPDM_RECORD_HEADER                 *RecordHeader;
  SPDM_ENC_DEC_MESSAGE_HEADER        *EncMsgHeader;

  RecordHeader = (SPDM_RECORD_HEADER *)SecuredMessage;
  EncMsgHeader = (SPDM_ENC_DEC_MESSAGE_HEADER *)(UINTN)(RecordHeader + 1);

  AeadTagSize = VTPM_SPDM_MAC_LENGTH;
  AeadKeySize = AEAD_AES_256_GCM_KEY_LEN;
  AeadIvSize  = AEAD_AES_256_GCM_IV_LEN;

  KeyIvInfo = (SPDM_AEAD_AES_256_GCM_KEY_IV_INFO *)(UINTN)(InfoTable + 1);
  Key       = KeyIvInfo->ReqeustDirection.Key;
  CopyMem (Salt, KeyIvInfo->ReqeustDirection.IV, sizeof (KeyIvInfo->ReqeustDirection.IV));

  // Sequence Number
  SequenceNumber = ReadUint64 (KeyIvInfo->ReqeustDirection.SequenceNumber);
  if (SequenceNumber == MAX_UINT64) {
    DEBUG ((DEBUG_ERROR, "The sequence number is equal to the max uint64 \n"));
    return LIBSPDM_STATUS_SEQUENCE_NUMBER_OVERFLOW;
  }

  SequenceNumberInHeader = SequenceNumber;

  // Salt(IV)
  Data64 = ReadUint64 ((const uint8_t *)Salt) ^ SequenceNumber;
  WriteUint64 (Salt, Data64);

  // Update the next SequenceNumber and store it in KeyIvInfo
  SequenceNumber++;
  WriteUint64 (KeyIvInfo->ReqeustDirection.SequenceNumber, SequenceNumber);

  // rand
  MaxRandCount = VTPM_MAX_RANDOM_NUMBER_COUNT;
  Result       = SpdmGetRandomNumber (sizeof (RandCount), (uint8_t *)&RandCount);
  if (!Result) {
    DEBUG ((DEBUG_ERROR, "SpdmGetRandomNumber failed \n"));
    return LIBSPDM_STATUS_LOW_ENTROPY;
  }

  RandCount = (uint8_t)((RandCount % MaxRandCount) + 1);

  RecordHeaderSize        = sizeof (SPDM_RECORD_HEADER);
  PlainTextSize           = sizeof (SPDM_ENC_DEC_MESSAGE_HEADER) + AppMessageSize + RandCount;
  CipherTextSize          = PlainTextSize;
  TotalSecuredMessageSize = RecordHeaderSize + CipherTextSize + AeadTagSize;
  if (*SecuredMessageSize < TotalSecuredMessageSize) {
    *SecuredMessageSize = TotalSecuredMessageSize;
    DEBUG ((DEBUG_ERROR, "The secure message size is small \n"));
    return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
  }

  *SecuredMessageSize = TotalSecuredMessageSize;

  // populate RecordHeader
  RecordHeader->SessionId = InfoTable->SessionId;
  CopyMem (RecordHeader->SequenceNumber, &SequenceNumberInHeader, sizeof (UINT64));
  RecordHeader->Length = (UINT16)(CipherTextSize + AeadTagSize);

  // populate EncMsgHeader
  EncMsgHeader->ApplicationDataLengh = (UINT16)AppMessageSize;

  // Random number
  Result = SpdmGetRandomNumber (
                                RandCount,
                                (uint8_t *)EncMsgHeader +
                                sizeof (SPDM_ENC_DEC_MESSAGE_HEADER) +
                                AppMessageSize
                                );
  if (!Result) {
    DEBUG ((DEBUG_ERROR, "SpdmGetRandomNumber failed \n"));
    return LIBSPDM_STATUS_LOW_ENTROPY;
  }

  Adata  = (UINT8 *)RecordHeader;
  EncMsg = (UINT8 *)(RecordHeader + 1);
  DecMsg = EncMsg;
  Tag    = (UINT8 *)RecordHeader + RecordHeaderSize + CipherTextSize;

  Result = SpdmAeadEncryption (
                               0,                                                  // secured_message_context->secured_message_version,
                               SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_AES_256_GCM,      // secured_message_context->aead_cipher_suite,
                               Key,
                               AeadKeySize,
                               Salt,
                               AeadIvSize,
                               Adata,
                               RecordHeaderSize,
                               DecMsg,
                               CipherTextSize,
                               Tag,
                               AeadTagSize,
                               EncMsg,
                               &CipherTextSize
                               );

  if (!Result) {
    DEBUG ((DEBUG_ERROR, "SpdmAeadEncryption failed \n"));
    return LIBSPDM_STATUS_CRYPTO_ERROR;
  }

  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Decode the secured spdm message,
 * and pack the ciphyer text into a App message Packet.
 *
 * @param  AppMessage           The app message which is to be decryped and packed.
 * @param  AppMessageSize       Size of app message
 * @param  SecuredMessage       Pointer to the encrypted and packet Secured Spdm Packet
 * @param  SecuredMessageSize   Size of SecuredMessage
 * @param  InfoTable            Contains the information of secure spdm session.
 *
 * @retval RETURN_SUCCESS               The message is decoded successfully.
 * @retval Others                       The message is decoded failed.
*/
STATIC
SPDM_RETURN
DecodeSecuredMessage (
  IN UINT8                               *SecuredMessage,
  IN UINTN                               SecuredMessageSize,
  IN OUT UINT8                           **AppMessage,
  IN OUT UINTN                           *AppMessageSize,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *InfoTable
  )
{
  UINTN  AeadTagSize;
  UINTN  AeadKeySize;
  UINTN  AeadIvSize;

  UINT8   *Key;
  UINT8   Salt[AEAD_AES_256_GCM_IV_LEN] = { 0 };
  UINT64  SequenceNumber;
  UINT64  SequenceNumberInHeader;
  UINT8   SequenceNumberSize;
  UINT64  Data64;

  UINTN  PlainTextSize;
  UINTN  CipherTextSize;
  UINTN  RecordHeaderSize;

  UINT8  *Adata;
  UINT8  *EncMsg;
  UINT8  *DecMsg;
  UINT8  *Tag;

  BOOLEAN  Result;

  SPDM_AEAD_AES_256_GCM_KEY_IV_INFO  *KeyIvInfo;
  SPDM_RECORD_HEADER                 *RecordHeader;
  SPDM_ENC_DEC_MESSAGE_HEADER        *EncMsgHeader;

  RecordHeader = (SPDM_RECORD_HEADER *)SecuredMessage;
  EncMsgHeader = (SPDM_ENC_DEC_MESSAGE_HEADER *)(UINTN)(RecordHeader + 1);

  AeadTagSize = VTPM_SPDM_MAC_LENGTH;
  AeadKeySize = AEAD_AES_256_GCM_KEY_LEN;
  AeadIvSize  = AEAD_AES_256_GCM_IV_LEN;

  KeyIvInfo = (SPDM_AEAD_AES_256_GCM_KEY_IV_INFO *)(UINTN)(InfoTable + 1);
  Key       = KeyIvInfo->ResponseDirection.Key;
  CopyMem (Salt, KeyIvInfo->ResponseDirection.IV, AEAD_AES_256_GCM_IV_LEN);

  // Sequence Number
  SequenceNumber = ReadUint64 (KeyIvInfo->ResponseDirection.SequenceNumber);
  if (SequenceNumber == MAX_UINT64) {
    return LIBSPDM_STATUS_SEQUENCE_NUMBER_OVERFLOW;
  }

  SequenceNumberInHeader = SequenceNumber;
  SequenceNumberSize     = VTPM_SEQUENCE_NUMBER_COUNT;

  // Salt(IV)
  Data64 = ReadUint64 ((const uint8_t *)Salt) ^ SequenceNumber;
  WriteUint64 (Salt, Data64);

  // Update the next SequenceNumber in
  SequenceNumber++;
  WriteUint64 (KeyIvInfo->ResponseDirection.SequenceNumber, SequenceNumber);

  // compare the sequence_number
  if (CompareMem (&SequenceNumberInHeader, RecordHeader->SequenceNumber, SequenceNumberSize) != 0) {
    DEBUG ((DEBUG_ERROR, "SequenceNumber doesn't match. %llx != %llx\n", SequenceNumberInHeader, *(UINT64 *)(UINTN)RecordHeader->SequenceNumber));
    return LIBSPDM_STATUS_INVALID_MSG_FIELD;
  }

  RecordHeaderSize = sizeof (SPDM_RECORD_HEADER);
  if (RecordHeader->Length != SecuredMessageSize - RecordHeaderSize) {
    DEBUG ((DEBUG_ERROR, "RecordHeader->Length doesn't match. %x vs %x\n", RecordHeader->Length, (UINT16)(SecuredMessageSize - RecordHeaderSize)));
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  if (RecordHeader->Length < AeadTagSize) {
    DEBUG ((DEBUG_ERROR, "RecordHeader->Length is less than AeadTagSize. %x vs %x\n", RecordHeader->Length, AeadTagSize));
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  CipherTextSize = RecordHeader->Length - (UINT16)AeadTagSize;
  if (CipherTextSize > *AppMessageSize) {
    return LIBSPDM_STATUS_BUFFER_TOO_SMALL;
  }

  ZeroMem (*AppMessage, *AppMessageSize); // ???

  Adata  = (UINT8 *)RecordHeader;
  EncMsg = (UINT8 *)(RecordHeader + 1);

  DecMsg = (UINT8 *)*AppMessage;

  Tag = (UINT8 *)RecordHeader + RecordHeaderSize + CipherTextSize;

  Result = SpdmAeadDecryption (
                               0,                                                  // secured_message_context->secured_message_version,
                               SPDM_ALGORITHMS_AEAD_CIPHER_SUITE_AES_256_GCM,      // secured_message_context->aead_cipher_suite,
                               Key,
                               AeadKeySize,
                               Salt,
                               AeadIvSize,
                               Adata,
                               RecordHeaderSize,
                               EncMsg,
                               CipherTextSize,
                               Tag,
                               AeadTagSize,
                               DecMsg,
                               &CipherTextSize
                               );

  if (!Result) {
    DEBUG ((DEBUG_ERROR, "SpdmAeadEncryption failed \n"));
    return LIBSPDM_STATUS_CRYPTO_ERROR;
  }

  EncMsgHeader  = (SPDM_ENC_DEC_MESSAGE_HEADER *)(UINTN)DecMsg;
  PlainTextSize = EncMsgHeader->ApplicationDataLengh;
  if (PlainTextSize > CipherTextSize) {
    DEBUG ((DEBUG_ERROR, " PlainTextSize is more than the CipherTextSize, %x vs %x\n", PlainTextSize, CipherTextSize));
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  if (*AppMessageSize < PlainTextSize) {
    DEBUG ((DEBUG_ERROR, " AppMessageSize is less than the CipherTextSize, %x vs %x\n", *AppMessageSize, CipherTextSize));
    return LIBSPDM_STATUS_INVALID_MSG_SIZE;
  }

  *AppMessage     = (UINT8 *)(UINTN)(EncMsgHeader + 1);
  *AppMessageSize = PlainTextSize;

  return LIBSPDM_STATUS_SUCCESS;
}

/**
 * Check the secure session info tables.
 *
 * @param SecureSessionInfoTable      Contains the information of secure spdm session
 *
 * @retval EFI_SUCCESS                The secure session info table is valid.
 * @retval EFI_INVALID_PARAMETER      The secure session info table is not valid.
*/
STATIC
EFI_STATUS
CheckSecureSessionInfoTable (
  VTPM_SECURE_SESSION_INFO_TABLE  *SecureSessionInfoTable
  )
{
  SPDM_AEAD_AES_256_GCM_KEY_IV_INFO  *KeyIvInfo;

  if (SecureSessionInfoTable->SessionId == 0) {
    return EFI_INVALID_PARAMETER;
  }

  KeyIvInfo = (SPDM_AEAD_AES_256_GCM_KEY_IV_INFO *)(UINTN)(SecureSessionInfoTable + 1);

  if ((ReadUint64 (KeyIvInfo->ReqeustDirection.Key) == 0) || (ReadUint64 (KeyIvInfo->ResponseDirection.Key) == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
 * Encode the input message to secured spdm app message.
 *
 * @param InputMessage                The input message
 * @param InputMessageSize            Size of the input message
 * @param OutputTransportMessage      The output buffer which holds the secured spdm app message
 * @param OutputTransportMessageSize  Size of the output message
 * @param SecureSessionInfoTable      Contains the information of secure spdm session
 *
 * @retval EFI_SUCCESS                The message is encoded successfully.
 * @retval Others                     The message is encoded failed.
*/
EFI_STATUS
VTpmCommEncodeMessage (
  IN UINTN                               InputMessageSize,
  IN UINT8                               *InputMessage,
  IN OUT UINTN                           *OutputTransportMessageSize,
  IN OUT UINT8                           *OutputTransportMessage,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *SecureSessionInfoTable
  )
{
  VTPM_SECURE_SPDM_APP_MESSAGE  *VtpmSecureSpdmAppMessage;

  UINT8  *Message;
  UINTN  MessageSize;

  UINT8  *AppMessage;
  UINTN  AppMessageSize;
  UINT8  *SecuredMessage;
  UINTN  SecuredMessageSize;
  UINT8  *TransportMessage;
  UINTN  TransportMessageSize;

  UINT32       SessionId;
  SPDM_RETURN  SpdmResult;
  EFI_STATUS   Status;

  Status = CheckSecureSessionInfoTable (SecureSessionInfoTable);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CheckSecureSessionInfoTable failed with %r \n", Status));
    return Status;
  }

  ZeroMem (OutputTransportMessage, *OutputTransportMessageSize);

  SessionId                = SecureSessionInfoTable->SessionId;
  VtpmSecureSpdmAppMessage = (VTPM_SECURE_SPDM_APP_MESSAGE *)OutputTransportMessage;
  Message                  = (UINT8 *)(UINTN)&VtpmSecureSpdmAppMessage->Header4 + 1;
  MessageSize              = InputMessageSize;

  CopyMem (Message, InputMessage, InputMessageSize);

  // encode app message
  SpdmResult = LibSpdmVtpmEncodeAppMessage (TRUE, MessageSize, Message, &AppMessageSize, (void **)&AppMessage);
  if (SpdmResult != 0) {
    DEBUG ((DEBUG_ERROR, "LibSpdmVtpmEncodeAppMessage failed with %lx \n", SpdmResult));
    return EFI_ABORTED;
  }

  if (AppMessageSize != (MessageSize + 1)) {
    DEBUG ((DEBUG_ERROR, "AppMessageSize should be equal MessageSize + 1 \n"));
    return EFI_ABORTED;
  }

  if (AppMessage != (UINT8 *)(UINTN)&VtpmSecureSpdmAppMessage->Header4) {
    return EFI_ABORTED;
  }

  // encode secured message
  SecuredMessage     = (UINT8 *)(UINTN)&VtpmSecureSpdmAppMessage->Header2;
  SecuredMessageSize = *OutputTransportMessageSize - sizeof (VTPM_MESSAGE_HEADER);
  SpdmResult         = EncodeSecuredMessage (AppMessage, AppMessageSize, SecuredMessage, &SecuredMessageSize, SecureSessionInfoTable);
  if (SpdmResult != 0) {
    DEBUG ((DEBUG_ERROR, "EncodeSecuredMessage failed with %lx \n", SpdmResult));
    return EFI_ABORTED;
  }

  // random_data is not calculated
  if (SecuredMessageSize <
      (sizeof (SPDM_RECORD_HEADER) +
       sizeof (SPDM_ENC_DEC_MESSAGE_HEADER) +
       AppMessageSize +
       VTPM_SPDM_MAC_LENGTH))
  {
    DEBUG (
           (DEBUG_ERROR,
            "SecuredMessageSize is less than corresponding strcut, %x vs %x\n",
            SecuredMessageSize,
            (sizeof (SPDM_RECORD_HEADER) + sizeof (SPDM_ENC_DEC_MESSAGE_HEADER) + AppMessageSize + VTPM_SPDM_MAC_LENGTH))
           );
    return EFI_UNSUPPORTED;
  }

  // encode message
  SpdmResult = LibSpdmVtpmEncodeMessage (&SessionId, SecuredMessageSize, SecuredMessage, &TransportMessageSize, (void **)&TransportMessage);
  if (SpdmResult != 0) {
    DEBUG ((DEBUG_ERROR, "LibSpdmVtpmEncodeMessage failed with %lx \n", SpdmResult));
    return EFI_ABORTED;
  }

  if (TransportMessageSize > *OutputTransportMessageSize) {
    DEBUG ((DEBUG_ERROR, "TransportMessageSize is more than OutputTransportMessageSize, %x vs %x \n", TransportMessageSize, *OutputTransportMessageSize));
    return EFI_ABORTED;
  }

  if (TransportMessage != (UINT8 *)OutputTransportMessage) {
    return EFI_ABORTED;
  }

  *OutputTransportMessageSize = TransportMessageSize;

  return EFI_SUCCESS;
}

/**
 * Decode the transport message to the message.
 *
 * @param TransportMessage           The transport message
 * @param TransportMessageSize       Size of the transport message
 * @param Message                    The output buffer which holds the message
 * @param MessageSize                Size of the output message
 * @param SecureSessionInfoTable     Contains the information of secure spdm session
 *
 * @retval EFI_SUCCESS                The message is decoded successfully.
 * @retval Others                     The message is decoded failed.
*/
EFI_STATUS
VTpmCommDecodeMessage (
  IN UINTN                               TransportMessageSize,
  IN UINT8                               *TransportMessage,
  IN OUT UINTN                           *MessageSize,
  IN OUT UINT8                           *Message,
  IN OUT VTPM_SECURE_SESSION_INFO_TABLE  *SecureSessionInfoTable
  )
{
  #define VTPM_DEFAULT_MAX_BUFFER_SIZE  0x1000
  SPDM_RETURN  SpdmStatus;
  EFI_STATUS   Status;
  UINT32       *SessionId;
  UINT8        *SecuredMessage;
  UINTN        SecuredMessageSize;
  UINT8        *AppMessage;
  UINTN        AppMessageSize;
  UINT8        Buffer[VTPM_DEFAULT_MAX_BUFFER_SIZE];
  UINTN        BufferSize;
  UINT8        *Ptr;
  UINTN        Size;

  Status = CheckSecureSessionInfoTable (SecureSessionInfoTable);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "CheckSecureSessionInfoTable failed with %r \n", Status));
    return Status;
  }

  SessionId  = NULL;
  BufferSize = VTPM_DEFAULT_MAX_BUFFER_SIZE;
  ZeroMem (Buffer, sizeof (Buffer));

  /* Detect received message*/
  SpdmStatus = LibSpdmVtpmDecodeMessage (
                                         &SessionId,
                                         TransportMessageSize,
                                         TransportMessage,
                                         &SecuredMessageSize,
                                         (VOID **)&SecuredMessage
                                         );
  if (LIBSPDM_STATUS_IS_ERROR (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "LibSpdmVtpmDecodeMessage failed - %lx\n", SpdmStatus));
    return EFI_ABORTED;
  }

  if (SessionId == NULL) {
    return EFI_ABORTED;
  }

  if (*SessionId != SecureSessionInfoTable->SessionId) {
    return EFI_ABORTED;
  }

  // decode secured message to app_message
  AppMessage     = Buffer;
  AppMessageSize = BufferSize;
  SpdmStatus     = DecodeSecuredMessage (SecuredMessage, SecuredMessageSize, &AppMessage, &AppMessageSize, SecureSessionInfoTable);
  if (LIBSPDM_STATUS_IS_ERROR (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "DecodeSecuredMessage - %lx\n", SpdmStatus));
    return EFI_ABORTED;
  }

  /* App message to TPM msg */
  BOOLEAN  IsAppMessage;
  SpdmStatus = LibSpdmVtpmDecodeAppMessage (
                                            &IsAppMessage,
                                            AppMessageSize,
                                            AppMessage,
                                            &Size,
                                            (void **)&Ptr
                                            );
  if (LIBSPDM_STATUS_IS_ERROR (SpdmStatus)) {
    DEBUG ((DEBUG_ERROR, "LibSpdmVtpmDecodeAppMessage - %lx\n", SpdmStatus));
    return EFI_ABORTED;
  }

  if (IsAppMessage == FALSE) {
    return EFI_ABORTED;
  }

  *MessageSize = Size;
  CopyMem (Message, Ptr, Size);

  return EFI_SUCCESS;
}


size_t libspdm_secret_lib_challenge_opaque_data_size;

bool libspdm_encap_challenge_opaque_data(
    spdm_version_number_t spdm_version,
    uint8_t slot_id,
    uint8_t *measurement_summary_hash,
    size_t measurement_summary_hash_size,
    void *opaque_data,
    size_t *opaque_data_size)
{
    size_t index;

    ASSERT(libspdm_secret_lib_challenge_opaque_data_size <= *opaque_data_size);

    *opaque_data_size = libspdm_secret_lib_challenge_opaque_data_size;

    for (index = 0; index < *opaque_data_size; index++)
    {
        ((uint8_t *)opaque_data)[index] = (uint8_t)index;
    }

    return true;
}

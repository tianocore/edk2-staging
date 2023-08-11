/** @file
  Defines the HOB GUID used to pass a TCG_PCR_EVENT or TCG_PCR_EVENT2 from a TPM PEIM to
  a TPM DXE Driver. A GUIDed HOB is generated for each measurement
  made in the PEI Phase.

Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _TCG_EVENT_HOB_H_
#define _TCG_EVENT_HOB_H_

///
/// The Global ID of a GUIDed HOB used to pass a TCG_PCR_EVENT from a TPM PEIM to a TPM DXE Driver.
///
#define EFI_TCG_EVENT_HOB_GUID \
  { \
    0x2b9ffb52, 0x1b13, 0x416f, { 0xa8, 0x7b, 0xbc, 0x93, 0xd, 0xef, 0x92, 0xa8 } \
  }

extern EFI_GUID  gTcgEventEntryHobGuid;

#define EFI_TCG_EVENT2_HOB_GUID \
  { \
    0xd26c221e, 0x2430, 0x4c8a, { 0x91, 0x70, 0x3f, 0xcb, 0x45, 0x0, 0x41, 0x3f } \
  }

extern EFI_GUID  gTcgEvent2EntryHobGuid;

///
/// The Global ID of a GUIDed HOB used to record TPM device error.
///
#define EFI_TPM_ERROR_GUID \
  { \
    0xef598499, 0xb25e, 0x473a, { 0xbf, 0xaf, 0xe7, 0xe5, 0x7d, 0xce, 0x82, 0xc4 } \
  }

extern EFI_GUID  gTpmErrorHobGuid;

///
/// The Global ID of a GUIDed HOB used to record TPM2 Startup Locality.
/// HOB payload is UINT8 according to Startup Locality Event.
///
#define EFI_TPM2_STARTUP_LOCALITY_HOB_GUID \
  { \
    0xef598499, 0xb25e, 0x473a, { 0xbf, 0xaf, 0xe7, 0xe5, 0x7d, 0xce, 0x82, 0xc4 } \
  }

extern EFI_GUID  gTpm2StartupLocalityHobGuid;

///
/// The Global ID of a GUIDed HOB used to record H-CRTM Component Event.
/// HOB payload is the whole TCG_HCRTMComponentEvent according to H-CRTM Component Event.
///
#define EDKII_HCRTM_COMPONENT_EVENT_HOB_GUID \
  { \
    0x7d6515ee, 0x0128, 0x11ee, { 0xa3, 0x1a, 0x98, 0x4f, 0xee, 0x01, 0x46, 0xbf } \
  }

extern EFI_GUID  gEdkiiHCRTMComponentEventHobGuid;

//
// The Global ID of a GUIDed HOB used to record HCRTM Event
// {E3343A38-EB44-DD85-690E-931CE95C11E0}
//
#define EDKII_HCRTM_EVENT_HOB_GUID \
  { 0xe3343a38, 0xeb44, 0xdd85, { 0x69, 0x0e, 0x93, 0x1c, 0xe9, 0x5c, 0x11, 0xe0 } };

extern EFI_GUID  gEdkiiHCRTMEventHobGuid;

//
// The Global ID of a GUIDed HOB used to record vTPM based measurement.
//
#define EDKII_VTPM_BASED_MEASUREMENT_HOB_GUID \
 { 0x908986d7, 0xdcc5, 0x4fa5, { 0x8c, 0x7a, 0xc6, 0x5c, 0x62, 0x31, 0xbe, 0x35 } }

extern EFI_GUID  gEdkiiVTpmBasedMeasurementHobGuid;

///
/// The Global ID of a GUIDed HOB used to record TCG 800-155 PlatformId Event.
/// HOB payload is the whole TCG_Sp800_155_PlatformId_Event2 according to TCG 800-155 PlatformId Event.
///
#define EFI_TCG_800_155_PLATFORM_ID_EVENT_HOB_GUID \
  { \
    0xe2c3bc69, 0x615c, 0x4b5b, { 0x8e, 0x5c, 0xa0, 0x33, 0xa9, 0xc2, 0x5e, 0xd6 } \
  }

extern EFI_GUID  gTcg800155PlatformIdEventHobGuid;

#endif

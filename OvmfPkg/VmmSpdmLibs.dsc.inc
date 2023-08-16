##
#    SPDX-License-Identifier: BSD-2-Clause-Patent
##
!if $(TPM2_ENABLE) == TRUE
!if $(VTPM_ENABLE) == TRUE
   CryptlibWrapper|SecurityPkg/DeviceSecurity/OsStub/CryptlibWrapper/CryptlibWrapper.inf
   MemLibWrapper|SecurityPkg/DeviceSecurity/OsStub/MemLibWrapper/MemLibWrapper.inf
   PlatformLibWrapper|SecurityPkg/DeviceSecurity/OsStub/PlatformLibWrapper/PlatformLibWrapper.inf
   SpdmDeviceSecretLib|OvmfPkg/Library/VTpmTdSecretLib/VTpmTdSecretLib.inf
   SpdmCryptLib|SecurityPkg/DeviceSecurity/SpdmLib/SpdmCryptLib.inf
   SpdmCommonLib|SecurityPkg/DeviceSecurity/SpdmLib/SpdmCommonLib.inf
   SpdmRequesterLib|SecurityPkg/DeviceSecurity/SpdmLib/SpdmRequesterLib.inf
   SpdmSecuredMessageLib|SecurityPkg/DeviceSecurity/SpdmLib/SpdmSecuredMessageLib.inf
!endif
!endif

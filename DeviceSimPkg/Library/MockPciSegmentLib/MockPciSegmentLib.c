
#include <Library/MockPciSegmentLib.h>
#include <Library/MockIoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
MockPciSegmentRegisterAtPciSegmentAddress (
  IN REGISTER_SPACE_MOCK *RegisterSpaceMock,
  IN UINT64              PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  MockIoRegisterMmioAtAddress (RegisterSpaceMock, MockIoTypeMmio, Address, 0x10000);

  return EFI_SUCCESS;
}

EFI_STATUS
MockPciSegmentUnRegisterAtPciSegmentAddress (
  IN UINT64  PciSegmentAddress
  )
{
  UINT64  Address;

  Address = PcdGet64 (PcdPciExpressBaseAddress) + PciSegmentAddress;
  MockIoUnRegisterMmioAtAddress (MockIoTypeMmio, Address);

  return EFI_SUCCESS;
}
#ifndef _MOCK_IO_LIB_H_
#define _MOCK_IO_LIB_H_

#include <Library/IoLib.h>
#include <RegisterSpaceMock.h>

typedef enum {
  MockIoTypeMmio = 0,
  MockIoTypeIo
} MOCK_IO_MEMORY_TYPE;

EFI_STATUS
MockIoRegisterMmioAtAddress (
  IN REGISTER_SPACE_MOCK *RegisterSpaceMock,
  IN MOCK_IO_MEMORY_TYPE  Type,
  IN UINT64               Address,
  IN UINT64               Size
);

EFI_STATUS
MockIoUnRegisterMmioAtAddress (
  IN MOCK_IO_MEMORY_TYPE  Type,
  IN UINT64               Address
  );

#endif
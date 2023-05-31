#ifndef _MAP_BASED_REGISTER_SPACE_LIB_H_
#define _MAP_BASED_REGISTER_SPACE_LIB_H_

#include <Base.h>
#include <RegisterSpaceMock.h>

typedef struct _LOCAL_REGISTER_SPACE LOCAL_REGISTER_SPACE;

typedef
VOID
(*REGISTER_READ_CALLBACK) (
  IN VOID                   *Context,
  IN  UINT64                Address,
  IN  UINT32                ByteEnable,
  OUT UINT32                *Value
  );

typedef
VOID
(*REGISTER_WRITE_CALLBACK) (
  IN VOID                  *Context,
  IN UINT64                Address,
  IN UINT32                ByteEnable,
  IN UINT32                Value
  );

struct _LOCAL_REGISTER_SPACE {
  REGISTER_SPACE_MOCK          RegisterSpace;
  VOID                         *RwContext;
  REGISTER_READ_CALLBACK       Read;
  REGISTER_WRITE_CALLBACK      Write;
};

EFI_STATUS
LocalRegisterSpaceCreate (
  IN CHAR16                       *RegisterSpaceDescription,
  IN REGISTER_WRITE_CALLBACK      Write,
  IN REGISTER_READ_CALLBACK       Read,
  IN VOID                         *RwContext,
  OUT REGISTER_SPACE_MOCK         **SimpleRegisterSpace
  );

EFI_STATUS
LocalRegisterSpaceDestroy (
  IN REGISTER_SPACE_MOCK  *RegisterSpace
  );

UINT32
ByteEnableToBitMask (
  IN UINT32  ByteEnable
  );

#endif
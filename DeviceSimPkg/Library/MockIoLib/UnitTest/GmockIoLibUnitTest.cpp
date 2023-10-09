#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <exception>
#include <Library/GmockIoLib.hpp>
extern "C" {
  #include <Library/FakeRegisterSpaceLib.h>
  #include <Library/RegisterAccessIoLib.h>
}

#define GMOCK_IO_LIB_TEST_DEVICE_NAME  L"RegisterAccessIoLibTestDevice"
#define GMOCK_IO_LIB_TEST_DEVICE_SIZE  0x100
#define GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS 0x10000000

#define FAKE_RET_VAL 0xAB

using ::testing::_;
using ::testing::Return;

VOID
TestRegisterAccessIoDeviceRead (
  IN  VOID    *Context,
  IN  UINT64  Address,
  IN  UINT32  ByteEnable,
  OUT UINT32  *Value
  )
{
  *Value = FAKE_RET_VAL;
}

VOID
TestRegisterAccessIoDeviceWrite (
  IN VOID    *Context,
  IN UINT64  Address,
  IN UINT32  ByteEnable,
  IN UINT32  Value
  )
{
}

class GmockIoLibTest : public ::testing::Test {
  protected:
  void SetUp() override {
    EFI_STATUS Status;
  
    Status = FakeRegisterSpaceCreate ((CHAR16*) GMOCK_IO_LIB_TEST_DEVICE_NAME, FakeRegisterSpaceAlignmentDword, TestRegisterAccessIoDeviceWrite, TestRegisterAccessIoDeviceRead, NULL, &this->RegisterAccess);
    if (EFI_ERROR (Status)) {
      throw new std::bad_alloc();
    }

    Status = RegisterAccessIoRegisterMmioAtAddress (this->RegisterAccess, RegisterAccessIoTypeMmio, GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, GMOCK_IO_LIB_TEST_DEVICE_SIZE);
    if (EFI_ERROR (Status)) {
      throw new std::bad_function_call();
    }

    GmockIoLibSetMock (&IoMock);
  }

  void TearDown() override {
    RegisterAccessIoUnRegisterMmioAtAddress (RegisterAccessIoTypeMmio, GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
    FakeRegisterSpaceDestroy (this->RegisterAccess);
    GmockIoLibUnsetMock ();
  }

  public:
  REGISTER_ACCESS_INTERFACE  *RegisterAccess;
  IoLibMock  IoMock;
};

TEST_F(GmockIoLibTest, ExpectCallTest) {
  EXPECT_CALL(IoMock, IoRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS));
  EXPECT_CALL(IoMock, IoWrite8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, _));
  IoRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  IoWrite8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS, 0);
}

TEST_F(GmockIoLibTest, ExpectCallWithWillReturnTest) {
  EXPECT_CALL(IoMock, IoRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS)).WillOnce(Return(0xAA));
  UINT8 RetVal = IoRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  EXPECT_EQ (RetVal, 0xAA);
}

TEST_F(GmockIoLibTest, ExpectCallWithDelegateToFakeTest) {
  IoMock.DelegateToFake();

  EXPECT_CALL(IoMock, MmioRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS)).Times(1);
  UINT8 RetVal = MmioRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  EXPECT_EQ (RetVal, FAKE_RET_VAL);
}

TEST_F(GmockIoLibTest, DelegateToFakeNoExpectTest) {
  IoMock.DelegateToFake();

  UINT8 RetVal = MmioRead8 (GMOCK_IO_LIB_TEST_DEVICE_MEM_ADDRESS);
  EXPECT_EQ (RetVal, FAKE_RET_VAL);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
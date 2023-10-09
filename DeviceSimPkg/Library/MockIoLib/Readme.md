# GmockIoLib

This library implements GMOCK object for IoLib interface.

## Initializing library

To initialize library first you need to create an object of IoLibMock type and then register it using GmockIoLibSetMock like so:

```
IoLibMock  Mock;

GmockIoLibMockSetMock (Mock);
```

After that you can use a C-library interface to IoLib which will call your registered mock.

Registration can be done in the fixture setup function. After the fixture is done remember to unregister the mock using GmockIoLibUnsetMock.

## Delegate to fake

Library supports delegation of function calls to RegisterAccessIoLib which can be activated by calling the DelegateToFake on IoLibMock object. This allows the test to check the sequence of function calls while still being able to send the read/write cycle to the device model. Fake delegation can
be activated in the following way:

```
TEST_F(SomeFixture, SomeTest) {
  IoMock.DelegateToFake(); // Enables delegation to fake

  EXPECT_CALL (IoMock, MmioRead8).Times(1);
  FunctionUnderTest();
}
```

In the above example test will fail if FunctionUnderTest will not satisfy expect call but the read value will come from the device model instead of the mock.
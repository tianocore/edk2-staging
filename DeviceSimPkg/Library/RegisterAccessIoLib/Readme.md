# RegisterAccess IO lib

This library wraps register access interface to implement EDK2 IoLib interface. Library can work in 2 modes:

1. Standalone mode - compiled by selecting RegisterAccessIoLib.inf
2. "Fake" mode - compiled by selecting FakeRegisterAccessIoLib.inf

The only difference between those 2 modes is that in "Fake" mode every symbol that would normally by expected to be defined by IoLib is
prefixed with "Fake". For instance MmioRead8 becomes FakeMmioRead8. The purpose of this mode is to allow mock implementation with delegation
to fake. You can also use it to decorate RegisterAccessIoLib with your own implementation of IoLib.
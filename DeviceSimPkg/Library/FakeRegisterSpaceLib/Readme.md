# FakeRegisterSpaceLib

## Introduction

The goal of this library is to provide a convinient way to write your own device model.

## Functionalities

The main functionality provided by this library is transforming memory reads/writes to conform to the natural boundaries of the device with correct byte enables set. Normally driver code will access memory by providing address and width. This library will convert that to aligned address and byte enable by performing address alignment and transaction splitting. The hope is that aligned address and byte enables will allow to easily model a device as this is the mode emplyed by majority of HW interconnects(e.g. PCIe, OCP).

### Transaction alignment

When driver code is trying to execute a memory transaction that is not aligned to natural alignment on the device side this library will align the transaction
so that the DeviceRead/DeviceWrite functions will always get transaction aligned to natural boundaries with correct byte enables set. For example:

Memory read at address 0x2 with WORD width will be changed to Memory read at address 0x0(DWORD aligned) with BE 0xC(enable upper 2 bytes).

### Transaction splitting

When driver code is trying to execute a memory read transaction larger then the natural alignment of the device this library will split the transaction into smaller chunks at natural boundaries. For example

Single memory read at address 0x0 with QWORD width will be split into 2 memory reads at address 0x0 and 0x4 with BE set to 0xF(all bytes enabled)

## Modeling a device

### Test code responsibilities

Test code is responsible for providing DeviceRead/DeviceWrite functions which contain device logic. Test code can assume that all accesses to the device passed to it from the FakeRegisterSpaceLib will be aligned to the natural boundary (currently only DWORD alignment is supported) of the device with correct byte enables set. Test code is also responsible for managing all of the device state.

### Limitations

This library assumes a device can be modeled only as a set of memory mapped registers that are accessed synchronously with the driver code. Other considerations typically associated with devices such as DMA, interrupts or asynchronous execution are not provided in this library and have to be provided either by a higher level library or by the test code itself. While limiting following arguments can be used to argue that such an implementation is sufficient for most cases:

1. DMA can be easily provided by higher level library as under OS environment you have unrestricted access to process memory
2. Interrupts are not supported in EDK2 which means majority of the code won't care about it
3. Asynchronous execution of device logic and driver code logic can be provided by the test code however test code will own synchronization between driver thread and device model thread/s.

Given that test code has to provide all of device logic, the library is best used when writing tests for code which interacts with very simple devices that have very little internal logic to limit the test code overhead.
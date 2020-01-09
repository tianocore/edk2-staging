# Testing

UEFI FW Testing Infrastructure supports a few types of testing and this page will help provide some high level info and links for more information.

_Note:_ This document only covers the host-based unit test component, and will grow as more testing features are added.

## Testing Infrastructure - Definitions and Descriptions

### Host-Based Unit Tests 

Host-based unit tests let you compile your unit tests to run as an application in the host.  This method can also leverage "mocking" objects/functions so that unit tests can validate functionality in isolation and force unexpected edge cases.  These unit tests call C functions directly with known parameter sets to force good and bad conditions.  They are linked directly against C code in either a library instance or module. These tests leverage a UnitTest Library.  


#### Library/Protocol/Ppi/Guid Interface

An interface unit test should be written such that it would be valid against any implementation of the library or protocol. For example, an interface test of DxeImageVerification lib should produce valid results for both an OpenSSL implementation and an implementation using another crypto provider. This is not going to always be true for proprietary implementations of certain libraries, such as libraries that intentionally remove some defined functionality, but it should be the ultimate goal of all interface tests.

*Note:* This assertion may not hold true for Null library implementations, which will usually return fixed values.

#### Library/Driver(PEI/DXE/SMM) Implementation

These tests expect a particular implementation of a library or a driver(PEI/DXE/SMM) and may have far more detailed test cases. For example, an implementation test could be written for both a Null instance and/or a proprietary implementation of a given library.

### Host-Based Functionality Tests

#### Tests for a functionality or feature

These tests are written against larger chunks of business logic and may not have obvious divisions along a single library or protocol/ppi/guid interface. Test targets come from multiple folders or packages. Most of these will likely be UEFI-based tests, rather than host-based tests, but it's conceivable that some host-based examples may exist.

### Shared Artifacts

Some testing formats -- especially the host-based unit tests, the fuzzing tests, and the shell-based functional tests -- may find it convenient to share logic in the form of libraries and helpers. These helpers may include mocks and stubs.

#### Shared Mocks (proposed definition)

A true mock is a functional interface that has almost no internal logic and is entirely scripted by the testing engine. An example of this might be a mocked version of GetVariable. When this mocked function is called by the business logic (the logic under test), all it does is ask the test what values it should return. Each test can script the order and content of the return values. As such, this is not an actual variable store, but can easily be used to force certain code paths.

#### Shared Stubs and Other Host Libs (proposed definition)

Stubs are a little more complicated than mocks. Rather than blindly returning values, stubs might have shorthand implementations behind them. In the example above, instead of GetVariable being entirely scripted, you might have an entire VariableServices interface (GetVariable, SetVariable, GetNextVariableName) that is backed by a simple array or other data structure. This stubbed version would behave very similarly to a real variable store, but can be pre-populated with specific contents prior to each test to demonstrate and excercise desired behaviors. Stubs and Mocks can also be combined in some instances to produce intricate behaviors.

## Where Should Things Live?

### Test cases for a library/protocol/ppi/guid interface

If what's being tested is an interface (e.g. a library with a public header file, like DebugLib), the test should be scoped to the parent package.

```
MdePkg/Test/UnitTest/[Library|Protocol|Ppi|Guid]/
```

### Test cases for a library/driver (PEI/DXE/SMM) implementation

If what's being tested is a specific implementation (e.g. BaseDebugLibSerialPort for DebugLib), the test should be scoped to the implementation directory itself.

```
MdePkg/Library/BaseDebugLibSerialPort/UnitTest/
```

### Test cases for a feature under one package

```
Pkg/Test/HostFuncTest/
```

### Test cases for a feature that spans multiple packages.

```
UefiTestPkg/HostFuncTest/
```

Code/Test                                   | Location
---------                                   | --------
`1)` Host-Based Unit Tests for a library/protocol/ppi/guid interface   | In the package that declares the library, protocol, ppi or guid interface in its .DEC file. Should be located in the  <code>*Pkg/Test/UnitTest/[Library&#124;Protocol&#124;Ppi&#124;Guid] directory.                                                            
`2)` Host-Based Unit Tests for a library/driver(PEI/DXE/SMM) implementation   | In the directory that contains the library/driver implementation, in a UnitTest subdirectory. <br /><br /> ============= <br /> Module Example: *Pkg/Universal/EsrtFmpDxe/UnitTest/ <br /> Library Example:  *Pkg/Library/BaseMemoryLib/UnitTest/ <br /> ============= <br /><br />
`3)` Host-Based Fuzz Tests for a library/protocol/ppi/guid interface  | In the package that declares the library, protocol, ppi or guid interface in its .DEC file. Should be located in the  <code>*Pkg/Test/FuzzTest/[Library&#124;Protocol&#124;Ppi&#124;Guid] directory.
`4)` Host-Based Fuzz Tests for a library/driver(PEI/DXE/SMM) implementation   | In the directory that contains the library/driver implementation, in a FuzzTest subdirectory. <br /><br /> ============= <br /> Module Example: *Pkg/Universal/EsrtFmpDxe/FuzzTest/ <br /> Library Example:  *Pkg/Library/BaseMemoryLib/FuzzTest/ <br /> ============= <br /><br />
`5)` Host-Based Tests for a functionality or feature  | If the feature is in one package, should be located in the*Pkg/Test/HostFuncTest directory. <br />(or) <br />If the feature spans multiple packages, should be located in the <br />(or) <br /> UefiTestPkg/HostFuncTest  directory.
`6)` Non-Host-Based (PEI/DXE/SMM/Shell) Tests for a functionality or feature | If the feature is in one package, should be located in the <code>*Pkg/Test/[Shell&#124;Dxe&#124;Smm&#124;Pei]Test</code> directory.   <br /> <br /> (Or) <br />  <br /> if the feature spans multiple packages, should be located in <code>UefiTestPkg/Feature/Functional/[Shell&#124;Dxe&#124;Smm&#124;Pei]Test</code>  <br /><br /> ============= <br /> * PEI Example: MP_SERVICE_PPI. Or check MTRR configuration in a notification function. <br /> * SMM Example: a test in a protocol callback function. (It is different with the solution that SmmAgent+ShellApp) <br /> * DXE Example: a test in a UEFI event call back to check SPI/SMRAM status. <br /> * Shell Example: the SMM handler audit test has a shell-based app that interacts with an SMM handler to get information. The SMM paging audit test gathers information about both DXE and SMM. And the SMM paging functional test actually forces errors into SMM via a DXE driver. <br /> ============= <br /> <br />
`7)` Host-Based Library Implementations                 | Host-Based Implementations of common libraries (eg. MemoryAllocationLibHost) should live in the same package that declares the library interface in its .DEC file in the `*Pkg/HostLibrary` directory. Should have 'Host' in the name.
`8)` Host-Based Mocks and Stubs  | Mock and Stub libraries should live in the `UefiHostTestPkg/Helpers` with either 'Mock' or 'Stub' in the library name.

## Copyright

Copyright (c) Microsoft Corporation.
SPDX-License-Identifier: BSD-2-Clause-Patent


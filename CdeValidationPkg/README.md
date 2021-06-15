# CdeValidationPkg
* [Introduction](README.md#introduction)
* [Intention](README.md#intention)
* [Implementation](README.md#implementation)
  * [Tracing](README.md#tracing)
* [Howto](README.md#howto)
* [Known Bugs](README.md#known-bugs)
* [Revision history](README.md#revision-history)

C Development Environment Validation Package for EDK2

## Introduction
**CdeValidationPkg** is the environment for unit tests  of each single ANSI C library function
implemented and provided for other POST drivers in the [**CdePkg**](../README.md#cdepkg)

The [list of completeness](../implemented.md#validation-status) is updated
by the results of those unit tests.

Furthermore the concept of passing the `EFI_PEI_FILE_HANDLE*` and `EFI_PEI_SERVICES**` parameter to PEI
module's `main()` entry point and `EFI_HANDLE` and `EFI_SYSTEM_TABLE*` for DXE module's `main()` entry point respectively is
demonstrated.

## Intention
The strategy of validating **CdePkg** is to demonstrate, that each single C library function has identical runtime behaviour
when running in UEFI POST or UEFI Shell as if running on Windows NT linked against Microsoft LIBCMT.LIB.

**It is considered the only effective way to reach the ANSI C compatibility and strive for a *faultless implemenation*
within a reasonable amount of time, because by far most parts of each single function test can be run through, debugged and tested natively on the (Windows) development machine. Only final tests need to be run on the UEFI Shell target.**
This proceeding can be reached only by the [OSIF](../README.md#interface-architecture) (Operating System Interface) architecture of the library.

NOTE: **Torito C Library** and CdePkg's **CdeLib** have the same sourcecode base. **Torito C Library**
provides CRT0 for Windows NT and CRT0 for UEFI Shell. In contrast **CdeLib**  provides CRT0 for DXE and PEI
drivers.

The tactical procedure for that intent is:
1.  create test programs that can be built for 
    * Windows NT (with Microsofts LIBCMT.LIB)
    * Windows NT (with Torito C Library)
    * UEFI SHELL (with Torito C Library)
    * UEFI DXE (with CdePkg CdeLib)
    * UEFI PEI (with CdePkg CdeLib)
    
    out of the same source code.
    
2.  issue test status messages utilizing a commonly used trace interface [`CDEMOFINE`](../CdePkg/Include/CDE.h#L56) to 
    `stdout` (WinNT and UEFI SHELL) or to the StatusCode interface / COM1 at 115200,n,8,1 baud.
    
3.  
    1. Capture the trace messages for POST drivers (terminal program or log window of the emulator).
    2. Capture the trace messages from the same source module compiled as Windows NT executable (linked against libcmt.lib)
        by redirecting to `stdout` as reference traces.
    3. Compare both trace logs to verify equality of the **CdeLib** / **Torito C** functions and the original Microsoft implementation.

The tests are kept simple and quick. (A comprehensive validation can not be done in POST, since the
transmission rate of the trace messages is too slow.)<br>
NOTE: **Torito C Library** has its own, comprehensive validation that produces .log files in size > 50 MiB in UEFI SHELL

## Implementation
Each of the VS2019 projects / EDK2 components can be built in:

1. Visual Studio 2019
2. the EDK2 Emulation Build (EmulationPkg)
3. the EDK2 MinnowBoard Build (Vlv2TbltDevicePkg)

### Tracing
 [`CDEMOFINE`](../CdePkg/Include/CDE.h#L56) (**MO**dule **FI**le Li**NE**)was created to provide
 detailed information about origin and location of an event in the trace message, without additional costs
 in maintaining the sourcecode. For that reason [`CDEMOFINE`](../CdePkg/Include/CDE.h#L56) automatically emits drivername, filename, functionname, line of sourcecode and the message type 
 (warning, error, info  etc.) and includes a condition to enable or suppress the emission of the message.

## Howto
1. Start the VS2019 solution **CdeValidationPkg.sln**
2. Notice that each project can be built in 5 different target configurations 
3. Notice that some external EDK configuration files are pulled into the solution that needs to be adjusted
   to place driver binaries into the flash image and to start drivers during POST<br>
  ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled.png)
4. Export the **template** project<br>
   ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled2.png)
   ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled3.png)
   ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled4.png)
5. Create a new project based on that **template**
   ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled5.png)
   ![pci1](https://raw.githubusercontent.com/KilianKegel/pictures/master/Untitled6.png)
6. Get the new project buildable in all target configurations
7. Adjust the .DSC and .FDF files to involve the new driver into EDK2 build<br>
   [EmulatorPkg.dsc](../CdeEmuPkg/EmulatorPkg.dsc#L267)<br>
   [EmulatorPkg.dsc](../CdeEmuPkg/EmulatorPkg.dsc#L305)<br>
   [EmulatorPkg.fdf](../CdeEmuPkg/EmulatorPkg.fdf#L118)<br>
   [EmulatorPkg.fdf](../CdeEmuPkg/EmulatorPkg.fdf#L146)<br>

   [PlatformPkgX64.dsc](../CdeMnwPkg/PlatformPkgX64.dsc#L745)<br>
   [PlatformPkgX64.dsc](../CdeMnwPkg/PlatformPkgX64.dsc#L889)<br>
   [PlatformPkg.fdf](../CdeMnwPkg/PlatformPkg.fdf#L337)<br>
   [PlatformPkg.fdf](../CdeMnwPkg/PlatformPkg.fdf#L414)<br>
8. Add the FILE_GUID / commandline pair to [CdeLoadOptions.h](../CdePkg/Include/CdeLoadOptions.h)
6. build the source tree:
    * For MINNOWBOARD BUILD type:<br>`build -a IA32 -a X64 -n 5 -t VS2015x86 -b DEBUG -p Vlv2TbltDevicePkg\PlatformPkgX64.dsc`
    * For  EMULATION  BUILD type:<br>`build -p EmulatorPkg\EmulatorPkg.dsc -t VS2015x86 -a IA32`
7. Emulation Build run/debug
    * run: `runemu.bat`
    * debug: `dbgemu.bat`<br>
      NOTE: To use `__debugbreak()` the debug engine (VS2019) must be connected to the process *before*
            the breakpoint is reached.

  
## Known bugs
* Torito C Library based programs can not build in 32Bit mode because Torito C is for x86-64
  mode only

## Revision history
### 20191023
* Initial version



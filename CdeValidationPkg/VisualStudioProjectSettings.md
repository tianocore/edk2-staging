## Visual Studio Configuration for the x64 Platform
<table>
        <tr>
            <td></td>
            <td>Windows NT<br>(Msft C Library)</td>
            <td>Windows NT<br>(Torito C Library)</td>
            <td>UEFI DXE<br>(CdeLib)</td>
            <td>UEFI PEI<br>(CdeLib)</td>
            <td>UEFI SHELL<br>(CdeLib)</td>
        </tr>
        <tr><td><th align="center" colspan="5"> <h2>Configuration Properties </h2></th> </td></tr>
        <tr><td>Output directory</td><td align="center" colspan="5"> $(SolutionDir)Edk2\CdeValidationPkg\build\$(Platform)\$(Configuration)</td></tr>
        <tr><td>Target Name</td><td align="center" colspan="5"> $(ProjectName)</td></tr>
        <tr><td>Target Extention</td><td>.exe</td><td>.exe</td><td>.efi</td><td>.efi</td><td>.efi</td>
        <tr>
            <td>Platform Toolset</td>
            <td align="center" colspan="5"> Visual Studio 2017 (v141)</td>
        </tr>
        <tr>
            <td>Configuration Type</td>
            <td align="center" colspan="5">   Application (.exe)</td>
        </tr>
        <tr><td><th align="center" colspan="5"> <h2>C/C++ compiler settings</h2></th> </td></tr>
        <tr>
            <td>Additional Include Directive</td>
            <td align="center" colspan="5">$(SolutionDir)Edk2\CdePkg\include;</td>
        </tr>
        <tr>
            <td>Debug Information Format</td>
            <td align="center" colspan="5">None</td>
        </tr>
        <tr>
            <td>Support Just My Code Debugging</td>
            <td align="center" colspan="5">No</td>
        </tr>
        <tr>
            <td>Warning Level</td>
            <td align="center" colspan="5">Level3 (/W3)</td>
        </tr>
        <tr>
            <td>Treat Warnings As Errors</td>
            <td align="center" colspan="5">Yes (/WX)</td>
        </tr>
        <tr>
            <td>Optimization</td>
            <td align="center" colspan="5">/O1</td>
        </tr>
        <tr>
            <td>Preprocessor Definitions</td>
            <td align="center" colspan="5">_NO_CRT_STDIO_INLINE;<br>_CRT_SECURE_NO_WARNINGS;<br> CDE_DRIVER_NAME="$(TargetName)";</td>
        </tr>
        <tr><td>Enable String Pooling</td><td align="center" colspan="5">Yes /GF</td></tr>
        <tr><td>Enable C++ Exceptions</td><td align="center" colspan="5">No</td></tr>
        <tr><td>Runtime Library</td><td align="center" colspan="5">Multi-threaded /MT</td></tr>
        <tr><td>Struct Alignment</td><td align="center" colspan="5">default</td></tr>
        <tr></tr>
        <tr><td>Security Check</td><td align="center" colspan="5">Disable /GS-</td></tr>
        <tr><td>Treat WChar_t as buildin type</td><td align="center" colspan="5">No</td></tr>
        <tr><td>Precompiled Headers</td><td align="center" colspan="5">No</td></tr>
        <tr><td>calling conventions</td><td align="center" colspan="5">__cdecl(/Gd)</td></tr>
        <tr><td>compile as</td><td align="center" colspan="5">/TC</td></tr>
        <tr><td>Use Full Paths</td><td align="center" colspan="5">No</td></tr>
        <tr><td><th align="center" colspan="5"> <h2>Linker settings</h2></th> </td></tr>
        <tr><td>Output File</td><td align="center" colspan="5">$(OutDir)$(TargetName)$(TargetExt)</td></td></tr>
       <tr><td>Additional Library Directory</td><td align="center" colspan="5">$(SolutionDir)Edk2\CdeValidationPkg\Library\;<br>$(SolutionDir)Edk2\CdePkg\CdeLib\x64\;</td></tr>
        <tr><td>Additional Dependancies</td>
            <td>_CdeMofine.lib;<br>legacy_stdio_definitions.lib;</td>
            <td>ToritoC64R.lib;<br>_CdeMofine.lib;<br>kernel32.lib;</td>
            <td>CdeLib.lib;<br>_CdeMofine.lib;</td>
            <td>CdeLib.lib;<br>_CdeMofine.lib;</td>
            <td>ToritoC64R.lib;<br>_CdeMofine.lib;</td>
        </tr>
        <tr><td>Ignore all default libraries</td>
            <td>No</td>
            <td align="center" colspan="4">Yes (/NODEFAULTLIB) </td>
        </tr>
        <tr><td>Generate Debug Info</td><td align="center" colspan="5">No</td></tr>
        <tr><td>Generate MAP File</td><td align="center" colspan="5">Yes (/MAP)</td></tr>
        <tr><td>SubSystem</td>
            <td align="center" colspan="2">Console (/SUBSYSTEM:CONSOLE)</td>
            <td align="center" colspan="2">EFI Boot Service Driver (/SUBSYSTEM:EFI_BOOT_SERVICE_DRIVER)</td>
            <td>EFI Application (/SUBSYSTEM:EFI_APPLICATION)</td>
        </tr>
        <tr><td>Entry Point</td>
            <td align="center" colspan="1"></td>
            <td align="center" colspan="1">_MainEntryPointWinNT</td>
            <td align="center" colspan="1">_MainEntryPointWinDxe</td>
            <td align="center" colspan="1">_MainEntryPointWinPei</td>
            <td align="center" colspan="1">_MainEntryPointWinShell</td>
        </tr>
        <tr><td>Randomized Base Address</td>
            <td align="center" colspan="5">No (/DYNAMICBASE:NO)</td>
        </tr>        
        <tr><td>Data Execution Prevention</td>
            <td align="center" colspan="5">No (/NXCOMPAT:NO)</td>
        </tr>        
        
    </table>

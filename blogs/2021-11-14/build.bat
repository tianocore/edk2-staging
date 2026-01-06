@echo off
SET BUILDSTART=%TIME%
if /I "%1" == "/clean" (
    if exist *.obj del *.obj > NUL
    if exist *.efi del *.efi > NUL
    goto EOF
)
echo compling...
cl /nologo /W4 /c /GS- /D_NO_CRT_STDIO_INLINE /D_CRT_SECURE_NO_WARNINGS *.c
for %%o in (*.obj) do (
    echo linking %%~no.efi 
    link /nologo /NODEFAULTLIB /ENTRY:_cdeCRT0UefiShell /SUBSYSTEM:EFI_APPLICATION %%~no lib\toroC64.lib
)
echo.
echo BuildStart: %BUILDSTART%
echo BuildEnd:   %TIME%
echo.
:EOF

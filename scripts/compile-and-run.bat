REM Disables the echo
@echo off
REM Define some local variables
SET CUR_PATH=%CD%
SET CUR_DRIVE=%CUR_PATH:~0,2%
SET OS_FULL_PATH=%CUR_PATH%\build\floppy.img
SET COPY_OS_TO_EXTERNAL_DRIVE=0
SET EXTERNAL_DRIVE_PATH=\\?\PhysicalDrive2
SET EXTERNAL_DRIVE_TYPE=2
SET EXTERNAL_DRIVE_SECTORS_TO_COPY=2880
echo PROJECT_PATH -^> %CUR_PATH%
Rem Compile the operating system
ubuntu run "cd /mnt/e/Programming/infinity-os/src; make all; make disk"
Rem (Optional) Copy operating system to a bootable pen-drive
If %COPY_OS_TO_EXTERNAL_DRIVE%==1 If %EXTERNAL_DRIVE_TYPE%==2 (
    echo,
    echo INFO         -^> Copying the Operating System to external drive
    src\tools\imgwrite\tools\PSTools\psexec \\LAPTOP-BQARV16T -accepteula -nobanner -user Administrador -p 91382152 wscript.exe "%CUR_PATH%\src\tools\imgwrite\tools\invisible.vbs" "%CUR_DRIVE% & cd %CUR_PATH% & src\tools\imgwrite\tools\InfinityArchiveTools --write --drive %EXTERNAL_DRIVE_PATH% --drive-type %EXTERNAL_DRIVE_TYPE% --input-file %OS_FULL_PATH% --sectors %EXTERNAL_DRIVE_SECTORS_TO_COPY% & exit" "%CUR_PATH%\src\tools\imgwrite\tools\psexec_output.txt"
    IF EXIST "src\tools\imgwrite\tools\psexec_output.txt" (
        type src\tools\imgwrite\tools\psexec_output.txt
        REM powershell -nologo "& "Get-Content -Wait imgwrite\tools\psexec_output.txt -Tail 10"
        REM more imgwrite\tools\psexec_output.txt
        Del src\tools\imgwrite\tools\psexec_output.txt
        echo,
    )
)
Rem Run compiled operating system in qemu virtual machine
Rem qemu-system-i386 -fda ./build/floppy.img -boot a -s -soundhw pcspk

Rem DEFAULT RUN SCRIPT
qemu-system-i386 -drive format=raw,file=./build/floppy.img -m 1024M 2> NUL

Rem Debug machine
Rem qemu-system-i386 -drive format=raw,file=./build/floppy.img -s -S
Rem gdb -ex "target remote localhost:1234" -ex "symbol-file ./build/kernel/kernel.elf" -ex "br *0x6403586"
Rem --------------------- GDB COMMANDS --------------------
Rem continue - Begin the kernel execution

Rem next  1 - Step program, proceeding through subroutine calls
Rem nexti 1 - Step one instruction, but proceed through subroutine calls
Rem stepi 1 - Step program until it reaches a different source line
Rem stepi 1 - Step one instruction exactly.

Rem Debugging pointers
Rem isr_48 0x64003b7 = 0x64003c8-38
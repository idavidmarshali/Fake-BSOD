@echo off

echo Building Object file ...

gcc -c -o BSOD.o BSOD.c

echo linking ...

gcc -o BSOD.exe BSOD.o C:\Windows\System32\Kernel32.dll C:\Windows\System32\User32.dll C:\Windows\System32\Gdi32.dll C:\Windows\System32\Ole32.dll

echo Removing object file ...

del BSOD.o

echo Done!
PAUSE
@echo off

echo [0;33m[INFO] Building Object file ...

gcc -c -o BSOD.o BSOD.c

echo [INFO] linking ...
IF NOT EXIST build\ (
	mkdir build
)
gcc -o build\BSOD.exe BSOD.o %systemroot%\System32\Kernel32.dll %systemroot%\System32\User32.dll %systemroot%\System32\Gdi32.dll %systemroot%\System32\Ole32.dll

echo [INFO] Removing object file ...

del BSOD.o
IF NOT EXIST build\qr.bmp (
echo [INFO] Copyning Qr code image ...
copy qr.bmp build\
)

echo [0;32m[INFO] build\BSOD.exe Done![0;0m
PAUSE
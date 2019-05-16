@echo off

rem ----------------- pre-define --------------------
rem 'realpad' control the temp.bin file size for padding
rem must be 2^n, 2/4/8/16/32/64...
rem max val depands on boot_limit & app_limit => 2048
rem larger size will cause longer time(gen temp.bin)
set realpad=128
rem boot partition limit1, no flag-page
set /a boot_limit1=28*0x400
rem boot partition limit2, with flag-page
set /a boot_limit2=30*0x400
rem flash total size
set /a flash_size=128*0x400
rem files
set bootm=Extend_360_Pro_Boot
set appm=Extend_360_Pro
set outm=Extend_360_Pro_flash
rem flag(4-byte), mach-type(8-byte) & uuid(16-byte)
set aflag=ARMC
set mtype=00000001
set uuid=0000000000000001
set odir=unknown
set dbg=0

rem ----------------- code start ---------------------
rem Following code NOT need setlocal enabledelayedexpansion

set bootfile=%bootm%.bin
set appfile=%appm%.bin
set outfile=%outm%.bin

if "x%1" == "x" (
	echo "this batch can't be executed directly!"
	exit /b 1
)

if "%outfile%" == "temp.bin" (
	echo "Error: invalid output filename %outfile%"
	exit /b 1
)

if "x%3" == "x" (
	if exist "..\Bin\%2.bin" del /f /q ..\Bin\%2.bin
	%1 --bin -o ..\Bin\%2.bin output\%2.axf
	cd ..\Bin
) else if "%3" == "gcc" (
	cd ..\Bin
	if exist "%2.bin" del /f /q %2.bin
	%1 --gap-fill 0xff -O binary -S ..\Project\obj\release\%2.elf %2.bin
	
) else (
	echo "Error: Not Supported Toolchain %3 Found!"
	exit /b 1
)

if not exist "%2.bin" (
	echo "Error: BIN file generation fail!"
	exit /b 1
)

rem new else-if case must be added for new project
for /f "tokens=1,2 delims=	" %%i in (..\Inc\hw_config.h) do (
	if "%%i" == "#define CFG_MACH_EXTEND_360" (
		for /f "tokens=1 delims= " %%a in ("%%j") do (
			if "%%a" == "1" set odir=Extend_360
		)
	)  else if "%%i" == "#define CFG_DEBUG_UART" (
		for /f "tokens=1 delims= " %%a in ("%%j") do (
			if "%%a" == "1" set dbg=1
		)
	)
)

if not exist "%odir%\." (
	echo "Error: %odir% not exist!"
	exit /b 1
)

if %dbg% equ 1 set odir=%odir%\debug
if %dbg% equ 0 set odir=%odir%\release

copy /b /y %2.bin %odir% > nul
if "%2" == "%bootm%" (
	if not exist "%odir%\%appfile%" (
		echo "Error: file %appfile% not found in dir %odir%!
		exit /b 1
	)

	copy /b /y %odir%\%appfile% . > nul
) else if "%2" == "%appm%" (
	if not exist "%odir%\%bootfile%" (
		echo "Error: file %bootfile% not found in dir %odir%!
		exit /b 1
	)

	copy /b /y %odir%\%bootfile% . > nul
) else (
	echo "Error: Not Supported Module %2!"
	exit /b 1
)

call :fsizechk %bootfile% %boot_limit1%
if errorlevel 1 exit /b 1

set /a var=%flash_size%-%boot_limit2%
call :fsizechk %appfile% %var%
if errorlevel 1 exit /b 1

rem generate emtpy & specified-content file
type nul > temp.bin
>pad.bin set /p="F" < nul
>flag.bin set /p="%aflag%0000%mtype%%uuid%" < nul

rem boot partition padding
set /p="Processing %bootfile%... " < nul
copy /b /y %bootfile% %outfile% > nul
call :gen_pad_file %outfile% temp.bin %boot_limit1%
call :pad_file %outfile% temp.bin %boot_limit1%
echo done

rem flag padding
set /p="Processing flag page... " < nul
copy /b /y %outfile% + flag.bin %outfile% > nul
call :gen_pad_file %outfile% temp.bin %boot_limit2%
call :pad_file %outfile% temp.bin %boot_limit2%
echo done

rem app partition
set /p="Processing %appfile%... " < nul
copy /b /y %outfile% + %appfile% %outfile% > nul
echo done

copy /b /y %outfile% %odir% > nul

rem file info
echo   Name		Size(byte)
echo   ----		----------
call :fileinfo %bootfile%
call :fileinfo %appfile%
call :fileinfo %outfile%

rem clean up
del /f /q temp.bin
del /f /q pad.bin
del /f /q flag.bin

echo "%2.bin & %outfile% in directory Bin\%odir% updated"

echo -------- Success ---------
: pause
exit /b 0

rem function, gen pad file according to file size
:gen_pad_file
	set gsize=%~z1
	
	if %gsize% equ %3 exit /b

	set /a left=%gsize%%%%realpad%
	if %left% equ 0 goto :gen_pad_next
	set /a left=%realpad%-%left%
	set /a left=%gsize%+%left%
	if %left% geq %3 goto :gen_pad_direct
	call :pad_file %1 pad.bin %left%
	
	:gen_pad_next
		call :pad_file %2 pad.bin %realpad%
		exit /b

	:gen_pad_direct
		call :pad_file %1 pad.bin %3
		exit /b

rem function, pad a file to the specified size
:pad_file
	if %~z1 equ %3 exit /b
	copy /b /y %1 + %2 %1 > nul
	goto :pad_file

:fileinfo
	echo %1         %~z1
	exit /b

:fsizechk
	if %~z1 leq %2 exit /b 0
	echo "Error: file %1 size %~z1 exceeds %2 bytes!"
:	pause
	exit /b 1


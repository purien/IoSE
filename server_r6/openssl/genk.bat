@echo off
@setlocal
set IP=%1
set PORT=%2
set SN=%3
set MAX=%4
set VTH=%5
set DELAY=%6
set FEND=%7
set S=0
set CT=%MAX%
set NT=0
set NE=0
:loop
echo %SN%@%IP%:%PORT%
set /a CT = %CT% - 1
if %CT% lss 0 GOTO END
set /a NT = %NT% + 1
set start=%time%
client -S %SN% -H  #c00 -H  #g00  -H  #! -s -p %PORT% -h  %IP%  -l  TLS13-AES128-CCM-SHA256
set end=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%end%")   do set   end_h=%%a&set /a   end_m=100%%b %% 100&set /a   end_s=100%%c %% 100&set /a   end_ms=100%%d %% 100
REM
REM echo %start% %end%
set  hours0=%start_h%
set  mins0=%start_m%
set  secs0=%start_s%
set  ms0=%start_ms%
REM echo %hours0%   %mins0% %secs0%  %ms0%
REM
set /a totalsecs0 = %hours0%*3600 + %mins0%*60 + %secs0%
set /a tm0=%totalsecs0%*1000 + %ms0%*10
REM
set hours1=%end_h%
set mins1=%end_m%
set secs1=%end_s%
set ms1=%end_ms%
REM echo %hours1%   %mins1% %secs1%  %ms1%
REM
set /a totalsecs1 = %hours1%*3600 + %mins1%*60 + %secs1%
set /a tm1=%totalsecs1%*1000 + %ms1%*10
REM
set /a tm=%tm1%-%tm0%
if %tm% lss 0 set /a tm = %tm% + 3600*24*1000
REM
if %tm% lss %VTH% GOTO NOERROR
set /a CT = %CT% + 1
set /a NT = %NT% - 1
set /a NE = %NE% + 1
echo time: %tm%ms total: %S%ms sucess: %NT% errors: %NE%
GOTO wait
REM
:NOERROR
set /a S= %S% + %tm%
echo time: %tm%ms total: %S%ms sucess: %NT% errors: %NE%
GOTO wait
:END
set /a M= %S% / %MAX%
echo mean: %M% ms errors: %NE%
if %FEND% equ 0 PAUSE
REM PAUSE
exit
REM
REM
:wait
set start=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
REM
set hours0=%start_h%
set mins0=%start_m%
set secs0=%start_s%
set ms0=%start_ms%
REM echo %hours0%   %mins0% %secs0%  %ms0%
REM
set /a totalsecs0 = %hours0%*3600 + %mins0%*60 + %secs0%
set /a tm0=%totalsecs0%*1000 + %ms0%*10
REM
:wloop
set end=%time%
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%end%")   do set   end_h=%%a&set /a   end_m=100%%b %% 100&set /a   end_s=100%%c %% 100&set /a   end_ms=100%%d %% 100
REM
set hours1=%end_h%
set mins1=%end_m%
set secs1=%end_s%
set ms1=%end_ms%
REM
set /a totalsecs1 = %hours1%*3600 + %mins1%*60 + %secs1%
set /a tm1=%totalsecs1%*1000 + %ms1%*10
REM
set /a tm=%tm1%-%tm0%
if %tm% lss 0 set /a tm = %tm% + 3600*24*1000
set /a tm=%tm%-%DELAY%
if %tm% geq 0 GOTO loop
GOTO wloop


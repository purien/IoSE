set ip=127.0.0.1
set port=8888
set n=10
set timeout=10000
set delay=0
REM 1=close 0=PAUSE
set fend=0
REM
FOR /L %%X IN (1,1,1)   DO (Start sign.bat %ip% %port% key%%X.com  %n% %timeout%  %delay%  %fend%)

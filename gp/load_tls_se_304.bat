REM https://github.com/martinpaljak/GlobalPlatformPro
gp -delete 0102030405
gp -delete A0000001515350
gp -delete D276000085304A434F9001
gp -list
PAUSE
gp -install .\tls_se\javacard\tls_se.cap  -default
gp -list
PAUSE

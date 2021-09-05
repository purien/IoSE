Getting started
===============

TLS-SE downloading in secure element
====================================
Go to /gp
Start load_tls_se_304.bat

TLS-SE javacard test
====================
Go to /test
Start test.bat

Starting IOSE server
===================
Start IOSE.exe

Using TLS-SE
============
Go to /openssl
Start OPENSSL_TLS_SE_LOCAL_8888.bat
A TLS session is opened
enter ?00 + ENTER (the keystore version is displayed)
enter ?02 + ENTER (this command closes the TLS session)

IOSE Server Administration
==========================
Go to admin
Start list_SCP03.bat to list applications in the secure element
Start helloInstallSCP03.bat to download TLS-SE in the secure element

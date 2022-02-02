# Internet of Secure Elements (IoSE)
![IOSE](https://github.com/purien/IoSE/blob/main/iose2.jpg)

### Getting started
Go to /server_r5

### TLS-SE downloading in secure element
Go to /gp
Start load_tls_se_304.bat

### Test TLS-SE javacard application
Go to /test
Start test.bat

### Starting IOSE server
Start IOSE.exe

### Using TLS-SE
Go to /openssl </br>
Start OPENSSL_TLS_SE_LOCAL_8888.bat </br>
A TLS session is opened </br>
enter ?00 + ENTER (the keystore version is displayed )</br>
enter ?02 + ENTER (this command closes the TLS session)</br>
A list of command is available here https://github.com/purien/IoSE/wiki/TLS-SE--APP-Main-Commands

### IoSE Server Administration
Go to /admin </br>
Start list_SCP03.bat to list apllications in the secure element </br>
Start helloInstallSCP03.bat to download TLS-SE in the secure element </br>
Start_RACS_Console_Local.bat to open a RACS console, enter LIST then ENTER ENTER

### IoSE PCSC RaspberryPi
![IoSE Pi](https://github.com/purien/IoSE/blob/main/iose_pi-small.jpg)

### IOSE I2C RaspberryPi
![IoSE I2C](https://github.com/purien/IoSE/blob/main/iose_i2c_b_small.jpg)

### IOSE Javacard Grid 4x4
![IOSE_I2S_STM32](https://github.com/purien/IoSE/blob/main/gridSTM32s.jpg)

### IOSE Sim Server
![IoSE SMm-Server](https://github.com/purien/IoSE/blob/main/IOSE_SIM_Server_Small.jpg)

### IOSE Infrastructure
![IoSE Infrastructure](https://github.com/purien/IoSE/blob/main/iose_arch_small.jpg)


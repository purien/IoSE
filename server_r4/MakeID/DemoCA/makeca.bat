openssl req -config openssl.cnf -x509 -newkey rsa:2048 -sha256 -out rootcert.pem -outform PEM -keyform PEM -nodes -keyout rootkey.pem -days 40000 -extensions CA_ROOT
openssl x509 -in rootcert.pem -inform PEM -out root.der -outform DER
copy rootcert.pem + rootkey.pem  root.pem
pause


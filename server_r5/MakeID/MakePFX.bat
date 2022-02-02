openssl pkcs12 -export  -inkey clientkey.pem -passin pass:pascal  -in clientcert.pem -certfile root.pem -out cert.pfx
PAUSE


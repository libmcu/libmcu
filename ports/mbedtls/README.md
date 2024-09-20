## Self-signed root CA
### Generate private key

`openssl ecparam -out root_ca.key -name prime256v1 -genkey`

### Create signing request

`openssl req -new -sha256 -key root_ca.key -out root_ca.csr`

### Self-sign the certificate

`openssl x509 -req -sha256 -days 3650 -in root_ca.csr -signkey root_ca.key -out root_ca.crt -extensions v3_ca -extfile <(printf "[v3_ca]\nbasicConstraints=CA:TRUE\nkeyUsage=critical,cRLSign,keyCertSign")`

## Intermediate CA
### Generate private key

`openssl ecparam -out intermediate_ca.key -name prime256v1 -genkey`

### Create signing request

`openssl req -new -sha256 -key intermediate_ca.key -out intermediate_ca.csr`

### Sign the certificate

`openssl x509 -req -sha256 -days 3650 -in intermediate_ca.csr -CA root_ca.crt -CAkey root_ca.key -CAcreateserial -out intermediate_ca.crt -extensions v3_ca -extfile <(printf "[v3_ca]\nbasicConstraints=CA:TRUE,pathlen:2\nkeyUsage=critical,cRLSign,keyCertSign")`

## Non-subordinating CA
### Generate private key

`openssl ecparam -out ca.key -name prime256v1 -genkey`

### Create signing request

`openssl req -new -sha256 -key ca.key -out ca.csr`

### Sign the certificate

`openssl x509 -req -sha256 -days 3650 -in ca.csr -CA intermediate_ca.crt -CAkey intermediate_ca.key -CAcreateserial -out ca.crt -extensions v3_ca -extfile <(printf "[v3_ca]\nbasicConstraints=CA:TRUE,pathlen:0\nkeyUsage=critical,cRLSign,keyCertSign")`

## End entity certificate
### Generate private key

`openssl ecparam -out device.key -name prime256v1 -genkey`

### Create signing request
`openssl req -new -sha256 -key device.key -out device.csr`

### Sign the certificate

`openssl x509 -req -sha256 -days 365 -in device.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out device.crt`

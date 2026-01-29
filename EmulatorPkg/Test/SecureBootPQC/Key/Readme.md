## Steps to generate test keys:

### 1. PK
```
openssl genpkey -algorithm mldsa87 -out PK.key

(
  echo [req]
  echo distinguished_name=dn
  echo [dn]
  echo [v3_ca]
  echo basicConstraints=critical,CA:TRUE
) > tmp_pk.cnf

openssl req -x509 -new -key PK.key -out PK.crt -nodes -days 3650 -subj "/CN=Secure Boot PK/" -config tmp_pk.cnf -extensions v3_ca

openssl x509 -in PK.crt -outform DER -out PK.der

del tmp_pk.cnf
```
### 2. KEK
#### Valid KEK

```
openssl genpkey -algorithm mldsa87 -out KEK.key

openssl req -new -key KEK.key -out KEK.csr -subj "/CN=Secure Boot KEK/"

echo basicConstraints=CA:TRUE > kek_ext.cnf

openssl x509 -req -in KEK.csr -CA PK.crt -CAkey PK.key -CAcreateserial -out KEK.crt -days 3650 -extfile kek_ext.cnf

openssl x509 -in KEK.crt -outform DER -out KEK.der

del kek_ext.cnf
```
#### Invalid KEK
```
openssl genpkey -algorithm mldsa87 -out KEK-invalid.key

(
  echo [req]
  echo distinguished_name=dn
  echo [dn]
  echo [v3_ca]
  echo basicConstraints=critical,CA:TRUE
) > tmp_kek.cnf

openssl req -x509 -new -key KEK-invalid.key -out KEK-invalid.crt -nodes -days 3650 -subj "/CN=Secure Boot KEK-invalid/" -config tmp_kek.cnf -extensions v3_ca

openssl x509 -in KEK-invalid.crt -outform DER -out KEK-invalid.der

del tmp_kek.cnf
```
#### 3. PQC DB
```
openssl genpkey -algorithm mldsa87 -out db-pqc.key

openssl req -new -key db-pqc.key -out db-pqc.csr -subj "/CN=Secure Boot DB (Leaf)/"

openssl x509 -req -in db-pqc.csr -CA KEK.crt -CAkey KEK.key -CAcreateserial -out db-pqc.crt -days 3650 -sha384

openssl x509 -in db-pqc.crt -outform DER -out db-pqc.der

openssl verify -CAfile PK.crt KEK.crt

openssl verify -CAfile PK.crt -untrusted KEK.crt db-pqc.crt
```
#### 4. RSA DB
```
openssl genrsa -out db-rsa.key 2048

openssl req -new -key db-rsa.key -out db-rsa.csr -subj "/CN=RSA2048 Secure Boot DB (Leaf)/"

openssl x509 -req -in db-rsa.csr -CA KEK.crt -CAkey KEK.key -CAcreateserial -out db-rsa.crt -days 3650 -sha384

openssl x509 -in db-rsa.crt -outform DER -out db-rsa.der
```
#### 5. PFX
```
openssl pkcs12 -export -out PK.pfx -inkey PK.key -in PK.crt -name "PK-Sign" -passout pass:123456

openssl pkcs12 -export -out KEK.pfx -inkey KEK.key -in KEK.crt -name "KEK-Sign" -passout pass:123456

openssl pkcs12 -export -out KEK-invalid.pfx -inkey KEK-invalid.key -in KEK-invalid.crt -name "KEK-invalid-Sign" -passout pass:123456

openssl pkcs12 -export -out db-rsa.pfx -inkey db-rsa.key -in db-rsa.crt -name "db-rsa-ImageSign" -passout pass:123456

openssl pkcs12 -export -out db-pqc.pfx -inkey db-pqc.key -in db-pqc.crt -name "db-pqc-ImageSign" -passout pass:123456
```
#### 6. Tool DB
```
openssl genrsa -out db-tool.key 2048

openssl req -new -key db-tool.key -out db-tool.csr -subj "/CN=RSA2048 Secure Boot DB for tools/"

openssl x509 -req -in db-tool.csr -CA KEK.crt -CAkey KEK.key -CAcreateserial -out db-tool.crt -days 3650 -sha384

openssl x509 -in db-tool.crt -outform DER -out db-tool.der

openssl pkcs12 -export -out db-tool.pfx -inkey db-tool.key -in db-tool.crt -name "db-tool-ImageSign" -passout pass:123456
```

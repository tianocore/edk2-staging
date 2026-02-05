## Naming Rule

prefix == content algo, suffix == signature algo.
Multiple algorithms can be concatenated with "-". Algorithm order does not matter.
Composite Algo name should not include "-".

## Steps to generate test keys:

### 1. PK
```
openssl genpkey -algorithm mldsa87 -out MLDSA-PK.key

openssl genrsa -out RSA-PK.key 2048

(
  echo [req]
  echo distinguished_name=dn
  echo [dn]
  echo [v3_ca]
  echo basicConstraints=critical,CA:TRUE
) > tmp_pk.cnf

openssl req -x509 -new -key MLDSA-PK.key -out MLDSA-PK.crt -nodes -days 3650 -subj "/CN=Secure Boot MLDSA PK/" -config tmp_pk.cnf -extensions v3_ca

openssl x509 -in MLDSA-PK.crt -outform DER -out MLDSA-PK.der

openssl req -x509 -new -key RSA-PK.key -out RSA-PK.crt -nodes -days 3650 -subj "/CN=Secure Boot RSA PK/" -config tmp_pk.cnf -extensions v3_ca

openssl x509 -in RSA-PK.crt -outform DER -out RSA-PK.der

del tmp_pk.cnf
```
### 2. KEK
#### Valid KEK

```
echo basicConstraints=CA:TRUE > kek_ext.cnf

openssl genpkey -algorithm mldsa87 -out MLDSA-KEK.key

openssl req -new -key MLDSA-KEK.key -out MLDSA-KEK.csr -subj "/CN=Secure Boot MLDSA KEK/"

openssl x509 -req -in MLDSA-KEK.csr -CA MLDSA-PK.crt -CAkey MLDSA-PK.key -CAcreateserial -out MLDSA-KEK.crt -days 3650 -extfile kek_ext.cnf

openssl x509 -in MLDSA-KEK.crt -outform DER -out MLDSA-KEK.der

openssl genrsa -out RSA-KEK.key 2048

openssl req -new -key RSA-KEK.key -out RSA-KEK.csr -subj "/CN=Secure Boot RSA KEK/"

openssl x509 -req -in RSA-KEK.csr -CA RSA-PK.crt -CAkey RSA-PK.key -CAcreateserial -out RSA-KEK.crt -days 3650 -extfile kek_ext.cnf

openssl x509 -in RSA-KEK.crt -outform DER -out RSA-KEK.der

del kek_ext.cnf
```
#### Invalid KEK
```
openssl genpkey -algorithm mldsa87 -out MLDSA-KEK-invalid.key

(
  echo [req]
  echo distinguished_name=dn
  echo [dn]
  echo [v3_ca]
  echo basicConstraints=critical,CA:TRUE
) > tmp_kek.cnf

openssl req -x509 -new -key MLDSA-KEK-invalid.key -out MLDSA-KEK-invalid.crt -nodes -days 3650 -subj "/CN=Secure Boot MLDSA-KEK-invalid/" -config tmp_kek.cnf -extensions v3_ca

openssl x509 -in MLDSA-KEK-invalid.crt -outform DER -out MLDSA-KEK-invalid.der

del tmp_kek.cnf
```
#### 3. PQC DB
```
openssl genpkey -algorithm mldsa87 -out MLDSA-DB.key

openssl req -new -key MLDSA-DB.key -out MLDSA-DB.csr -subj "/CN=Secure Boot DB (Leaf)/"

openssl x509 -req -in MLDSA-DB.csr -CA MLDSA-KEK.crt -CAkey MLDSA-KEK.key -CAcreateserial -out MLDSA-DB.crt -days 3650 -sha384

openssl x509 -in MLDSA-DB.crt -outform DER -out MLDSA-DB.der

openssl verify -CAfile MLDSA-PK.crt MLDSA-KEK.crt

openssl verify -CAfile MLDSA-PK.crt -untrusted MLDSA-KEK.crt MLDSA-DB.crt
```
#### 4. RSA DB
```
openssl genrsa -out RSA-DB.key 2048

openssl req -new -key RSA-DB.key -out RSA-DB.csr -subj "/CN=RSA2048 Secure Boot DB (Leaf)/"

openssl x509 -req -in RSA-DB.csr -CA MLDSA-KEK.crt -CAkey MLDSA-KEK.key -CAcreateserial -out RSA-DB.crt -days 3650 -sha384

openssl x509 -in RSA-DB.crt -outform DER -out RSA-DB.der
```
#### 5. PFX
```
openssl pkcs12 -export -out MLDSA-PK.pfx -inkey MLDSA-PK.key -in MLDSA-PK.crt -name "PK-Sign" -passout pass:123456

openssl pkcs12 -export -out RSA-PK.pfx -inkey RSA-PK.key -in RSA-PK.crt -name "RSA-PK-Sign" -passout pass:123456

openssl pkcs12 -export -out MLDSA-KEK.pfx -inkey MLDSA-KEK.key -in MLDSA-KEK.crt -name "MLDSA-KEK-Sign" -passout pass:123456

openssl pkcs12 -export -out RSA-KEK.pfx -inkey RSA-KEK.key -in RSA-KEK.crt -name "RSA-KEK-Sign" -passout pass:123456

openssl pkcs12 -export -out MLDSA-KEK-invalid.pfx -inkey MLDSA-KEK-invalid.key -in MLDSA-KEK-invalid.crt -name "MLDSA-MLDSA-KEK-invalid-Sign" -passout pass:123456

openssl pkcs12 -export -out RSA-DB.pfx -inkey RSA-DB.key -in RSA-DB.crt -name "RSA-DB-ImageSign" -passout pass:123456

openssl pkcs12 -export -out MLDSA-DB.pfx -inkey MLDSA-DB.key -in MLDSA-DB.crt -name "MLDSA-DB-ImageSign" -passout pass:123456
```
#### 6. Tool DB
```
openssl genrsa -out RSA-DB-TOOL.key 2048

openssl req -new -key RSA-DB-TOOL.key -out RSA-DB-TOOL.csr -subj "/CN=RSA2048 Secure Boot DB for tools/"

openssl x509 -req -in RSA-DB-TOOL.csr -CA MLDSA-KEK.crt -CAkey MLDSA-KEK.key -CAcreateserial -out RSA-DB-TOOL.crt -days 3650 -sha384

openssl x509 -in RSA-DB-TOOL.crt -outform DER -out RSA-DB-TOOL.der

openssl pkcs12 -export -out RSA-DB-TOOL.pfx -inkey RSA-DB-TOOL.key -in RSA-DB-TOOL.crt -name "RSA-DB-TOOL-ImageSign" -passout pass:123456

openssl genpkey -algorithm mldsa87 -out MLDSA-DB-TOOL.key

openssl req -new -key MLDSA-DB-TOOL.key -out MLDSA-DB-TOOL.csr -subj "/CN=MLDSA Secure Boot DB for tools/"

openssl x509 -req -in MLDSA-DB-TOOL.csr -CA MLDSA-KEK.crt -CAkey MLDSA-KEK.key -CAcreateserial -out MLDSA-DB-TOOL.crt -days 3650 -sha384

openssl x509 -in MLDSA-DB-TOOL.crt -outform DER -out MLDSA-DB-TOOL.der

openssl pkcs12 -export -out MLDSA-DB-TOOL.pfx -inkey MLDSA-DB-TOOL.key -in MLDSA-DB-TOOL.crt -name "MLDSA-DB-TOOL-ImageSign" -passout pass:123456
```

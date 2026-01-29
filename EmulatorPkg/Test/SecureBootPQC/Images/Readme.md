## Steps to generate test images:

### 1. DB signed EFI image
```
copy HelloWorld.efi HelloWorld-RSA.efi

signtool sign /f ../Key/db-rsa.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-RSA.efi

copy HelloWorld.efi HelloWorld-MLDSA.efi

signtool sign /f ../Key/db-pqc.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-MLDSA.efi
```

### 2. DB tool key signed SecureBootUpdate.efi
MUST enroll tool key to db before enrolling PK(enabling secure boot), to make sure SecureBootUpdate can be run in standard mode.
```
copy SecureBootUpdate.efi SecureBootUpdateSigned.efi

signtool sign /f ../Key/db-tool.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v SecureBootUpdateSigned.efi
```

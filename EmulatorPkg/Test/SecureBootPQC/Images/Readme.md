## Steps to generate test images:

### 1. DB signed EFI image
```
copy HelloWorld.efi HelloWorld-RSA.efi

signtool sign /f ../Key/RSA-DB.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-RSA.efi

copy HelloWorld.efi HelloWorld-MLDSA.efi

signtool sign /f ../Key/MLDSA-DB.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-MLDSA.efi

copy HelloWorld.efi HelloWorld-RSA-MLDSA.efi

# First, sign with RSA key
signtool sign /f ../Key/RSA-DB.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-RSA-MLDSA.efi

# Then sign again with pqc key by /as flag
# /as  Append this signature. If no primary signature is present, this signature will be made the primary signature instead.
signtool sign /as /f ../Key/MLDSA-DB.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v HelloWorld-RSA-MLDSA.efi
```

### 2. DB tool key signed SecureBootUpdate.efi
MUST enroll tool key to db before enrolling PK(enabling secure boot), to make sure SecureBootUpdate can be run in standard mode.
```
copy SecureBootUpdate.efi SecureBootUpdateSigned.efi

signtool sign /f ../Key/RSA-DB-TOOL.pfx /p 123456 /fd sha384 /tr http://timestamp.digicert.com /td sha384 /v SecureBootUpdateSigned.efi
```

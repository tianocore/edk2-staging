@echo -off
#
# Test12-MLDSA-Hash-RSA-Fail.nsh
# Enroll only the SHA-384 hash of the MLDSA-DB certificate's TBSCertificate into db,
# then verify that an RSA-signed image is rejected (Expected FAIL).
#
# The RSA signing certificate's TBS hash is NOT enrolled, so the image should not
# be allowed by the EFI_CERT_X509_SHA384 path in IsAllowedByDb.
#

echo " "
echo "==========================================="
echo "  Test 15: Enroll MLDSA cert hash to db and run RSA-signed image (Expected FAIL)"
echo "==========================================="
echo " "

set TOOL Images\SecureBootUpdate.efi
set SIGNED_TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%SIGNED_TOOL% clear

%TOOL% update-db-hash Key\MLDSA-DB.der
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth

Images\HelloWorld-RSA.efi

echo " "

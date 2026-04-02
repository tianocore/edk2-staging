@echo -off
#
# Test11-MLDSA-Hash-Success.nsh
# Enroll the SHA-384 hash of the MLDSA-DB certificate's TBSCertificate into db,
# then verify that an ML-DSA-signed image is accepted (Expected SUCCESS).
#
# This exercises the EFI_CERT_X509_SHA384 allow-list path in IsAllowedByDb.
#

echo " "
echo "==========================================="
echo "  Test 14: Enroll MLDSA cert hash to db and run ML-DSA-signed image (Expected SUCC)"
echo "==========================================="
echo " "

set TOOL Images\SecureBootUpdate.efi
set SIGNED_TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%SIGNED_TOOL% clear

%TOOL% update-db-hash Key\RSA-DB.der
%TOOL% update-db-hash Key\MLDSA-DB.der
%TOOL% update-db Key\MLDSA-DB-TOOL.der

%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth

Images\HelloWorld-MLDSA.efi

echo " "

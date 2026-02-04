@echo -off
#
# Test2-RSA.nsh
# Test 5: RSA signature verification
#

echo " "
echo "==========================================="
echo "  Test 5: RSA Signature Verification"
echo "==========================================="
echo " "

set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

Images\HelloWorld-RSA.efi
echo " "

@echo -off
#
# Test8-DualSign.nsh
# Test enroll RSA or MLDSA key to db and run dual signed image, should success
#

echo " "
echo "==========================================="
echo "  Test 11: Test enroll RSA or MLDSA key to db and run dual signed image (Expected SUCC)"
echo "==========================================="
echo " "
set TOOL Images\SecureBootUpdateSigned.efi

%TOOL% clear
%TOOL% update-db Key\db-tool.der
%TOOL% update-pk Key\PK.der
%TOOL% update-kek AuthVars\KEK.auth
%TOOL% update-db AuthVars\db-pqc.auth
Images\HelloWorld-DualSig.efi

%TOOL% clear
%TOOL% update-db Key\db-tool.der
%TOOL% update-pk Key\PK.DER
%TOOL% update-kek AuthVars\KEK.auth
%TOOL% update-db AuthVars\db-rsa.auth
Images\HelloWorld-DualSig.efi

echo " "

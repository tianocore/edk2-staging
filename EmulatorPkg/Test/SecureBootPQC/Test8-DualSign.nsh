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
set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%TOOL% clear
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth
%TOOL% update-kek AuthVars\MLDSA-KEK-MLDSA.auth
%TOOL% update-db AuthVars\MLDSA-DB-MLDSA.auth
Images\HelloWorld-RSA-MLDSA.efi

%TOOL% clear
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth
%TOOL% update-kek AuthVars\MLDSA-KEK-MLDSA.auth
%TOOL% update-db AuthVars\RSA-DB-MLDSA.auth
Images\HelloWorld-RSA-MLDSA.efi

echo " "

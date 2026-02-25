@echo -off
#
# Test9-Certchain.nsh
# Test enroll MLDSA CA key to db and run image signed by leaf, should success
#

echo " "
echo "==========================================="
echo "  Test 12: Test enroll MLDSA CA key to db and run image signed by leaf (Expected SUCC)"
echo "==========================================="
echo " "
set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%TOOL% clear
%TOOL% update-db Key\MLDSA-DB-CA.der
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth
Images\HelloWorld-LEAF-MLDSA.efi

echo " "

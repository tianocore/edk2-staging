@echo -off
#
# Test9-Certchain.nsh
# Test append KEK which signed by another KEK, should success
#

echo " "
echo "==========================================="
echo "  Test 13: Append KEK which signed by another KEK (Expected SUCC)"
echo "==========================================="
echo " "
set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%TOOL% clear
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth
# Set KEK1
%TOOL% update-kek AuthVars\MLDSA-KEK-MLDSA.auth
# Append KEK2 which signed by KEK1
%TOOL% update-kek AuthVars\RSA-KEK-SignedByKEK-MLDSA.auth
# Append DB which signed by KEK2
%TOOL% update-db AuthVars\RSA-DB-RSA.auth

Images\HelloWorld-RSA.efi

echo " "

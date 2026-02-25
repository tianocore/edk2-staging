@echo -off
#
# Test7-Clear-db.nsh
# Test clear db and then run MLDSA signed image, should fail
#

echo " "
echo "==========================================="
echo "  Test 10: Clear db and then run MLDSA signed image (Expected FAIL)"
echo "==========================================="
echo " "
set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

%TOOL% clear
%TOOL% update-db Key\MLDSA-DB-TOOL.der
%TOOL% update-pk AuthVars\MLDSA-PK-MLDSA.auth
Images\HelloWorld-MLDSA.efi

echo " "

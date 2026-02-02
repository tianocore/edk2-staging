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
set TOOL Images\SecureBootUpdateSigned.efi

%TOOL% clear
%TOOL% update-db Key\db-tool.der
%TOOL% update-pk Key\PK.der
Images\HelloWorld-MLDSA.efi

echo " "

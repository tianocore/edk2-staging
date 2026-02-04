@echo -off
#
# Test4-EnrollPQC.nsh
# Enroll ML-DSA certificate and test
#

echo " "
echo "==========================================="
echo "  Test 7: ML-DSA (PQC) Certificate Enrollment"
echo "==========================================="
echo " "

set TOOL Images\SecureBootUpdateSigned.efi

echo "[INFO] Enrolling MLDSA-DB-MLDSA.auth (ML-DSA certificate)..."
%TOOL% update-db AuthVars\MLDSA-DB-MLDSA.auth
if %lasterror% eq 0 then
    echo "[PASS] Test 7: MLDSA-DB-MLDSA.auth enrollment (APPEND mode)"
else
    echo "[FAIL] Test 7: MLDSA-DB-MLDSA.auth enrollment (error: %lasterror%)"
endif
echo " "

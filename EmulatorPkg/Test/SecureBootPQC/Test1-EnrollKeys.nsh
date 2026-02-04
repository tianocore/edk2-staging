@echo -off
#
# Test1-EnrollKeys.nsh
# Tests 1-4: Key enrollment scenarios
#

set TOOL Images\SecureBootUpdate-RSA-MLDSA.efi

#============================================
# Test Case 1: Try to enroll DB directly without KEK (should fail)
#============================================
echo "[INFO] This should FAIL because KEK is not enrolled yet"
echo " "

%TOOL% update-db AuthVars\RSA-DB-MLDSA.auth
if %lasterror% eq 0 then
    echo "[FAIL] Test 1: RSA-DB-MLDSA.auth enrollment without KEK (expected to fail but succeeded)"
else
    echo "[PASS] Test 1: RSA-DB-MLDSA.auth enrollment without KEK (correctly failed)"
endif
echo " "

#============================================
# Test Case 2: Try to enroll MLDSA-KEK.der (should fail)
#============================================
echo "[INFO] This should FAIL because we're in User Mode"
echo " "

%TOOL% update-kek Key\MLDSA-KEK.der
if %lasterror% eq 0 then
    echo "[FAIL] Test 2: MLDSA-KEK.der enrollment without authentication (expected to fail but succeeded)"
else
    echo "[PASS] Test 2: MLDSA-KEK.der enrollment without authentication (correctly failed)"
endif
echo " "

#============================================
# Test Case 3: Enroll MLDSA-KEK-MLDSA.auth (should succeed)
#============================================
echo "[INFO] This should SUCCEED because MLDSA-KEK-MLDSA.auth is signed by PK"
echo " "

%TOOL% update-kek AuthVars\MLDSA-KEK-MLDSA.auth
if %lasterror% eq 0 then
    echo "[PASS] Test 3: MLDSA-KEK-MLDSA.auth enrollment with valid signature"
else
    echo "[FAIL] Test 3: MLDSA-KEK-MLDSA.auth enrollment with valid signature (error: %lasterror%)"
endif
echo " "

#============================================
# Test Case 4: Enroll RSA-DB-MLDSA.auth (should succeed now)
#============================================
echo "[INFO] Now that KEK is enrolled, RSA-DB-MLDSA.auth should work"
echo " "

%TOOL% update-db AuthVars\RSA-DB-MLDSA.auth
if %lasterror% eq 0 then
    echo "[PASS] Test 4: RSA-DB-MLDSA.auth enrollment with KEK"
else
    echo "[FAIL] Test 4: RSA-DB-MLDSA.auth enrollment (error: %lasterror%)"
endif
echo " "

echo "Test1-EnrollKeys.nsh completed."
echo " "

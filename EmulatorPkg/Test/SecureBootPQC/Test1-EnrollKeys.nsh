@echo -off
#
# Test1-EnrollKeys.nsh
# Tests 1-4: Key enrollment scenarios
#

set TOOL Images\SecureBootUpdateSigned.efi

#============================================
# Test Case 1: Try to enroll DB directly without KEK (should fail)
#============================================
echo "[INFO] This should FAIL because KEK is not enrolled yet"
echo " "

%TOOL% update-db AuthVars\db-rsa.auth
if %lasterror% eq 0 then
    echo "[FAIL] Test 1: db-rsa.auth enrollment without KEK (expected to fail but succeeded)"
else
    echo "[PASS] Test 1: db-rsa.auth enrollment without KEK (correctly failed)"
endif
echo " "

#============================================
# Test Case 2: Try to enroll KEK.der (should fail)
#============================================
echo "[INFO] This should FAIL because we're in User Mode"
echo " "

%TOOL% update-kek Key\KEK.der
if %lasterror% eq 0 then
    echo "[FAIL] Test 2: KEK.der enrollment without authentication (expected to fail but succeeded)"
else
    echo "[PASS] Test 2: KEK.der enrollment without authentication (correctly failed)"
endif
echo " "

#============================================
# Test Case 3: Enroll KEK.auth (should succeed)
#============================================
echo "[INFO] This should SUCCEED because KEK.auth is signed by PK"
echo " "

%TOOL% update-kek AuthVars\KEK.auth
if %lasterror% eq 0 then
    echo "[PASS] Test 3: KEK.auth enrollment with valid signature"
else
    echo "[FAIL] Test 3: KEK.auth enrollment with valid signature (error: %lasterror%)"
endif
echo " "

#============================================
# Test Case 4: Enroll db-rsa.auth (should succeed now)
#============================================
echo "[INFO] Now that KEK is enrolled, db-rsa.auth should work"
echo " "

%TOOL% update-db AuthVars\db-rsa.auth
if %lasterror% eq 0 then
    echo "[PASS] Test 4: db-rsa.auth enrollment with KEK"
else
    echo "[FAIL] Test 4: db-rsa.auth enrollment (error: %lasterror%)"
endif
echo " "

echo "Test1-EnrollKeys.nsh completed."
echo " "

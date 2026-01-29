@echo -off
#
# RunAllTests.nsh
# Master script to run all Secure Boot PQC tests
#
# Prerequisites:
#   1. Run EnableSecureBoot.nsh first (enrolls PK and db-tool)
#   2. Run from Test\SecureBootPQC directory
#
# Usage:
#   fs0:
#   cd Test\SecureBootPQC
#   EnableSecureBoot.nsh    (first time setup)
#   RunAllTests.nsh         (run all tests)
#

echo " "
echo "==========================================="
echo "  Secure Boot PQC Test Suite"
echo "==========================================="
echo " "

# Check prerequisites
if not exist Images\SecureBootUpdateSigned.efi then
    echo "[FAIL] Required files not found!"
    goto End
endif

echo "[INFO] Prerequisites check..."
Images\SecureBootUpdateSigned.efi status
echo " "

Test1-EnrollKeys.nsh

Test2-RSA.nsh
if %lasterror% eq 0 then
    echo "[PASS] Test 5: Run RSA-signed HelloWorld"
else
    echo "[FAIL] Test 5: Run RSA-signed HelloWorld fail (error: %lasterror%)"
endif

Test3-MLDSA-Fail.nsh
if %lasterror% eq 0 then
    echo "[FAIL] Test 6: Run HelloWorld-MLDSA should fail before enroll pqc key but succ"
else
    echo "[PASS] Test 6: Run HelloWorld-MLDSA failed before enroll pqc key (correctly failed)"
endif

Test4-EnrollPQC.nsh

Test5-MLDSA-Success.nsh
if %lasterror% eq 0 then
    echo "[PASS] Test 8: Run ML-DSA-signed HelloWorld after enroll pqc key to db"
else
    echo "[FAIL] Test 8: Run ML-DSA-signed HelloWorld after enroll pqc key to db (error: %lasterror%)"
endif

Test6-Unsigned.nsh
if %lasterror% eq 0 then
    echo "[FAIL] Test 9: Run unsigned HelloWorld should fail but succ"
else
    echo "[PASS] Test 9: Run unsigned HelloWorld failed (correctly failed)"
endif

echo " "
echo "========================================="
echo "  All Tests Completed"
echo "========================================="
echo "Check the output above for individual test results."
echo " "

UDF:
-- SeedGenUdf.py: Generate a simple UDF partition as seed
     python SeedGenUdf.py -o Seed\Udf.bin
-- DumpUdf.py: Dump the UDF binary
     python DumpUdf.py Seed\Udf.bin
     python DumpUdf.py Seed\Udf.bin --Lba 0x100
-- Udf.py: UDF definition

Mutator:
-- MutatorSimple.py: Randomize UINT8, UINT16, UINT32, UINT64 in a given buffer
     python MutatorSimple.py Seed\XXX.bin -e TestXXX.exe

Include:
-- Uefi.py: UEFI definition

TPM:
-- SeedGenTpm2Response.py

====================================================================================
                                 Mapping List
====================================================================================
Case Name:                                 Seed Location:
TestTpm2CommandLib                         HBFA\UefiHostFuzzTestCasePkg\Seed\TPM\Raw
TestBmpSupportLib                          HBFA\UefiHostFuzzTestCasePkg\Seed\BMP\Raw
TestPartition                              HBFA\UefiHostFuzzTestCasePkg\Seed\UDF\Raw\Partition
TestUdf                                    HBFA\UefiHostFuzzTestCasePkg\Seed\UDF\Raw\FileSystem
TestUsb                                    HBFA\UefiHostFuzzTestCasePkg\Seed\USB\Raw
TestPeiUsb                                 HBFA\UefiHostFuzzTestCasePkg\Seed\USB\Raw
TestDxeCapsuleLibFmp                       HBFA\UefiHostFuzzTestCasePkg\Seed\Capsule
TestVariableSmm                            HBFA\UefiHostFuzzTestCasePkg\Seed\VariableSmm\Raw
TestFmpAuthenticationLibPkcs7              HBFA\UefiHostFuzzTestCasePkg\Seed\Capsule
TestFmpAuthenticationLibRsa2048Sha256      HBFA\UefiHostFuzzTestCasePkg\Seed\Capsule
TestCapsuePei                              HBFA\UefiHostFuzzTestCasePkg\Seed\Capsule
TestUpdateLockBoxFuzzLength                HBFA\UefiHostFuzzTestCasePkg\Seed\LockBox\Raw
TestUpdateLockBoxFuzzOffset                HBFA\UefiHostFuzzTestCasePkg\Seed\LockBox\Raw
TestFileName                               HBFA\UefiHostFuzzTestCasePkg\Seed\UDF\Raw\FileName
TestPeiGpt                                 HBFA\UefiHostFuzzTestCasePkg\Seed\Gpt\Raw
TestValidateTdxCfv                         HBFA\UefiHostFuzzTestCasePkg\Seed\Cfv
TestHobList                                HBFA\UefiHostFuzzTestCasePkg\Seed\TdxHob
TestTcg2MeasureGptTable                    HBFA\UefiHostFuzzTestCasePkg\Seed\Gpt
TestTcg2MeasurePeImage                     # PE format image
TestVirtioPciDevice                        HBFA\UefiHostFuzzTestCasePkg\Seed\Blk
TestVirtio10Blk                            HBFA\UefiHostFuzzTestCasePkg\Seed\Blk
TestVirtioBlk                              HBFA\UefiHostFuzzTestCasePkg\Seed\Blk
TestVirtioBlkReadWrite                     HBFA\UefiHostFuzzTestCasePkg\Seed\Blk
TestParseMmioExitInstructions              HBFA\UefiHostFuzzTestCasePkg\Seed\Instruction
# Fuzzy cases for MbedTls API 
TestBigNumIsOdd                            HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestBigNumIsOdd                             
TestBigNumRShift                           HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestBigNumRShift                           
TestAeadAesGcmEncryptIv                    HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmEncryptIv                    
TestAeadAesGcmEncryptKye                   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmEncryptKye                   
TestAeadAesGcmEncryptAData                 HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmEncryptAData
TestAeadAesGcmEncryptDataIn                HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmEncryptDataIn
TestAeadAesGcmEncryptTagSize               HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmEncryptTagSize                 
TestAeadAesGcmDecryptIv                    HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmDecryptIv                      
TestAeadAesGcmDecryptKye                   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmDecryptKye                            
TestAeadAesGcmDecryptTag                   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmDecryptTag                          
TestAeadAesGcmDecryptAData                 HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmDecryptAData                   
TestAeadAesGcmDecryptDataIn                HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAeadAesGcmDecryptDataIn                  
TestEcPointSetCompressedCoordinates        HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestEcPointSetCompressedCoordinates          
TestEcPointSetCompressedCoordinatesBnX     HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestEcPointSetCompressedCoordinatesBnX                                             
TestPkcs7GetAttachedContent                HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7GetAttachedContent                              
TestPkcs7Verify                            HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7Verify                                   
TestPkcs7GetSigners                        HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7GetSigners                               
TestPkcs7SignCert                          HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7SignCert                                 
TestPkcs7SignInData                        HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7SignInData                               
TestPkcs7SignPrivateKey                    HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7SignPrivateKey  
TestPkcs7SignKeyPassword                   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestPkcs7SignKeyPassword  
TestAuthenticodeVerifyAuthData     	   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAuthenticodeVerifyAuthData     
TestAuthenticodeVerifyImageHash    	   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAuthenticodeVerifyImageHash    
TestAuthenticodeVerifyTrustedCert  	   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestAuthenticodeVerifyTrustedCert  
TestImageTimestampVerifyAuthData   	   HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestImageTimestampVerifyAuthData   
TestImageTimestampVerifyTsaCert            HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestImageTimestampVerifyTsaCert         
TestVerifyEKUsInPkcs7Signature	        HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestVerifyEKUsInPkcs7Signature
TestEcPointAdd   	                       HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestEcPointAdd   
TestEcPointInvert     	                  HBFA/UefiHostFuzzTestCasePkg/Seed/Mbedtls/TestEcPointInvert
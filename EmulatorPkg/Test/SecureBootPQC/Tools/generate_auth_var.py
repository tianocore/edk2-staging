## @file
# Generate EFI Authenticated Variable Update File (.auth)
#
# This script creates an EFI_VARIABLE_AUTHENTICATION_2 structure for updating
# Secure Boot variables (PK, KEK, DB, DBX) in User Mode.
#
# Copyright (c) 2026, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

"""
Generate EFI Authenticated Variable Update File (.auth)

This script creates an EFI_VARIABLE_AUTHENTICATION_2 structure for updating
Secure Boot variables (PK, KEK, DB, DBX) in User Mode.

Workflow:
    1. Create EFI_SIGNATURE_LIST from certificate
    2. Prepare data to sign (VariableName + GUID + Attributes + TimeStamp + Data)
    3. Use Microsoft SignTool to generate PKCS#7 signature
    4. Assemble EFI_VARIABLE_AUTHENTICATION_2 structure

Usage:
    python generate_auth_var.py --cert KEK.cer --key PK.pfx --password <pwd> --var-name KEK --output KEK.auth
    
Requirements:
    - Python 3.7+
    - Microsoft SignTool.exe (must be in PATH environment variable)
"""

import struct
import hashlib
import uuid
import argparse
import subprocess
import tempfile
import os
from datetime import datetime, timezone
from pathlib import Path

# GUID definitions
EFI_CERT_X509_GUID = uuid.UUID('a5c059a1-94e4-4aa7-87b5-ab155c2bf072')
EFI_CERT_PKCS7_GUID = uuid.UUID('4aafd29d-68df-49ee-8aa9-347d375665a7')
EFI_GLOBAL_VARIABLE_GUID = uuid.UUID('8BE4DF61-93CA-11d2-AA0D-00E098032B8C')
EFI_IMAGE_SECURITY_DATABASE_GUID = uuid.UUID('d719b2cb-3d3a-4596-a3bc-dad00e67656f')

# Variable names
VAR_NAMES = {
    'PK': ('PK', EFI_GLOBAL_VARIABLE_GUID),
    'KEK': ('KEK', EFI_GLOBAL_VARIABLE_GUID),
    'db': ('db', EFI_IMAGE_SECURITY_DATABASE_GUID),
    'DB': ('db', EFI_IMAGE_SECURITY_DATABASE_GUID),
    'dbx': ('dbx', EFI_IMAGE_SECURITY_DATABASE_GUID),
    'DBX': ('dbx', EFI_IMAGE_SECURITY_DATABASE_GUID),
}

# WIN_CERTIFICATE types
WIN_CERT_TYPE_EFI_GUID = 0x0EF1
WIN_CERT_REVISION_2_0 = 0x0200

# EFI Variable Attributes
EFI_VARIABLE_NON_VOLATILE = 0x00000001
EFI_VARIABLE_BOOTSERVICE_ACCESS = 0x00000002
EFI_VARIABLE_RUNTIME_ACCESS = 0x00000004
EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS = 0x00000020


def create_efi_time(dt=None):
    """
    Create EFI_TIME structure (16 bytes)
    
    typedef struct {
      UINT16  Year;       // 1900 – 9999
      UINT8   Month;      // 1 – 12
      UINT8   Day;        // 1 – 31
      UINT8   Hour;       // 0 – 23
      UINT8   Minute;     // 0 – 59
      UINT8   Second;     // 0 – 59
      UINT8   Pad1;
      UINT32  Nanosecond; // 0 – 999,999,999
      INT16   TimeZone;   // -1440 to 1440 or 2047
      UINT8   Daylight;
      UINT8   Pad2;
    } EFI_TIME;
    """
    if dt is None:
        dt = datetime.now(timezone.utc)
    
    return struct.pack(
        '<HBBBBBBIHBB',
        dt.year,        # UINT16 Year
        dt.month,       # UINT8 Month
        dt.day,         # UINT8 Day
        dt.hour,        # UINT8 Hour
        dt.minute,      # UINT8 Minute
        dt.second,      # UINT8 Second
        0,              # UINT8 Pad1
        0,              # UINT32 Nanosecond
        0,              # INT16 TimeZone (UTC)
        0,              # UINT8 Daylight
        0               # UINT8 Pad2
    )


def create_signature_list(cert_data, owner_guid=None):
    """
    Create EFI_SIGNATURE_LIST structure
    
    typedef struct {
      EFI_GUID  SignatureType;
      UINT32    SignatureListSize;
      UINT32    SignatureHeaderSize;
      UINT32    SignatureSize;
      // UINT8  SignatureHeader[SignatureHeaderSize];
      // EFI_SIGNATURE_DATA Signatures[...];
    } EFI_SIGNATURE_LIST;
    
    typedef struct {
      EFI_GUID  SignatureOwner;
      UINT8     SignatureData[...];
    } EFI_SIGNATURE_DATA;
    """
    if owner_guid is None:
        owner_guid = uuid.UUID(int=0)  # Zero GUID
    
    signature_size = 16 + len(cert_data)  # sizeof(EFI_GUID) + cert_data
    signature_list_size = 28 + signature_size  # sizeof(EFI_SIGNATURE_LIST) + signature_size
    
    # EFI_SIGNATURE_LIST header
    sig_list = struct.pack(
        '<16sIII',
        EFI_CERT_X509_GUID.bytes_le,  # SignatureType
        signature_list_size,            # SignatureListSize
        0,                              # SignatureHeaderSize
        signature_size                  # SignatureSize
    )
    
    # EFI_SIGNATURE_DATA
    sig_data = owner_guid.bytes_le + cert_data
    
    return sig_list + sig_data


def create_win_certificate(pkcs7_data):
    """
    Create WIN_CERTIFICATE_UEFI_GUID structure
    
    typedef struct {
      WIN_CERTIFICATE   Hdr;
      EFI_GUID          CertType;
      UINT8             CertData[...];
    } WIN_CERTIFICATE_UEFI_GUID;
    
    typedef struct {
      UINT32  dwLength;
      UINT16  wRevision;
      UINT16  wCertificateType;
    } WIN_CERTIFICATE;
    """
    cert_length = 8 + 16 + len(pkcs7_data)  # WIN_CERT + GUID + data
    
    win_cert = struct.pack(
        '<IHH16s',
        cert_length,                        # dwLength
        WIN_CERT_REVISION_2_0,              # wRevision
        WIN_CERT_TYPE_EFI_GUID,             # wCertificateType
        EFI_CERT_PKCS7_GUID.bytes_le        # CertType
    )
    
    return win_cert + pkcs7_data


def sign_with_signtool(data_to_sign, pfx_path, password):
    """
    Use Microsoft SignTool to generate PKCS#7 signature
    
    NOTE: SignTool.exe must be in PATH environment variable.
          Add Windows SDK bin directory to PATH before running this script.
    
    Returns: PKCS#7 DER-encoded signature data
    """
    print(f"[*] Using SignTool from PATH")
    
    with tempfile.TemporaryDirectory() as tmpdir:
        # Write data to temporary file
        data_file = os.path.join(tmpdir, "data.bin")
        with open(data_file, 'wb') as f:
            f.write(data_to_sign)
        
        # Prepare signtool command
        # /fd: File digest algorithm (SHA384)
        # /p7: Output directory for PKCS7 files
        # /p7co: PKCS7 content OID (1.2.840.113549.1.7.1 = data)
        # /p7ce: PKCS7 content encoding (DetachedSignedData)
        cmd = [
            'signtool',  # Directly use signtool from PATH
            'sign',
            '/fd', 'SHA384',
            '/p7', tmpdir,
            '/p7co', '1.2.840.113549.1.7.1',
            '/p7ce', 'DetachedSignedData',
            '/f', pfx_path,
        ]
        
        if password:
            cmd.extend(['/p', password])
        
        cmd.append(data_file)
        
        print(f"[*] Running SignTool...")
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )
            print(f"    {result.stdout.strip()}")
        except subprocess.CalledProcessError as e:
            print(f"[!] SignTool failed:")
            print(f"    stdout: {e.stdout}")
            print(f"    stderr: {e.stderr}")
            raise
        
        # Read generated PKCS#7 file
        p7_file = os.path.join(tmpdir, "data.bin.p7")
        if not os.path.exists(p7_file):
            raise FileNotFoundError(f"SignTool did not generate {p7_file}")
        
        with open(p7_file, 'rb') as f:
            pkcs7_data = f.read()
        
        print(f"[*] PKCS#7 signature generated: {len(pkcs7_data)} bytes")
        
        return pkcs7_data


def generate_auth_variable(cert_path, key_path, key_password, var_name, output_path):
    """
    Generate authenticated variable update file (.auth)
    """
    print(f"\n{'='*60}")
    print(f"  Generate EFI Authenticated Variable Update File")
    print(f"{'='*60}")
    print(f"[*] Variable: {var_name}")
    print(f"[*] Certificate: {cert_path}")
    print(f"[*] Signing Key: {key_path}")
    print(f"[*] Output: {output_path}")
    print(f"{'='*60}\n")
    
    # 1. Load certificate
    print(f"[*] Loading certificate from {cert_path}")
    with open(cert_path, 'rb') as f:
        cert_data = f.read()
    
    print(f"    Certificate size: {len(cert_data)} bytes")
    
    # 2. Get variable name and GUID
    if var_name.upper() in VAR_NAMES:
        var_name_str, var_guid = VAR_NAMES[var_name.upper()]
    else:
        print(f"[!] Unknown variable name: {var_name}")
        print(f"    Supported: {', '.join(VAR_NAMES.keys())}")
        return False
    
    print(f"[*] Variable details:")
    print(f"    Name: {var_name_str}")
    print(f"    GUID: {{{var_guid}}}")
    
    # 3. Create EFI_SIGNATURE_LIST
    print(f"\n[*] Creating EFI_SIGNATURE_LIST")
    sig_list = create_signature_list(cert_data)
    print(f"    Size: {len(sig_list)} bytes")
    
    # 4. Create EFI_TIME (timestamp)
    print(f"\n[*] Creating timestamp")
    timestamp = create_efi_time()
    current_time = datetime.now(timezone.utc)
    print(f"    Time: {current_time.isoformat()}")
    
    # 5. Prepare data to sign
    # According to UEFI Spec 2.10 Section 8.2.2:
    # SignedData = VariableName + VendorGuid + Attributes + TimeStamp + Data
    print(f"\n[*] Preparing data to sign")
    var_name_utf16 = (var_name_str + '\0').encode('utf-16le')
    attributes = (EFI_VARIABLE_NON_VOLATILE | 
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                  EFI_VARIABLE_RUNTIME_ACCESS |
                  EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)
    
    data_to_sign = (
        var_name_utf16 +
        var_guid.bytes_le +
        struct.pack('<I', attributes) +
        timestamp +
        sig_list
    )
    
    print(f"    Data to sign: {len(data_to_sign)} bytes")
    print(f"      - Variable name (UTF-16): {len(var_name_utf16)} bytes")
    print(f"      - GUID: 16 bytes")
    print(f"      - Attributes: 4 bytes")
    print(f"      - Timestamp: 16 bytes")
    print(f"      - Signature List: {len(sig_list)} bytes")
    
    data_hash = hashlib.sha384(data_to_sign).hexdigest()
    print(f"    SHA384: {data_hash}")
    
    # 6. Sign with SignTool
    print(f"\n[*] Signing with Microsoft SignTool")
    try:
        pkcs7_data = sign_with_signtool(data_to_sign, key_path, key_password)
    except Exception as e:
        print(f"[!] Failed to sign with SignTool: {e}")
        return False
    
    # 7. Create WIN_CERTIFICATE_UEFI_GUID
    print(f"\n[*] Creating WIN_CERTIFICATE_UEFI_GUID structure")
    win_cert = create_win_certificate(pkcs7_data)
    print(f"    WIN_CERTIFICATE size: {len(win_cert)} bytes")
    
    # 8. Assemble EFI_VARIABLE_AUTHENTICATION_2
    print(f"\n[*] Assembling EFI_VARIABLE_AUTHENTICATION_2")
    auth_var = timestamp + win_cert + sig_list
    
    print(f"    Total size: {len(auth_var)} bytes")
    print(f"      - Timestamp: 16 bytes")
    print(f"      - WIN_CERTIFICATE: {len(win_cert)} bytes")
    print(f"      - Signature List: {len(sig_list)} bytes")
    
    # 9. Write to output file
    print(f"\n[*] Writing to {output_path}")
    with open(output_path, 'wb') as f:
        f.write(auth_var)
    
    print(f"\n{'='*60}")
    print(f"[+] Success! Generated {len(auth_var)} bytes")
    print(f"    Output: {output_path}")
    print(f"{'='*60}\n")
    
    return True


def main():
    parser = argparse.ArgumentParser(
        description='Generate EFI Authenticated Variable Update File using Microsoft SignTool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
This tool uses Microsoft SignTool.exe to generate PKCS#7 signatures and
assembles them into EFI_VARIABLE_AUTHENTICATION_2 structures.

PREREQUISITE: SignTool.exe must be in PATH environment variable!

Examples:
  # Generate KEK.auth from KEK certificate, signed by PK
  python generate_auth_var.py --cert KEK.der --key PK.pfx --password mypass --var-name KEK --output KEK.auth
  
  # Generate db.auth, signed by KEK
  python generate_auth_var.py --cert db.der --key KEK.pfx --password mypass --var-name db --output db.auth

Requirements:
  - Python 3.7+
  - Microsoft SignTool.exe in PATH (from Windows SDK)
  - PFX file containing signing key and certificate

Workflow:
  1. Load certificate (KEK/DB)
  2. Create EFI_SIGNATURE_LIST
  3. Prepare data to sign (VariableName + GUID + Attributes + Timestamp + Data)
  4. Call SignTool to generate PKCS#7 signature
  5. Assemble EFI_VARIABLE_AUTHENTICATION_2 structure
  6. Write .auth file
        """
    )
    
    parser.add_argument('--cert', required=True, help='Certificate file (DER format, .cer/.der)')
    parser.add_argument('--key', required=True, help='Signing key file (PKCS#12 format, .pfx)')
    parser.add_argument('--password', help='Password for signing key')
    parser.add_argument('--var-name', required=True, choices=['PK', 'KEK', 'db', 'DB', 'dbx', 'DBX'],
                        help='Variable name to update')
    parser.add_argument('--output', required=True, help='Output .auth file')
    
    args = parser.parse_args()
    
    # Verify input files exist
    if not Path(args.cert).exists():
        print(f"[!] Certificate file not found: {args.cert}")
        return 1
    
    if not Path(args.key).exists():
        print(f"[!] Key file not found: {args.key}")
        return 1
    
    # Generate authenticated variable
    success = generate_auth_variable(
        args.cert,
        args.key,
        args.password,
        args.var_name,
        args.output
    )
    
    return 0 if success else 1


if __name__ == '__main__':
    exit(main())

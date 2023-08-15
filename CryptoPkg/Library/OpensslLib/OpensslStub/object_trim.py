#!/usr/bin/python3
# SPDX-License-Identifier: BSD-2-Clause-Patent
import os
import sys
import shutil
import subprocess

# generate obj files by trimed objects.txt.
# reference: ../openssl/Makefile.in L1171
def generated_openssl_obj_files():
    # prepare
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if os.path.exists('tempobj'):
        shutil.rmtree('tempobj')
    os.makedirs('tempobj', exist_ok = True)
    # empty obj_mac.num
    filetemp = open('tempobj/obj_mac.num', 'w')
    filetemp.write('undef\t\t0')
    filetemp.close()

    # generate trimed obj_mac.num and renamed to obj_mac.new
    cmdline = [ 'perl', '../openssl/crypto/objects/objects.pl', '-n', 'objects.txt', 'tempobj/obj_mac.num']
    filetemp = open('tempobj/obj_mac.new', 'w')
    rc = subprocess.run(cmdline, stdout = filetemp, stderr = subprocess.PIPE)
    filetemp.close()
    if rc.returncode:
        print(rc.stderr)
        sys.exit(rc.returncode)

    # The order of obj list should be the same as before
    objnumraw = []
    objnum = []
    objnumnew = []
    with open('../openssl/crypto/objects/obj_mac.num', 'r') as f1:
        for line in f1.readlines():
            objnumraw.append(line.split()[0])
    with open('tempobj/obj_mac.new', 'r') as f2:
        for line in f2.readlines():
            objnum.append(line.split()[0])
    numcount = 0
    for numitem in objnumraw:
        if numitem in objnum:
            objnumnew.append(str(numitem) + '\t\t' + str(numcount) + '\n')
            numcount += 1
    with open('tempobj/obj_mac.new', 'w') as f3:
        for numitem in objnumnew:
            f3.write(numitem)

    # generate trimed obj_mac.h
    cmdline = [ 'perl', '../openssl/crypto/objects/objects.pl', 'objects.txt', 'tempobj/obj_mac.new']
    filetemp = open('tempobj/obj_mac.h.new', 'w')
    rc = subprocess.run(cmdline, stdout = filetemp, stderr = subprocess.PIPE)
    filetemp.close()
    if rc.returncode:
        print(rc.stderr)
        sys.exit(rc.returncode)

    # generate trimed obj_dat.h
    cmdline = [ 'perl', '../openssl/crypto/objects/obj_dat.pl', 'tempobj/obj_mac.h.new']
    filetemp = open('tempobj/obj_dat.h', 'w')
    rc = subprocess.run(cmdline, stdout = filetemp, stderr = subprocess.PIPE)
    filetemp.close()
    if rc.returncode:
        print(rc.stderr)
        sys.exit(rc.returncode)

    # copy content of obj_compat.h
    filetemp = open('tempobj/obj_mac.h.new', 'a')
    with open('../openssl/crypto/objects/obj_compat.h', 'r') as f:
        for line in f.readlines():
            if not line.startswith('/*') and not line.startswith(' *'):
                filetemp.write(line)
    filetemp.close()

def add_removed_nid_as_null():
    objlistraw = []
    objlist = []
    objlistmissed = ''
    
    f1 = open('../openssl/include/openssl/obj_mac.h', 'r')
    for line in f1.readlines():
        if 'NID_' in line:
            lineList = line.split()
            objlistraw.append(lineList[1])

    f2 = open('tempobj/obj_mac.h.new', 'r')
    rawfile = f2.readlines()
    for line in rawfile:
        if 'NID_' in line:
            lineList = line.split()
            objlist.append(lineList[1])
    for nidItem in objlistraw:
        if nidItem not in objlist:
            objlistmissed = objlistmissed + '#define ' + nidItem + '                NID_undef\r\n'
            objlistmissed = objlistmissed + '#define ' + nidItem.replace('NID_', 'SN_') + '                SN_undef\r\n'

    f3 = open('tempobj/obj_mac.h', 'w')
    for line in rawfile:
        if '#endif /* OPENSSL_OBJ_MAC_H */' in line:
            f3.write(objlistmissed)
        f3.write(line)

def copy_generated_file(src, dst):
    src_file = []
    with open(src, 'r') as fsrc:
        src_file = fsrc.readlines()
    with open(dst, 'w') as fdst:
        for lines in range(len(src_file)):
            s = src_file[lines]
            s = s.rstrip() + "\r\n"
            fdst.write(s.expandtabs())

def main():
    generated_openssl_obj_files()
    add_removed_nid_as_null()
    # move generated files to correct location
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if not os.path.exists('openssl'):
        os.mkdir('openssl')
    if not os.path.exists('crypto'):
        os.mkdir('crypto')
        os.mkdir('crypto/object')
    copy_generated_file('tempobj/obj_dat.h', 'crypto/objects/obj_dat.h')
    copy_generated_file('tempobj/obj_mac.h', 'openssl/obj_mac.h')
    if os.path.exists('tempobj'):
        shutil.rmtree('tempobj')
    shutil.copyfile('../openssl/crypto/objects/obj_dat.c', 'crypto/objects/obj_dat.c')

if __name__ == '__main__':
    sys.exit(main())

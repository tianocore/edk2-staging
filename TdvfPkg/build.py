#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# @file
#
# @copyright
# INTEL CONFIDENTIAL
# Copyright 2020 Intel Corporation. <BR>
#
# The source code contained or described herein and all documents related to the
# source code ("Material") are owned by Intel Corporation or its suppliers or
# licensors. Title to the Material remains with Intel Corporation or its suppliers
# and licensors. The Material may contain trade secrets and proprietary    and
# confidential information of Intel Corporation and its suppliers and licensors,
# and is protected by worldwide copyright and trade secret laws and treaty
# provisions. No part of the Material may be used, copied, reproduced, modified,
# published, uploaded, posted, transmitted, distributed, or disclosed in any way
# without Intel's prior express written permission.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery
# of the Materials, either expressly, by implication, inducement, estoppel or
# otherwise. Any license under such intellectual property rights must be
# express and approved by Intel in writing.
#
# Unless otherwise agreed by Intel in writing, you may not remove or alter
# this notice or any other notice embedded in Materials by Intel or
# Intel's suppliers or licensors in any way.
##

import sys
import argparse
import os
import pathlib
import shutil
import stat
import platform
import time
import subprocess
import logging
import struct
from scripts.tdvf_metadata import inject_metadata
from scripts.VarEnroll import var_enroll, VarEnrollOps, is_guid, str2guid

#
# Secure boot related pk/kek/db/dbx files
# and SecureBootEnable variable
#
DefaultSecureBootConfig = {
    'PK': ['77fa9abd-0359-4d32-bd60-28f4e78f784b', os.path.join('MsCert', 'Test-pk.cer')],
    'KEK': ['77fa9abd-0359-4d32-bd60-28f4e78f784b', os.path.join('MsCert', 'MicCorKEKCA2011_2011-06-24.cer')],
    'db': ['77fa9abd-0359-4d32-bd60-28f4e78f784b', os.path.join('MsCert', 'MicCorUEFCA2011_2011-06-27.cer')],
    'dbx': ['d719b2cb-3d3a-4596-a3bc-dad00e67656f', os.path.join('MsCert', 'dbxupdate_x64.bin')],
    'SecureBootEnable': ['f0a30bc7-af08-4556-99c4-001009c93a44', os.path.join('MsCert', 'SecureBootEnable.bin')]
}

def IsSecureBootConfigValid(SecureBootConfig, build_log):
    '''
    If User provide the PK/KEK/db/dbx params via command line,
    We need check whether whether the params are valid.
    PK/KEK/db/SecureBootEnable is mandatory,
    dbx is optional
    :param SecureBootConfig:
    :param build_log:
    :return:
    '''
    vars = [v for v in SecureBootConfig.keys()]
    mandatory_vars = ['PK', 'KEK', 'db', 'SecureBootEnable']
    valid = True
    for v in mandatory_vars:
        if v not in vars:
            build_log.log(LOG_ERR, "SecureBoot variable [%s] is missing" % v)
            valid = False

    return valid

def SetSecureBootConfig(SecureBootConfig, arg, guid, cert_bin_file, pkg_path, build_log):
    '''
    Set the SecureBootConfig.
    If cert_bin_file is a relative file path, then it should be relative to @pkg_path
    :param SecureBootConfig:
    :param arg:
    :param guid:
    :param cert_bin_file:
    :param pkg_path:
    :return:
    '''
    sb_var_names = {
        '-pk': 'PK',
        '-kek': 'KEK',
        '-db': 'db',
        '-dbx': 'dbx',
        '-secure_boot': 'SecureBootEnable'
    }
    if arg not in sb_var_names.keys():
        build_log.log(LOG_ERR, "Invalid SecureBoot variables[%s]" % arg)
        return False, SecureBootConfig

    if not is_guid(guid):
        build_log.log(LOG_ERR, "Invalid SecureBoot Guid[%s]" % guid)
        return False, SecureBootConfig

    if os.path.isabs(cert_bin_file):
        abs_cert_bin_file = cert_bin_file
    else:
        abs_cert_bin_file = os.path.join(pkg_path, cert_bin_file)

    if not os.path.isfile(abs_cert_bin_file):
        build_log.log(LOG_ERR, "File not exist [%s]. relative path?" % cert_bin_file)
        return False, SecureBootConfig

    var_name = sb_var_names[arg]
    SecureBootConfig[var_name] = [guid, abs_cert_bin_file]
    return True, SecureBootConfig

#
# logging level
#
LOG_DBG  = 0x1
LOG_INFO = 0x2
LOG_WARN = 0x4
LOG_ERR  = 0x8

class BuildLog(object):
    '''
    build log
    '''
    def __init__(self,log_file):
        # Define a Handler which writes INFO messages or higher to the sys.stderr
        console = logging.StreamHandler()
        console.setLevel(logging.DEBUG)

        # Set a format which is simpler for console use
        console_formatter = logging.Formatter('  %(message)s')
        console.setFormatter(console_formatter) # Tell the handler to use this format

        # Define a Handler which writes INFO messages or higher to the log_file
        file_handler = logging.FileHandler(log_file,'w')
        file_format_str = logging.Formatter('%(asctime)s %(levelname)-8s %(message)s',datefmt='%m-%d %H:%M:%S')
        file_handler.setFormatter(file_format_str)
        file_handler.setLevel(logging.DEBUG)

        self.logger = logging.getLogger(log_file)
        self.logger.addHandler(console)
        self.logger.addHandler(file_handler)
        self.logger.setLevel(logging.DEBUG)

    def close_handlers(self):
        for handler in self.logger.handlers:
            handler.close()

    def log(self, level, log):
        if level == LOG_DBG:
            self.logger.debug(log)
        elif level == LOG_INFO:
            self.logger.info(log)
        elif level == LOG_WARN:
            self.logger.warning(log)
        elif level == LOG_ERR:
            self.logger.error(log)

class VarEnrollParams:
    '''
    VarEnroll related params
    '''
    def __init__(self):
        self.info = None
        self.fd = None
        self.operation = None
        self.name = None
        self.guid = None
        self.attributes = None
        self.data_file = None
        self.output = None

def do_var_enroll(input_fd, output_fd, pkg_path, SecureBootConfig, build_log):
    '''
    enroll Secure Boot related variables
    :param input_fd:
    :param output_fd:
    :param pkg_path:
    :return:
    '''

    FILE_PATH = lambda file, base_dir: file if os.path.isabs(file) else os.path.join(base_dir, file)
    pk_file = FILE_PATH(SecureBootConfig['PK'][1], pkg_path)
    kek_file = FILE_PATH(SecureBootConfig['KEK'][1], pkg_path)
    db_file = FILE_PATH(SecureBootConfig['db'][1], pkg_path)
    dbx_file = FILE_PATH(SecureBootConfig['dbx'][1], pkg_path) if 'dbx' in SecureBootConfig.keys() else None
    enable_bin_file = FILE_PATH(SecureBootConfig['SecureBootEnable'][1], pkg_path)

    out_pk = input_fd + '.pk'
    out_kek = out_pk + '.kek'
    out_db = out_kek + '.db'
    out_dbx = out_db + '.dbx'
    out_sb_enable = out_dbx + '.sb'
    result = False

    while True:
        # enroll pk
        if not os.path.isfile(pk_file):
            break
        args = VarEnrollParams()
        args.__dict__.update(fd=input_fd, output=out_pk, data_file=pk_file,
                             guid=SecureBootConfig['PK'][0], name='PK', operation=VarEnrollOps.add)
        ret = var_enroll(args)
        build_log.log(LOG_DBG, "\nEnroll PK variable -- %s\n"%('Success' if ret else 'Failed'))
        if not ret:
            break

        # enroll kek
        if not os.path.isfile(kek_file):
            break
        args.__dict__.update(fd=out_pk, output=out_kek, data_file=kek_file,
                             guid=SecureBootConfig['KEK'][0], name='KEK', operation=VarEnrollOps.add)
        ret = var_enroll(args)
        build_log.log(LOG_DBG, "\nEnroll KEK variable -- %s\n" % ('Success' if ret else 'Failed'))
        if not ret:
            break

        # enroll db
        if not os.path.isfile(db_file):
            break
        args.__dict__.update(fd=out_kek, output=out_db, data_file=db_file,
                             guid=SecureBootConfig['db'][0], name='db', operation=VarEnrollOps.add)
        ret = var_enroll(args)
        build_log.log(LOG_DBG, "\nEnroll db variable -- %s\n" % ('Success' if ret else 'Failed'))
        if not ret:
            break

        # enroll dbx
        # dbx may not be enrolled
        if dbx_file:
            if not os.path.isfile(dbx_file):
                break

            args.__dict__.update(fd=out_db, output=out_dbx, data_file=dbx_file,
                                 guid=SecureBootConfig['dbx'][0], name='dbx', operation=VarEnrollOps.add)
            ret = var_enroll(args)
            build_log.log(LOG_DBG, "\nEnroll dbx variable -- %s\n" % ('Success' if ret else 'Failed'))
            if not ret:
                break
        else:
            shutil.copyfile(out_db, out_dbx)

        # enable SecureBoot
        if not os.path.isfile(enable_bin_file):
            break
        args.__dict__.update(fd=out_dbx, output=out_sb_enable, data_file=enable_bin_file,
                             name='SecureBootEnable', guid=SecureBootConfig['SecureBootEnable'][0],
                             attributes="0x3", operation=VarEnrollOps.add)
        ret = var_enroll(args)
        build_log.log(LOG_DBG, "\nEnroll SecureBootEnable variable -- %s\n" % ('Success' if ret else 'Failed'))
        if not ret:
            break

        shutil.copyfile(out_sb_enable, output_fd)
        result = True
        break

    ## then clean the tmp files
    if os.path.isfile(out_pk):
        os.remove(out_pk)
    if os.path.isfile(out_kek):
        os.remove(out_kek)
    if os.path.isfile(out_db):
        os.remove(out_db)
    if os.path.isfile(out_dbx):
        os.remove(out_dbx)
    if os.path.isfile(out_sb_enable):
        os.remove(out_sb_enable)

    build_log.log(LOG_DBG, "\n[%s]Enroll Secure Boot Variables\n" % ('Success' if result else 'Failed'))

    return result


def execute_cmd(cmd, is_linux, build_log, work_dir=None):
    '''
    execute the command
    :param cmd:
    :param is_linux:
    :param build_log:
    :param work_dir:
    :return:
    '''
    build_log.log(LOG_DBG, "%s" % cmd)
    failed = False
    if cmd.strip().startswith('None '):
        failed = True
    try:
        start = int(round(time.time() * 1000))
        try:
            if work_dir:
                if is_linux:
                    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, executable='/bin/bash',\
                                     stderr=subprocess.STDOUT,cwd=work_dir)
                else:
                    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,\
                                     stderr=subprocess.STDOUT,cwd=work_dir)
            else:
                if is_linux:
                    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, executable='/bin/bash',\
                                     stderr=subprocess.STDOUT)
                else:
                    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,\
                                     stderr=subprocess.STDOUT)

            while True:
                line = p.stdout.readline()
                if not line:
                    break
                build_log.log(LOG_DBG, line.decode('utf-8'))
                tmp_str = line.split()
                if line.strip().startswith('- Failed -'.encode(encoding='ascii')) or \
                        (len(tmp_str) >= 2 and tmp_str[0].endswith(':'.encode(encoding='ascii')) \
                        and tmp_str[1] == 'ERROR') or (len(tmp_str) >= 3 \
                        and tmp_str[0] == 'Error' and tmp_str[2] == '-'):
                    failed = True

            p.communicate()

        except subprocess.CalledProcessError as e:
            ret = e.returncode
        else:
            ret = 0
        finally:
            build_log.log(LOG_DBG, '[cmd=%s]' % cmd)
            pass
        end = int(round(time.time() * 1000))

    except Exception as e:
        build_log.log(LOG_ERR, e)
        raise RuntimeError
    else:
        if failed:
            ret = -1
        if ret == 0:
            ret_str = 'SUCCESS'
        else:
            ret_str = 'FAIL'
        build_log.log(LOG_DBG, "\t - %s : %d ms\n" % (ret_str, end - start))
        if ret:
            os._exit(ret)
    return ret

def check_gcc():
    '''
    check the gcc version
    :return:
    '''
    ss = sys.version
    pos = ss.find('[GCC')
    gcc_major = 0
    if pos > 0:
        gcc_str = ss[pos:]
        gcc_ver_str = gcc_str.split(' ')[1]
        gcc_major = int(gcc_ver_str.split('.')[0])
    return gcc_major

def check_os():
    '''
    check the working os and return 'Windows', 'Linux', ...
    '''
    sys_str = platform.system()
    if 'Windows' in sys_str:
        return 'Windows'
    elif 'Linux' in sys_str:
        return 'Linux'
    else:
        return sys_str

def check_workspace(tdvf_pkg_path, is_linux, build_log):
    '''
    Check the workspace and set the env of WORKSPACE and PACKAGES_PATH
    :param tdvf_pkg_path:
    :param is_linux:
    :param build_log:
    :return:
    '''

    tdvfpkg_path = str(tdvf_pkg_path)
    tdvfpkg_parent_path = str(tdvf_pkg_path.parent)
    if not os.environ.get('WORKSPACE'):
        # workspace is not set
        build_log.log(LOG_ERR, "Set the env of WORKSPACE to edk2 before building")
        return None

    work_space = os.environ.get('WORKSPACE')
    build_log.log(LOG_DBG, "Building from: %s" % work_space)
    build_log.log(LOG_DBG, "WORKSPACE     = %s" % work_space)

    # check the TdvfPkg
    if not os.path.isdir(tdvfpkg_path) or not tdvfpkg_path.lower().endswith('tdvfpkg'):
        build_log.log(LOG_ERR, "Invalid TdvfPkg path - %s" % tdvfpkg_path)
        return None

    # set PACKAGES_PATH
    if is_linux:
        os.environ['PACKAGES_PATH'] = '%s:%s' % (tdvfpkg_parent_path, work_space)
    else:
        os.environ['PACKAGES_PATH'] = '%s;%s' % (tdvfpkg_parent_path, work_space)

    build_log.log(LOG_DBG, "PACKAGES_PATH = %s" % os.environ['PACKAGES_PATH'])
    return work_space

def check_basetools(edk_tools_bin, is_linux):
    '''
    Check whether the edk2 BaseTools are ready
    '''
    if is_linux:
        tools = ['GenFv', 'GenFfs', 'VfrCompile']
    else:
        tools = ['GenFv.exe', 'GenFfs.exe', 'VfrCompile.exe']
    for tool in tools:
        if not os.path.isfile(os.path.join(edk_tools_bin, tool)):
            return False
    return True

def main():
    '''
    the main function
    :return:
    '''

    # the default build params
    ARCH = 'X64'
    BUILDTARGET = "DEBUG"
    KEY_ENROLL = False
    TDVF_NAME = 'TDVF'
    TDVF_SB_ENABLED = 'TDVF.sb'
    BUILD_OPTIONS = ''
    SHOW_HELP = False

    SecureBootConfig = {}

    parser = argparse.ArgumentParser(add_help=False)
    curr_path = pathlib.Path(__file__).parent.absolute()
    pkg_path = str(curr_path)
    build_log = BuildLog(os.path.join(pkg_path, "Build.log"))

    os_str = check_os()
    is_linux = True if os_str == 'Linux' else False


    #
    # Pick a default tool type for a given OS
    #
    quit = False
    if 'Linux' in os_str:
        SUPPORTED_TARGET_TOOLS = ['GCC5']
        OS = 'Linux'
        # check gcc version
        gcc_major = check_gcc()
        if gcc_major < 5:
            build_log.log(LOG_ERR, 'Tdvf requires GCC5 or later')
            quit = True
        else:
            TARGET_TOOLS = 'GCC5'

    elif 'Windows' in os_str:
        SUPPORTED_TARGET_TOOLS = ['VS2015', 'VS2017', 'VS2019']
        TARGET_TOOLS = 'VS2015'
        OS = 'Windows'

    else:
        build_log.log(LOG_DBG, 'Only support Linux and Windows.')
        quit = True

    if quit:
        return False

    #
    # scan command line to override defaults
    #
    nm, args = parser.parse_known_args()
    argn = len(args)
    i = 0

    while i < argn:
        arg = args[i]
        i += 1

        if arg in ['-a', '-b', '-t', '-p', '-d', '-D']:
            # there should be one more params after above args
            if i >= argn:
                build_log.log(LOG_ERR, 'Missing val of command args [%s] ?' % arg)
                quit = True
                break

            val = args[i]
            if val.startswith('-'):
                build_log.log(LOG_ERR, 'Invalid command args [%s %s] ' %(arg, val))
                quit = True
                break

            if arg == '-a':
                if val != 'X64':
                    build_log.log(LOG_ERR, 'TDVF only supports X64')
                    quit = True
                    break

            elif arg == '-b':
                if val not in ['DEBUG', 'RELEASE']:
                    build_log.log(LOG_ERR, 'TDVF only support DEBUG/RELEASE')
                    quit = True
                    break
                else:
                    BUILDTARGET = val

            elif arg == '-t':
                if val not in SUPPORTED_TARGET_TOOLS:
                    build_log.log(LOG_ERR, 'In %s TDVF only support tool_chains of %s' % (OS, ','.join(SUPPORTED_TARGET_TOOLS)))
                    quit = True
                    break
                else:
                    TARGET_TOOLS = val

            elif arg == '-p':
                PLATFORMFILE = val

            elif arg in ['-d', '-D']:
                BUILD_OPTIONS += (" -D %s "%val)

            else:
                build_log.log(LOG_ERR, 'Unsupported command args [%s] ' % arg)
                quit = True
                break

            i += 1

        elif arg == '-workspace':
            val = args[i]
            os.environ['WORKSPACE'] = val
            i += 1
        elif arg == '-nasm_prefix':
            val = args[i]
            os.environ['NASM_PREFIX'] = val
            i += 1

        elif arg == '-key_enroll':
            # if -key_enroll is present, it means the default SecureBootConfig will be used.
            # the -pk/-kek/-db/-dbx params will be ignored (if exists)
            KEY_ENROLL = True

        elif arg in ['-pk', '-kek', '-db', '-dbx', '-secure_boot']:
            # parse SecureBoot related params
            # SecureBoot related params has its dedicated format
            # for exampl: -pk <guid> <cert_file|bin_file>
            if i + 1 >= argn:
                build_log.log(LOG_ERR, "Invalid SecureBoot related params - %s" % arg)
                quit = True
                break

            guid = args[i]
            cert_bin_file = args[i+1]
            if guid.startswith('-') or cert_bin_file.startswith('-'):
                build_log.log(LOG_ERR, "Invalid SecureBoot related params - %s %s" % (guid, cert_bin_file))
                quit = True
                break
            # set SecureBootConfig
            valid, SecureBootConfig = SetSecureBootConfig(SecureBootConfig, arg, guid, cert_bin_file, pkg_path, build_log)
            if not valid:
                build_log.log(LOG_ERR, "Failed to set SecureBoot related params.[%s, %s, %s]" % (arg, guid, cert_bin_file))
                quit = True
                break

            i += 2

        elif arg == '-h':
            SHOW_HELP = True

        else:
            BUILD_OPTIONS += (" %s" % arg)

    if quit:
        return False

    ## -D TDX_EMULATION_ENABLE is mandatory
    if '-D TDX_EMULATION_ENABLE=' not in BUILD_OPTIONS:
        build_log.log(LOG_ERR, "-D TDX_EMULATION_ENABLE=TRUE/FALSE is a mandatory build flag. TRUE for KVM Software SDV, FALSE for the real TDX platform.")
        return False

    ## check workspace first
    work_space = check_workspace(curr_path, is_linux, build_log)
    if not work_space:
        return False

    edk_tools_path = os.path.join(work_space, 'BaseTools')
    PLATFORMFILE = os.path.join(pkg_path, 'TdvfPkg.dsc')

    # if -key_enroll is present, use the DefaultSecureBootConfig
    if KEY_ENROLL:
        SecureBootConfig = DefaultSecureBootConfig

    #
    # check SecureBoot config valid
    #
    if len(SecureBootConfig.keys()) > 0:
        valid = IsSecureBootConfigValid(SecureBootConfig, build_log)
        if not valid:
            return False
        else:
            KEY_ENROLL = True

    #
    # Check BaseTools
    #
    if is_linux:
        edk_tools_bin = os.path.join(edk_tools_path, 'Source', 'C', 'bin')
    else:
        edk_tools_bin = os.path.join(edk_tools_path, 'Bin', 'Win32')
    os.environ['EDK_TOOLS_BIN'] = edk_tools_bin

    if check_basetools(edk_tools_bin, is_linux):
        build_log.log(LOG_DBG, "Using prebuilt tools")
    else:
        # build base tools
        build_log.log(LOG_DBG, "Building tools in %s" % edk_tools_path)
        if is_linux:
            cmd = 'make -C %s' % edk_tools_path
        else:
            cmd = 'edksetup.bat Rebuild %s' % TARGET_TOOLS
        result = execute_cmd(cmd, is_linux, build_log, work_space)
        if result != 0 or not check_basetools(edk_tools_bin, is_linux):
            build_log.log(LOG_ERR, "Failed to make BaseTools")
            return False

    # there are some tricks that when build in windows with VS2015
    # the tool chain should be VS2015x86
    if TARGET_TOOLS == 'VS2015':
        TARGET_TOOLS = 'VS2015x86'
    BUILD_ROOT = os.path.join(work_space, 'Build', 'Tdvf', BUILDTARGET + '_' + TARGET_TOOLS)
    FV_DIR = os.path.join(BUILD_ROOT, 'FV')
    TDVF_FD = os.path.join(FV_DIR, TDVF_NAME + '.fd')

    #
    # now let's build
    #
    build_log.log(LOG_DBG, 'Running edk2 build for TdvfPkg on %s' % OS)
    os.chdir(work_space)
    build_cmd = "build -p %s %s -b %s -t %s -a %s" % (PLATFORMFILE, BUILD_OPTIONS, BUILDTARGET, TARGET_TOOLS, ARCH)
    help_cmd = "build -h"
    if OS == 'Linux':
        cmd = ". edksetup.sh; %s" % (help_cmd if SHOW_HELP else build_cmd)
    elif OS == "Windows":
        if TARGET_TOOLS == 'VS2015x86':
            TARGET_TOOLS = 'VS2015'
        cmd = "edksetup.bat %s && %s"%(TARGET_TOOLS, help_cmd if SHOW_HELP else build_cmd)
    else:
        build_log.log(LOG_ERR, "OS[%s] is not supported. " % OS)
        return False

    result = execute_cmd(cmd, is_linux, build_log)
    if SHOW_HELP:
        return True

    ## post build
    if not os.path.isfile(TDVF_FD):
        build_log.log(LOG_ERR, "TDVF.fd is not generated!")
        return False

    ## metadata
    result = inject_metadata(TDVF_FD, None, build_log)
    if not result:
        return False

    if not KEY_ENROLL:
        build_log.log(LOG_DBG, "\n--Final Output-- ")
        build_log.log(LOG_DBG, "    Fd file: %s" % TDVF_FD)
        return True

    ## enroll Secure Boot related variables
    output_fd = os.path.join(FV_DIR, TDVF_SB_ENABLED + '.fd')
    result = do_var_enroll(TDVF_FD, output_fd, pkg_path, SecureBootConfig, build_log)
    if not result:
        return False

    build_log.log(LOG_DBG, "\n--Final Output-- ")
    build_log.log(LOG_DBG, "  Fd file: %s" % TDVF_FD)
    build_log.log(LOG_DBG, "  Fd file(Secure Boot Enabled): %s" % output_fd)

    return True

if __name__ == '__main__':
    exit(0) if main() else exit(1)

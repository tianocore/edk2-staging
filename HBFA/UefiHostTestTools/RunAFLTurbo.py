## @file
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import os
import sys
import shutil
import platform
import subprocess
from optparse import OptionParser

__prog__ = 'RunAFLTurbo.py'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__version__ = '{} Version {}'.format(__prog__, '0.1 ')

## Get System type info
SysType = platform.system()

## Get System arch info
Is32bit = bool("64" not in platform.machine())

## build tool chain
if SysType == "Windows":
    ToolChain = "VS2015x86"
elif SysType == "Linux":
    ToolChain = "AFL"

## WORKSPACE
workspace = ''

## HBFA package path
HBFA_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))

## Conf directory
Conf_Path = os.path.join(HBFA_PATH, 'UefiHostFuzzTestPkg', 'Conf')

## Get PYTHON version
PyVersion = sys.version_info[0]

def CheckTestEnv():
    # Check EDKII BUILD WORKSPACE whether be set in system environment variable
    if 'WORKSPACE' not in os.environ:
        print("Please set system environment variable 'WORKSPACE' before run this script.")
        os._exit(0)
    global workspace
    workspace = os.environ['WORKSPACE']

    # Check whether HBFA environment is set up
    if 'build_rule.txt' not in os.listdir(Conf_Path) or 'tools_def.txt' not in os.listdir(Conf_Path) or 'target.txt'\
            not in os.listdir(Conf_Path):
        print("Please run HBFAEnvSetup.py before run this script.")
        os._exit(0)

    if "AFL_PATH" not in os.environ:
        print("Please set AFL_PATH in system environment variables.")
        os._exit(0)

    if SysType == "Windows":
        print("AFLTurbo doesnt support running in Windows. Please try with linux.")
        os._exit(0)

def GetPkgName(path):
    return path.split(os.path.sep)[0]

def GetModuleBinName(ModuleInfPath):
    with open(ModuleInfPath, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if 'BASE_NAME' in line:
                return line.split('=')[1].strip()

def CheckBuildResult(ModuleBinPath):
    if os.path.exists(ModuleBinPath):
        print("Build Successfully !!!\n")
    else:
        print("Build failure, can not find Module Binary file: {}".format(ModuleBinPath))
        os._exit(0)

def CopyFile(src, dst):
    try:
        shutil.copyfile(src, dst)
    except Exception as err:
        print(err)
    

def Build(Arch, Target, ModuleFilePath):
    PkgName = GetPkgName(ModuleFilePath)  
    ModuleFileAbsPath = os.path.join(HBFA_PATH, ModuleFilePath)
    ModuleBinName = GetModuleBinName(ModuleFileAbsPath)
    ModuleBinAbsPath = os.path.join(workspace, 'Build', PkgName, Target + '_' + ToolChain, Arch, ModuleBinName)
    PlatformDsc = os.path.join(PkgName, PkgName+'.dsc')

    BuildCmdList = []
    BuildCmdList.append('-p {}'.format(PlatformDsc))
    BuildCmdList.append('-m {}'.format(ModuleFilePath))
    BuildCmdList.append('-a {}'.format(Arch))
    BuildCmdList.append('-b {}'.format(Target))
    BuildCmdList.append('-t {}'.format(ToolChain))
    BuildCmdList.append('--conf {}'.format(Conf_Path))
    if SysType == 'Linux':
        BuildCmdList.append('-t GCC5')

    BuildCmd = 'build ' + ' '.join(BuildCmdList)
    SetEnvCmd = "export PATH=$PATH:$AFL_PATH\n"
    print("Start build Test Module:")
    print(BuildCmd)

    ExecCmd = SetEnvCmd + BuildCmd

    proccess = subprocess.Popen(ExecCmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT,
                                shell=True)
    msg = list(proccess.communicate())
    if PyVersion == 3:
        for num, submsg in enumerate(msg):
            if submsg is not None:
                msg[num] = submsg.decode()

    if msg[1]:
        print(msg[0] + msg[1])
        os._exit(0)
    elif "- Done -" not in msg[0]:
        print(msg[0])
        os._exit(0)
    else:
        pass
    CheckBuildResult(ModuleBinAbsPath)
    return ModuleBinAbsPath

def RunAFLTurbo(TestModuleBinPath, InputPath, OutputPath):
    SetEnvCmd = "export PATH=$PATH:$AFL_PATH\n"
    AFL_CMD = "afl-turbo-fuzz -i {} -o {} {} @@".format(InputPath, OutputPath, TestModuleBinPath)
    print("Start run AFLTurbo test:")
    print(AFL_CMD)
    ExecCmd = "gnome-terminal --geometry=90x30 -e 'bash -c \"{};exec bash\" ' ".format(SetEnvCmd + AFL_CMD)
    subprocess.Popen(ExecCmd,
                     stdout=subprocess.PIPE,
                     stderr=subprocess.STDOUT,
                     shell=True)

gParamCheck = []
def SingleCheckCallback(option, opt_str, value, parser):
    if option not in gParamCheck:
        setattr(parser.values, option.dest, value)
        gParamCheck.append(option)
    else:
        parser.error("Option %s only allows one instance in command line!" % option)

# Parse command line options
def MyOptionParser():
    Parser = OptionParser(description=__copyright__, version=__version__, prog=__prog__, usage="python %prog [options][argument]")
    Parser.add_option("-a", "--arch", action="callback", type="choice", choices=['IA32', 'X64', 'ARM', 'AARCH64'], dest="TargetArch", default="IA32", callback=SingleCheckCallback,
        help="ARCHS is one of list: IA32, X64, ARM or AARCH64, which overrides target.txt's TARGET_ARCH definition.")
    Parser.add_option("-b", "--buildtarget", action="callback", type="string", dest="BuildTarget", default="DEBUG", callback=SingleCheckCallback,
        help="Using the TARGET to build the platform, overriding target.txt's TARGET definition.")
    Parser.add_option("-m", "--module", action="callback", type="string", dest="ModuleFile", callback=SingleCheckCallback,
        help="Build the module specified by the INF file name argument.")
    Parser.add_option("-i", "--input", action="callback", type="string", dest="InputSeed", callback=SingleCheckCallback,
        help="Test input seed path.")
    Parser.add_option("-o", "--output", action="callback", type="string", dest="Output", callback=SingleCheckCallback,
        help="Test output path for AFL.")
    (Opt, Args) = Parser.parse_args()
    return (Opt, Args)

def main():
    (Option, Target) = MyOptionParser()
    TargetArch = Option.TargetArch
    BuildTarget = Option.BuildTarget

    CheckTestEnv()

    if not Option.ModuleFile:
        print("ModuleFile should be set once by command -m MODULEFILE, --module=MODULEFILE.")
        os._exit(0)
    elif os.path.isabs(Option.ModuleFile):
        if Option.ModuleFile.startswith(HBFA_PATH):
            ModuleFilePath = os.path.relpath(Option.ModuleFile, HBFA_PATH)
        else:
            print("ModuleFile path: {} should be start with {}.".format(Option.ModuleFile, HBFA_PATH))
            os._exit(0)
    elif not os.path.exists(os.path.join(HBFA_PATH, Option.ModuleFile)):
        print("ModuleFile path: {} is no exits or not the relative path for HBFA".format(Option.ModuleFile))
        os._exit(0)
    else:
        ModuleFilePath = Option.ModuleFile

    if not Option.InputSeed:
        print("InputSeed path should be set once by command -i INPUTSEED, --input=INPUTSEED.")
        os._exit(0)
    elif not os.path.exists(Option.InputSeed):
        print("InputSeed path: {} is no exists".format(os.path.abspath(Option.InputSeed)))
        os._exit(0)
    else:
        InputSeedPath = Option.InputSeed

    if not Option.Output:
        print("OutputSeed path should be set once by command -o OUTPUT, --output=OUTPUT.")
        os._exit(0)
    else:
        OutputSeedPath = Option.Output
        if not os.path.isabs(OutputSeedPath):
            OutputSeedPath = os.path.join(os.getcwd(), OutputSeedPath)
        if not os.path.exists(OutputSeedPath):
            try:
                os.makedirs(OutputSeedPath)
            except Exception as err:
                print(err)
        elif os.path.exists(os.path.join(OutputSeedPath, 'fuzzer_stats')):
            print("OutputSeedPath:{} is already exists, please change another directory.".format(OutputSeedPath))
            os._exit(0)

    TestModuleBinPath = Build(TargetArch, BuildTarget, ModuleFilePath)
    RunAFLTurbo(TestModuleBinPath, InputSeedPath, OutputSeedPath)

if __name__ == "__main__":
    main()



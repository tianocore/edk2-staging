How to run AFLTurbo with UefiHostTestPkg in OS?
==============
AFLTurbo:
Prepare AFLTurbo in Linux
1) Install crypto dependency
    sudo apt-get install libssl-dev
2) Goto https://github.com/sleicasper/aflturbo
3) Clone the repository and build the aflturbo code
    git clone https://github.com/sleicasper/aflturbo.git
    cd aflturbo/
    make
    cp afl-fuzz afl-turbo-fuzz

> Build AFLTurbo with `make` command & ensure AFLTurbo binary is in PATH environment variable.

4) Add below content at the end of ~/.bashrc:
	  export AFL_PATH=<AFL_TURBO_PATH>
	  export PATH=$PATH:$AFL_PATH
	 For example:
	  export AFL_PATH=/home/<user name>/Env/aflturbo
	  export PATH=$PATH:$AFL_PATH

5) Run these commands as root (these command need to be ran every time you reboot your OS):
	  sudo bash -c 'echo core >/proc/sys/kernel/core_pattern'
		cd /sys/devices/system/cpu/
		sudo bash -c 'echo performance | tee cpu*/cpufreq/scaling_governor'

Build EDKII test case in Linux
1) python edk2-staging/HBFA/UefiHostTestTools/HBFAEnvSetup.py
2) cp edk2-staging/HBFA/UefiHostFuzzTestPkg/Conf/build_rule.txt edk2/Conf/build_rule.txt
3) cp edk2-staging/HBFA/UefiHostFuzzTestPkg/Conf/tools_def.txt edk2/Conf/tools_def.txt
4) build -p UefiHostFuzzTestCasePkg/UefiHostFuzzTestCasePkg.dsc -a IA32 -t AFL


Run AFLTurbo in Linux
1) mkdir testcase_dir
2) mkdir /dev/shm/findings_dir
    Note:'/dev/shm' is tmpfs, so 'findings_dir' will disappear after reboot system, please backup test result before reboot system.
3) cp edk2-staging/HBFA/UefiHostFuzzTestCasePkg/Seed/<SPECIFIC_SEED_FOLDER>/xxx.bin testcase_dir
    NOTE: <SPECIFIC_SEED_FOLDER> mapping list please refer to edk2-staging/HBFA/UefiHostFuzzTestCasePkg/Seed/readme.txt
4) afl-turbo-fuzz -i testcase_dir -o /dev/shm/findings_dir Build/UefiHostFuzzTestCasePkg/DEBUG_AFL/IA32/TestPartition @@
5)  You will see something like below. Have fun!


                    AFL-BSO 2.52b (TestPartition)

lq process timing qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqwq overall results qqqqqk
x        run time : 0 days, 0 hrs, 0 min, 5 sec        x  cycles done : 40     x
x   last new path : none yet (odd, check syntax!)      x  total paths : 1      x
x last uniq crash : none seen yet                      x uniq crashes : 0      x
x  last uniq hang : none seen yet                      x   uniq hangs : 0      x
tq cycle progress qqqqqqqqqqqqqqqqqqqqwq map coverage qvqqqqqqqqqqqqqqqqqqqqqqqu
x  now processing : 0 (0.00%)         x    map density : 0.00% / 0.00%         x
x paths timed out : 0 (0.00%)         x count coverage : 1.00 bits/tuple       x
tq stage progress qqqqqqqqqqqqqqqqqqqqnq findings in depth qqqqqqqqqqqqqqqqqqqqu
x  now trying : havoc                 x favored paths : 1 (100.00%)            x
x stage execs : 164/256 (64.06%)      x  new edges on : 1 (100.00%)            x
x total execs : 11.8k                 x total crashes : 0 (0 unique)           x
x  exec speed : 2092/sec              x  total tmouts : 0 (0 unique)           x
tq fuzzing strategy yields qqqqqqqqqqqvqqqqqqqqqqqqqqqwq path geometry qqqqqqqqu
x   bit flips : 0/32, 0/31, 0/29                      x    levels : 1          x
x  byte flips : 0/4, 0/3, 0/1                         x   pending : 0          x
x arithmetics : 0/224, 0/204, 0/68                    x  pend fav : 0          x
x  known ints : 0/8, 0/18, 0/10                       x own finds : 0          x
x  dictionary : 0/0, 0/0, 0/0                         x  imported : n/a        x
x       havoc : 0/11.0k, 0/0                          x stability : 100.00%    x
x        trim : 100.00%/29, 0.00%                     tqqqqqqqqqqqqqqqqqqqqqqqqj
^Cqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj          [cpu000: 70%]

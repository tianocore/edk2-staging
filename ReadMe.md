# VfrCompile Enhancement 
## Overview
This branch is for VfrCompile enhancement. VfrCompile knows everything in VFR files but it only outputs IFR binary. We want to enhance the vfrcompile to generate intermediate files that could include more information from VFR files so that we can better debug VFR-related issues and simplify the build process.


The branch owner: Bob Feng <bob.c.feng@intel.com>, Yuwei Chen <yuwei.chen@intel.com>

## Feature introduction
1. Generate the intermediate json file to store the efi variable structure and default value from VFR file.
2. Generate the intermediate yaml format file to describe the UI information. The yaml format is a popular format and it's easy to be consumed by other tools.
3. Change the VFR compile process from vfr->ifr to vfr->yaml->ifr.

## Timeline
Target for 2022 Q4
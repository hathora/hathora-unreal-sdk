#!/bin/bash

# This script can be used in Git Bash/MinGW/Cygwin on Windows as long as tar is installed (which it is in Git Bash)
# It assums you packaged your linux build in `./builds` such that `./builds/LinuxServer` exists

baseZip=sdk-demo-$(date +%s).tar.gz
tar -czvf ${baseZip} --exclude='*.debug' --exclude='*.sym' builds/LinuxServer Dockerfile

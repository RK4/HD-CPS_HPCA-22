#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`

echo "${green}Cloning Galois${reset}"
git clone -b release-5.0 https://github.com/IntelligentSoftwareSystems/Galois
cd Galois

# Compile Galois
echo "${green}Compiling Galois${reset}"
mkdir build_release
cd build_release
cmake ..
make -j32

cd $MAIN_DIR


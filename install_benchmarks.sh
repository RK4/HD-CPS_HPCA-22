#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

echo "${green}Copying Files${reset}"
cp $MAIN_DIR/workloads/SSSP.cpp $MAIN_DIR/Galois/lonestar/sssp
cp $MAIN_DIR/workloads/BFS_SSSP.h $MAIN_DIR/Galois/lonestar/include/Lonestar/

# Compile Galois
echo "${green}Compiling SSSP${reset}"
cd $GALOIS_DIR
make -j32

cd $PMOD_DIR
make -j32

cd $MAIN_DIR


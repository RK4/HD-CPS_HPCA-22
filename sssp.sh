#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

MAIN_DIR=`pwd`

echo "${green}Copying Files${reset}"
cp $MAIN_DIR/workloads/SSSP.cpp $MAIN_DIR/Galois/lonestar/sssp
cp $MAIN_DIR/workloads/BFS_SSSP.h $MAIN_DIR/Galois/lonestar/include/Lonestar/

# Compile Galois
echo "${green}Compiling SSSP${reset}"
cd Galois
cd build_release
make -j32

cd lonestar/sssp

echo "${green}Running SSSP with RELD${reset}"
./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 14
echo "${green}Running SSSP with OBIM${reset}"
./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0  -delta 14
echo "${green}Running SSSP with HDCPS${reset}"
./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14


cd $MAIN_DIR

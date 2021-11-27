#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

echo "${green}Copying Files${reset}"
cp $MAIN_DIR/workloads/SSSP.cpp $MAIN_DIR/Galois/lonestar/sssp
cp $MAIN_DIR/workloads/BFS_SSSP.h $MAIN_DIR/Galois/lonestar/include/Lonestar/
cp $MAIN_DIR/workloads/SSSP_2.2.1.cpp $PMOD_DIR/sssp/SSSP.cpp

cp $MAIN_DIR/workloads/bfs_2.2.1.cpp $PMOD_DIR/bfs/bfs.cpp
cp $MAIN_DIR/workloads/BFS.cpp $MAIN_DIR/Galois/lonestar/bfs


# Compile Galois
echo "${green}Compiling SSSP${reset}"
cd $GALOIS_DIR
make -j32

cd $PMOD_DIR
make -j32

cd $MAIN_DIR


#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`
export GALOIS_HOME=$MAIN_DIR/Galois
export PMOD_HOME=$MAIN_DIR/PMOD/Galois-2.2.1
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps


echo "${green}Copying Files${reset}"
cp $MAIN_DIR/workloads/SSSP.cpp $GALOIS_HOME/lonestar/sssp
cp $MAIN_DIR/workloads/BFS_SSSP.h $GALOIS_HOME/lonestar/include/Lonestar/
cp $MAIN_DIR/workloads/SSSP_2.2.1.cpp $PMOD_HOME/apps/sssp/SSSP.cpp

cp $MAIN_DIR/workloads/bfs_2.2.1.cpp $PMOD_HOME/apps/bfs/bfs.cpp
cp $MAIN_DIR/workloads/BFS.cpp $GALOIS_HOME/lonestar/bfs/bfs.cpp


# Compile Galois
echo "${green}Compiling SSSP${reset}"
cd $GALOIS_DIR
cd sssp;
make clean; make -j32;

#cd ../bfs;
#make clean; make -j32;

#cd ../astar;
#make clean; make -j32;

#cd ../boruvka;
#make clean; make -j32;

#cd ../pagerank;
#make clean; make -j32;

cd $PMOD_DIR
cd sssp;
make clean; make -j32;

#cd ../bfs;
#make clean; make -j32;

#cd ../astar;
#make clean; make -j32;

#cd ../boruvka;
#make clean; make -j32;

#cd ../pagerank;
#make clean; make -j32;

cd $MAIN_DIR


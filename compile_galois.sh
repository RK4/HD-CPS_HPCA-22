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

# For PMOD
git clone https://github.com/serifyesil/PMOD.git
cd PMOD

export GALOIS_HOME=./Galois-2.2.1/

sed -i '53i #include <math.h>' $GALOIS_HOME/include/Galois/Runtime/ParallelWork.h
sed -i '50i #include <math.h>' $GALOIS_HOME/include/Galois/WorkList/AdaptiveObim.h
sed -i 's/while(current.getRemote(i)->lock.try_lock())/while(!current.getRemote(i)->lock.try_lock());/' $GALOIS_HOME/include/Galois/WorkList/AdaptiveObim.h

#chunk sizes if needed
echo "#define CHUNK_SIZE 64" > $GALOIS_HOME/apps/sssp/chunk_size.h
echo "#define CHUNK_SIZE 64" > $GALOIS_HOME/apps/bfs/chunk_size.h
echo "#define CHUNK_SIZE 64" > $GALOIS_HOME/apps/astar/chunk_size.h
echo "#define CHUNK_SIZE 64" > $GALOIS_HOME/apps/boruvka/chunk_size.h
echo "#define CHUNK_SIZE 64" > $GALOIS_HOME/apps/pagerank/chunk_size.h

cd $GALOIS_HOME;

mkdir build_final;
cd build_final;
rm -rf *
cmake ../ -DCMAKE_BUILD_TYPE=Debug;


cd apps/sssp;
make clean; make;

cd ../bfs;
make clean; make;

cd ../astar;
make clean; make

cd ../boruvka;
make clean; make

cd ../pagerank;
make clean; make

cd $MAIN_DIR


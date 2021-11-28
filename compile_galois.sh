#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`
export GALOIS_HOME=$MAIN_DIR/Galois
export PMOD_HOME=$MAIN_DIR/PMOD/Galois-2.2.1
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

echo "${green}Cloning Galois${reset}"
git clone -b release-5.0 https://github.com/IntelligentSoftwareSystems/Galois
cd Galois

# Compile Galois
echo "${green}Compiling Galois${reset}"
mkdir build
cd build
cmake ..
make -j32

cd $MAIN_DIR

# For PMOD
git clone https://github.com/serifyesil/PMOD.git
cd PMOD

# Adding math lib for compilation
sed -i '53i #include <math.h>' $PMOD_HOME/include/Galois/Runtime/ParallelWork.h
sed -i '50i #include <math.h>' $PMOD_HOME/include/Galois/WorkList/AdaptiveObim.h
# Lock acquistion waiting bug fix
sed -i 's/while(current.getRemote(i)->lock.try_lock())/while(!current.getRemote(i)->lock.try_lock());/' $PMOD_HOME/include/Galois/WorkList/AdaptiveObim.h

#chunk sizes if needed
echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/sssp/chunk_size.h
echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/bfs/chunk_size.h
echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/astar/chunk_size.h
echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/boruvka/chunk_size.h
echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/pagerank/chunk_size.h
#echo "#define CHUNK_SIZE 64" > $PMOD_HOME/apps/color/chunk_size.h

cd $PMOD_HOME;

mkdir build;
cd build;
rm -rf *
cmake ../

cd tools;
make; 
cd ..

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

#cd ../color;
#make clean; make

cd $MAIN_DIR


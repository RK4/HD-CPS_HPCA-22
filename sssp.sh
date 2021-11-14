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

echo "" > output/sssp.out

case $1 in
    reld)
        echo "${green}Running SSSP with RELD${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out

    ;;

    obim)
        echo "${green}Running SSSP with OBIM${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out

    ;;

    pmod)
        echo "${green}Running SSSP with PMOD${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out

    ;;

    minnow)
        echo "${green}Running SSSP with Minnow-sw${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out

    ;;

    hdcps)
        echo "${green}Running SSSP with hdcps${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
    ;;


    *)
        echo "${green}Running SSSP with RELD${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
        echo "${green}Running SSSP with OBIM${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
        echo "${green}Running SSSP with PMOD${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
        echo "${green}Running SSSP with HDCPS${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
        echo "${green}Running SSSP with Minnow${reset}"
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 | grep 'pattern1\|pattern2' >> output/sssp.out
    ;;

esac



cd $MAIN_DIR

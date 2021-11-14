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

cd MAIN_DIR=`pwd`

cd PMOD/Galois-2.2.1/build_final/apps/sssp
make -j32

cd MAIN_DIR=`pwd`

echo "" > output/sssp.out

case $1 in
    reld)
        echo "${green}Running SSSP with RELD${reset}"
        echo "Running SSSP with RELD" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 0 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

    ;;

    obim)
        echo "${green}Running SSSP with OBIM${reset}"
        echo "Running SSSP with OBIM" >>  output/sssp.out
        ./PMOD/Galois-2.2.1/build_final/apps/sssp/sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0 -wl obim -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out
    ;;

    pmod)
        echo "${green}Running SSSP with PMOD${reset}"
        echo "Running SSSP with PMOD" >>  output/sssp.out 
        ./PMOD/Galois-2.2.1/build_final/apps/sssp/sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0 -wl adap-obim -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out
    ;;

    minnow)
        echo "${green}Running SSSP with Minnow-sw${reset}"
        echo "Running SSSP with Minnow-sw" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out
    ;;

    hdcps)
        echo "${green}Running SSSP with hdcps${reset}"
        echo "Running SSSP with HD-CPS" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out
    ;;


    *)
        echo "${green}Running SSSP with RELD${reset}"
        echo "Running SSSP with RELD" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 0 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

        echo "${green}Running SSSP with OBIM${reset}"
        echo "Running SSSP with OBIM" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0  -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

        echo "${green}Running SSSP with PMOD${reset}"
        echo "Running SSSP with PMOD" >>  output/sssp.out 
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 0 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

        echo "${green}Running SSSP with Minnow-sw${reset}"
        echo "Running SSSP with Minnow-sw" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

        echo "${green}Running SSSP with hdcps${reset}"
        echo "Running SSSP with HD-CPS" >>  output/sssp.out
        ./sssp ~/work/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 > temp
        grep 'Elapsed time:' >> output/sssp.out
        cat temp | grep "PD"| tail -n1 >> output/sssp.out

    ;;

esac



cd $MAIN_DIR

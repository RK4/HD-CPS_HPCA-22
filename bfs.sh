#!/bin/bash

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/build/Galois-2.2.1/build/apps

mkdir -p output
echo "" > output/sssp.out

echo "${green}Running SSSP with RELD${reset}"
echo "Running SSSP with RELD" >>  output/sssp.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/USA-road-dUSA.bin -algo deltaStep_reld -t 40 -startNode 0  -delta 0 > temp
cat temp | grep 'Elapsed time' >> output/sssp.out
cat temp | grep "PD:"| tail -n1 >> output/sssp.out


echo "${green}Running SSSP with OBIM${reset}"
echo "Running SSSP with OBIM" >>  output/sssp.out
$PMOD_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo async -t 40 -startNode 0 -wl obim -delta 14 > temp
cat temp | grep 'Elapsed time:' >> output/sssp.out
cat temp | grep "PD"| tail -n1 >> output/sssp.out

echo "${green}Running SSSP with PMOD${reset}"
echo "Running SSSP with PMOD" >>  output/sssp.out 
$PMOD_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep -t 40 -startNode 0 -wl adap-obim -delta 0 > temp
cat temp | grep 'Elapsed time:' >> output/sssp.out
cat temp | grep "PD"| tail -n1 >> output/sssp.out

echo "${green}Running SSSP with Minnow-sw${reset}"
echo "Running SSSP with Minnow-sw" >>  output/sssp.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep_minn -t 40 -startNode 0  -delta 14 > temp
cat temp | grep 'Elapsed time:' >> output/sssp.out
cat temp | grep "PD"| tail -n1 >> output/sssp.out

echo "${green}Running SSSP with hdcps${reset}"
echo "Running SSSP with HD-CPS" >>  output/sssp.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 40 -startNode 0  -delta 14 > temp
cat temp | grep 'Elapsed time:' >> output/sssp.out
cat temp | grep "PD"| tail -n1 >> output/sssp.out

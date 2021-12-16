#!/bin/bash

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

mkdir -p output
echo "" > output/bfs.out

echo "${green}Running BFS with RELD${reset}"
echo "Running BFS with RELD" >>  output/bfs.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep_reld -t 8 -startNode 0  -delta 0 > temp
cat temp | grep 'Elapsed Time' >> output/bfs.out
cat temp | grep "PD"| tail -n1 >> output/bfs.out

echo "${green}Running BFS with OBIM${reset}"
echo "Running BFS with OBIM" >>  output/bfs.out
$PMOD_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin  -t 8 -startNode 0 -wl obim -delta 9 -algo async > temp
cat temp | grep 'Elapsed Time' >> output/bfs.out
cat temp | grep "PD"| tail -n1 >> output/bfs.out

echo "${green}Running BFS with PMOD${reset}"
echo "Running BFS with PMOD" >>  output/bfs.out 
$PMOD_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin  -t 8 -startNode 0 -wl adap-obim -delta 14 -algo async > temp
cat temp | grep 'Elapsed Time' >> output/bfs.out
cat temp | grep "PD"| tail -n1 >> output/bfs.out

echo "${green}Running BFS with Minnow-sw${reset}"
echo "Running BFS with Minnow-sw" >>  output/bfs.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep_minn -t 8 -startNode 0  -delta 19  -minCores 2 > temp
cat temp | grep 'Elapsed Time' >> output/bfs.out
cat temp | grep "PD"| tail -n1 >> output/bfs.out

echo "${green}Running BFS with hdcps${reset}"
echo "Running BFS with HD-CPS" >>  output/bfs.out
$GALOIS_DIR/bfs/bfs $MAIN_DIR/datasets/USA-road-dUSA.bin -algo deltaStep_hdcps -t 8 -startNode 0  -delta 14 > temp
cat temp | grep 'Elapsed Time' >> output/bfs.out
cat temp | grep "PD"| tail -n1 >> output/bfs.out


#!/bin/bash

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

mkdir -p output
echo "" > output/MST.out

echo "${green}Running MST with RELD${reset}"
echo "Running MST with RELD" >>  output/MST.out
$GALOIS_DIR/boruvka/boruvka $MAIN_DIR/datasets/USA-road-dUSA.bin -wl reld -t 40 > temp
cat temp | grep 'Elapsed Time' >> output/MST.out
cat temp | grep "PD"| tail -n1 >> output/MST.out

echo "${green}Running MST with OBIM${reset}"
echo "Running MST with OBIM" >>  output/MST.out
$PMOD_DIR/boruvka/boruvka-merge $MAIN_DIR/datasets/USA-road-dUSA.bin  -t 40  -wl obim > temp
cat temp | grep 'Elapsed Time' >> output/MST.out
cat temp | grep "PD"| tail -n1 >> output/MST.out

echo "${green}Running MST with PMOD${reset}"
echo "Running MST with PMOD" >>  output/MST.out 
$PMOD_DIR/boruvka/boruvka-merge $MAIN_DIR/datasets/USA-road-dUSA.bin  -t 40 -wl adap-obim  > temp
cat temp | grep 'Elapsed Time' >> output/MST.out
cat temp | grep "PD"| tail -n1 >> output/MST.out

echo "${green}Running MST with Minnow-sw${reset}"
echo "Running MST with Minnow-sw" >>  output/MST.out
$GALOIS_DIR/boruvka/boruvka $MAIN_DIR/datasets/USA-road-dUSA.bin -wl minn -t 40   > temp
cat temp | grep 'Elapsed Time' >> output/MST.out
cat temp | grep "PD"| tail -n1 >> output/MST.out

echo "${green}Running MST with hdcps${reset}"
echo "Running MST with HD-CPS" >>  output/MST.out
$GALOIS_DIR/boruvka/boruvka $MAIN_DIR/datasets/USA-road-dUSA.bin -wl hdcps -t 40 > temp
cat temp | grep 'Elapsed Time' >> output/MST.out
cat temp | grep "PD"| tail -n1 >> output/MST.out


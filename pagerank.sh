#!/bin/bash

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

mkdir -p output
echo "" > output/pagerank.out

echo "${green}Running pagerank with RELD${reset}"
echo "Running pagerank with RELD" >>  output/pagerank.out
$GALOIS_DIR/pagerank/pagerank-push $MAIN_DIR/web-Google.bin  -algo Async -t 40 -wl reld > temp
cat temp | grep 'Elapsed Time' >> output/pagerank.out
cat temp | grep "PD"| tail -n1 >> output/pagerank.out

echo "${green}Running pagerank with OBIM${reset}"
echo "Running pagerank with OBIM" >>  output/pagerank.out
$GALOIS_DIR/pagerank/pagerank-push $MAIN_DIR/web-Google.bin   -algo Async -t 40 -wl obim > temp
cat temp | grep 'Elapsed Time' >> output/pagerank.out
cat temp | grep "PD"| tail -n1 >> output/pagerank.out

echo "${green}Running pagerank with PMOD${reset}"
echo "Running pagerank with PMOD" >>  output/pagerank.out 
$GALOIS_DIR/pagerank/pagerank-push $MAIN_DIR/web-Google.bin    -algo Async -t 40 -wl adap-obim > temp
cat temp | grep 'Elapsed Time' >> output/pagerank.out
cat temp | grep "PD"| tail -n1 >> output/pagerank.out

echo "${green}Running pagerank with Minnow-sw${reset}"
echo "Running pagerank with Minnow-sw" >>  output/pagerank.out
$GALOIS_DIR/pagerank/pagerank-push $MAIN_DIR/web-Google.bin   -algo Async -t 40 -wl minn > temp
cat temp | grep 'Elapsed Time' >> output/pagerank.out
cat temp | grep "PD"| tail -n1 >> output/pagerank.out

echo "${green}Running pagerank with hdcps${reset}"
echo "Running pagerank with HD-CPS" >>  output/pagerank.out
$GALOIS_DIR/pagerank/pagerank-push $MAIN_DIR/web-Google.bin   -algo Async -t 40 -wl hdcps > temp
cat temp | grep 'Elapsed Time' >> output/pagerank.out
cat temp | grep "PD"| tail -n1 >> output/pagerank.out


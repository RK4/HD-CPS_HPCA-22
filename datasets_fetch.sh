#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

#goto datasets folder
mkdir -p datasets
cd datasets

echo "${green}Fetching USA Road network${reset}"
wget -c http://users.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.USA.gr.gz
gunzip USA-road-d.USA.gr.gz

echo "${green}Converting Graph to binary format${reset}"
$MAIN_DIR/Galois/build/tools/graph-convert-standalone/graph-convert-standalone -dimacs2gr USA-road-d.USA64.gr USA-road-dUSA64.bin

#echo "${green}Fetching Web Google graph${reset}"
#wget -c https://snap.stanford.edu/data/web-Google.txt.gz
#gunzip web-Google.txt.gz

#convert the graph to binary
#echo "${green}Converting Graph to binary format${reset}"
#$MAIN_DIR/Galois/build/tools/graph-convert-standalone/graph-convert-standalone -intedgelist2gr web-Google.txt web-Google.bin 

cd ..

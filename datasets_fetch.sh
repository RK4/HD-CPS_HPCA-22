#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`
export GALOIS_HOME=$MAIN_DIR/Galois
export PMOD_HOME=$MAIN_DIR/PMOD/Galois-2.2.1
export GALOIS_DIR=$MAIN_DIR/Galois/build/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps

#goto datasets folder
mkdir -p datasets
cd datasets

echo "${green}Fetching USA Road network${reset}"
wget -c http://users.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.USA.gr.gz
gunzip USA-road-d.USA.gr.gz

echo "${green}Converting Graph to binary format${reset}"
$PMOD_HOME/build/tools/graph-convert-standalone/graph-convert-standalone -dimacs2gr USA-road-d.USA.gr USA-road-dUSA64.bin

echo "${green}Fetching Web Google graph${reset}"
wget -c https://snap.stanford.edu/data/web-Google.txt.gz
gunzip web-Google.txt.gz

echo "${green}Converting Graph to binary format${reset}"
$PMOD_HOME/build/tools/graph-convert-standalone/graph-convert-standalone -intedgelist2gr web-Google.txt web-Google.bin 

cd $MAIN_DIR

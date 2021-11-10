#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

echo "${green}Fetching USA Road network${reset}"
wget -c http://users.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.USA.gr.gz
gunzip USA-road-d.USA.gr.gz

echo "${green}Converting Graph to binary format${reset}"
$MAIN_DIR/Galois/build/tools/graph-convert-standalone/graph-convert-standalone -dimacs2gr USA-road-d.USA64.gr USA-road-dUSA64.bin
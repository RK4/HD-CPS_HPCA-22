#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

export MAIN_DIR=`pwd`
export GALOIS_DIR=$MAIN_DIR/Galois/build_release/lonestar
export PMOD_DIR=$MAIN_DIR/PMOD/Galois-2.2.1/build/apps


echo "${green}Copying Files For RELD, HDCPS, and Minnow${reset}"
cp cps/WorkListHelpers_hdcps.h ./Galois/libgalois/include/galois/worklists/WorkListHelpers.h
cp cps/Obim5.h ./Galois/libgalois/include/galois/worklists/Obim.h

echo "${green}Copying Files For PMOD and OBIM${reset}"
cp cps/AdaptiveObim.h ./PMOD/Galois-2.2.1/include/Galois/WorkList/AdaptiveObim.h
cp cps/Obim.h ./PMOD/Galois-2.2.1/include/Galois/WorkList/Obim.h

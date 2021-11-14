#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

MAIN_DIR=`pwd`

echo "${green}Copying Files For RELD and HDCPS${reset}"
cp cps/WorkListHelpers_hdcps.h ./Galois/libgalois/include/galois/worklists//WorkListHelpers.h

echo "${green}Copying Files For PMOD and OBIM${reset}"
cp cps/AdaptiveObim.h ./PMOD/Galois-2.2.1/include/Galois/WorkList/AdaptiveObim.h
cp cps/Obim.h ./PMOD/Galois-2.2.1/include/Galois/WorkList/Obim.h

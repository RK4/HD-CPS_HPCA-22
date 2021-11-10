#!/bin/bash
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

MAIN_DIR=`pwd`

echo "${green}Copying Files For Reld${reset}"
cp cps/WorkListHelpers_reld.h ./Galois/libgalois/include/galois/worklists//WorkListHelpers.h

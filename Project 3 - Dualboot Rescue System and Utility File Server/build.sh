#!/bin/bash


## Almost useless variables (for prettier output output):
red=`tput setaf 1`      #Notice: uses ` instead of ' or ".
green=`tput setaf 2`    #Notice: uses ` instead of ' or ".
yellow=`tput setaf 3`   #Notice: uses ` instead of ' or ".
magenta=`tput setaf 5`  #Notice: uses ` instead of ' or ".
reset=`tput sgr0`       #Notice: uses ` instead of ' or ".
dashes='-----------------------------------------'       #uses normal '.

## Logging stdout to a different file so only stderr is displayed for only info more useful in this situation being printed in the terminal. # Got from https://stackoverflow.com/questions/2362255/bash-scripting-checking-for-errors-logging
#reset logfile
rm ./stdout.log
#Save standard output and standard error
exec 3>&1 4>&2
# Redirect standard output to a log file
exec 1>./stdout.log
# Redirect standard error to a log file
#exec 2>/stderr.log

## NOTIFYING ONLY ERRORS WILL BE PRINTED AND STDOUT GOES TO LOG FILE:
echo "${yellow} !!!!!!     IMPORTANT: ONLY STDERR WILL BE PRINTED! FOR STDOUT OUTPUT, CHECK CORRESPONDING LOGFILE IN THE SAME DIRECTORY AS THIS SCRIPT!    !!!!!! ${reset}" 1>&3

## Variables:
echo "${magenta}${dashes}[SETTING VARIABLES]${dashes}${reset}" 1>&3
BASE_DIR=$(pwd)
TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY="./temp"
SOLN_ARCHIVE="lines-l3-skeleton-final.tar.gz"
DIRECTORY_STRUCTURE_SKELETON_FOLDER="LINES-LAB-3-FINAL-SKELETON"
BUILDROOT_FOLDER_NAME="buildroot-2022.02"
BUILDROOT_ARCHIVE_NAME="buildroot-2022.02.tar.gz"
PHP_SCRIPT_NAME="phpscript.php"
SERVER_TIME_TO_WAIT="600"
RESCUE_CONFIG_FILE_NAME="rescueFINAL.config"
UTILITY_CONFIG_FILE_NAME="utilityFINAL.config"

## Preparing directories:
echo "${magenta}${dashes}[PREPARING TEMPORARY DIRECTORY]${dashes}${reset}" 1>&3
rm -rf $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY
mkdir $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY

## Extracting solution archive: #For the solution zip, might change to using wget or git+github when I upload my solution after the results are out.
echo "${magenta}${dashes}[EXTRACTING SOLUTION ARCHIVE]${dashes}${reset}" 1>&3
tar -xvf $SOLN_ARCHIVE

## SETTING UP DIRECTORY STRUCTURE FROM SKELETON
echo "${magenta}${dashes}[SETTING UP DIRECTORY STRUCTURE]${dashes}${reset}" 1>&3
cp -a ./$DIRECTORY_STRUCTURE_SKELETON_FOLDER/. $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY
rm -rf ./$DIRECTORY_STRUCTURE_SKELETON_FOLDER

## Downloading Buildroot:
echo "${magenta}${dashes}[DOWNLOADING BUILDROOT]${dashes}${reset}" 1>&3
wget -P $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY https://buildroot.org/downloads/$BUILDROOT_ARCHIVE_NAME

## Setting up Buildroot in both Utility and Rescue directories:
echo "${magenta}${dashes}[SETTING UP BUILDROOT FOR UTILITY AND RESCUE DIRECTORIES]${dashes}${reset}" 1>&3
tar -xvf $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/$BUILDROOT_ARCHIVE_NAME  --directory $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Utility
tar -xvf $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/$BUILDROOT_ARCHIVE_NAME  --directory $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Rescue
mv $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/$UTILITY_CONFIG_FILE_NAME $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Utility/$BUILDROOT_FOLDER_NAME/.config
mv $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/$RESCUE_CONFIG_FILE_NAME $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Rescue/$BUILDROOT_FOLDER_NAME/.config
#ls -l ./temp/Utility 1>&3 #was just here for testing/debugging

## Setting up Buildroot in both Utility and Rescue directories:
echo "${magenta}${dashes}[MAKING NEW BOOT SCRIPT IMAGE \"boot.scr\" from \"boot.txt\" FOR U-BOOT]${dashes}${reset}" 1>&3
cd $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY
rm ./boot.scr #removing the boot.scr that's already in the archive
./mkimage -T script -C none -n 'Start script' -d boot.txt boot.scr
cd $BASE_DIR



## Build Recovery OS
echo "${magenta}${dashes}[Running \"make\" for building Rescue OS (and U-BOOT)]${dashes}${reset}" 1>&3
cd $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Rescue/$BUILDROOT_FOLDER_NAME
echo "PATH OF BUILDROOT: $PWD" 1>&3
make


## Changing dir back to where we started 
echo "${magenta}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}" 1>&3
cd $BASE_DIR

## Build Utility OS
echo "${magenta}${dashes}[Running \"make\" for building Utility OS]${dashes}${reset}" 1>&3
cd $TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY/Utility/$BUILDROOT_FOLDER_NAME
echo "PATH OF BUILDROOT: $PWD" 1>&3
make


## Changing dir back to where we started #This (last one in the scrupt) might not be needed as the working directory changes in the script do not affect the terminal the script itself is run in. These changes only affect the script itself during its runtime.
echo "${magenta}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}" 1>&3
cd $BASE_DIR

## RESTORING STDOUT AND STDERR TO THEIR STATES PRIOR TO LOGGING
# Restore original stdout/stderr
exec 1>&3 2>&4
# Close the unused descriptors
exec 3>&- 4>&-

## NOTIFYING COMPLETION:
echo "${green}${dashes}[FINISHED]${dashes}${reset}"
echo "Please check the logs for any errors that may have occured since this script does not check for or react to things like that at the moment."
echo "${red}Also, the rest of the steps are described in the report which should be in the same directory as this script as long as it hasn't been moved.${reset}"

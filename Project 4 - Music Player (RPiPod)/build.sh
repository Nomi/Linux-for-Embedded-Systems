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

## Variables/CONFIGURATION:
echo "${magenta}${dashes}[SETTING VARIABLES]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[SETTING VARIABLES]${dashes}${reset}" #for log file.
BASE_DIR=`pwd`
TEMP_DIR_RELATIVETOBUILDSCRIPTPARENTDIRECTORY="./temp"
TEMP_FOLDER_NAME="tempWorkspace"
SOLN_ARCHIVE="rpi4musicplayer-skeleton.tar.gz"
DIRECTORY_STRUCTURE_SKELETON_FOLDER="rpi4musicplayer-skeleton"
BUILDROOT_FOLDER_NAME="buildroot-2022.02"
BUILDROOT_ARCHIVE_NAME="buildroot-2022.02.tar.gz"
PHP_SCRIPT_NAME="phpscript.php"
SERVER_TIME_TO_WAIT="600"
CONFIG_FILE_NAME="musicplayer.config"
BR_PACKAGE_OVERLAY_FOLDER_NAME="brpackagempd"

## Preparing directories:
echo "${magenta}${dashes}[PREPARING TEMPORARY DIRECTORY]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[PREPARING TEMPORARY DIRECTORY]${dashes}${reset}" #for log file.
rm -rf $BASE_DIR/$TEMP_FOLDER_NAME
mkdir $BASE_DIR/$TEMP_FOLDER_NAME 

## Extracting solution archive: #For the solution zip, might change to using wget or git+github when I upload my solution after the results are out.
echo "${magenta}${dashes}[EXTRACTING SOLUTION ARCHIVE]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[EXTRACTING SOLUTION ARCHIVE]${dashes}${reset}" #for log file.
tar -xvf $SOLN_ARCHIVE

## SETTING UP DIRECTORY STRUCTURE FROM SKELETON
echo "${magenta}${dashes}[SETTING UP DIRECTORY STRUCTURE]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[SETTING UP DIRECTORY STRUCTURE]${dashes}${reset}" #for log file.
cp -a $BASE_DIR/$DIRECTORY_STRUCTURE_SKELETON_FOLDER/. ./$TEMP_FOLDER_NAME 
rm -rf $BASE_DIR/$DIRECTORY_STRUCTURE_SKELETON_FOLDER

## Downloading Buildroot:
echo "${magenta}${dashes}[DOWNLOADING BUILDROOT]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[DOWNLOADING BUILDROOT]${dashes}${reset}" #for log file.
#wget -P $BASE_DIR/$TEMP_FOLDER_NAME https://buildroot.org/downloads/$BUILDROOT_ARCHIVE_NAME
wget -P $BASE_DIR/$TEMP_FOLDER_NAME https://buildroot.org/downloads/$BUILDROOT_ARCHIVE_NAME

## Setting up Buildroot in both Utility and Rescue directories:
echo "${magenta}${dashes}[SETTING UP BUILDROOT]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[SETTING UP BUILDROOT]${dashes}${reset}" #for log file.
tar -xvf $BASE_DIR/$TEMP_FOLDER_NAME/$BUILDROOT_ARCHIVE_NAME  --directory $BASE_DIR/$TEMP_FOLDER_NAME
mv -f $BASE_DIR/$TEMP_FOLDER_NAME/$CONFIG_FILE_NAME $BASE_DIR/$TEMP_FOLDER_NAME/$BUILDROOT_FOLDER_NAME/.config 
cp -rf $BASE_DIR/$TEMP_FOLDER_NAME/$BR_PACKAGE_OVERLAY_FOLDER_NAME/* $BASE_DIR/$TEMP_FOLDER_NAME/$BUILDROOT_FOLDER_NAME/package #For more info on the regex used here: https://unix.stackexchange.com/a/439763
rm -rf $BASE_DIR/$TEMP_FOLDER_NAME/$BR_PACKAGE_OVERLAY_FOLDER_NAME
# FROM LAB3 VERSION OF THE SCRIPT: # ls -l ./temp/Utility 1>&3 #was just here for testing/debugging


## Building
echo "${magenta}${dashes}[RUNNING \"make\" TO START BUILD]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[RUNNING \"make\" TO START BUILD]${dashes}${reset}" #for log file.
cd $BASE_DIR/$TEMP_FOLDER_NAME/$BUILDROOT_FOLDER_NAME
echo "PATH OF BUILDROOT: $CWD" 1>&3
make


## Changing dir back to where we started #This (last one in the scrupt) might not be needed as the working directory changes in the script do not affect the terminal the script itself is run in. These changes only affect the script itself during its runtime.
echo "${magenta}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}" 1>&3
echo "${magenta}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}" #for log file.
cd $BASE_DIR

## RESTORING STDOUT AND STDERR TO THEIR STATES PRIOR TO LOGGING
echo "${magenta}${dashes}[session end]${dashes}${reset}" #for log file.
# Restore original stdout/stderr
exec 1>&3 2>&4
# Close the unused descriptors
exec 3>&- 4>&-

## NOTIFYING COMPLETION:
echo "${green}${dashes}[FINISHED]${dashes}${reset}"
echo "Please check the logs for any errors that may have occured since this script does not check for or react to things like that at the moment."
echo "${red}Also, the rest of the steps are described in the report which should be in the same directory as this script as long as it hasn't been moved.${reset}"

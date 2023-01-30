#!/bin/bash


## Almost useless variables (for prettier output output):
red=`tput setaf 1`      #Notice: uses ` instead of ' or ".
green=`tput setaf 2`    #Notice: uses ` instead of ' or ".
yellow=`tput setaf 3`   #Notice: uses ` instead of ' or ".
magenta=`tput setaf 5`  #Notice: uses ` instead of ' or ".
reset=`tput sgr0`       #Notice: uses ` instead of ' or ".
dashes='-----------------------------------------'       #uses normal '.


## Variables:
echo "${green}${dashes}[SETTING VARIABLES]${dashes}${reset}"
BASE_DIR=$(pwd)
TEMP_DIR="./temp"
SOLN_ARCHIVE="LINES-Whackamole.tar.gz"
BUILDROOT_FOLDER_NAME="buildroot-2022.02"
BUILDROOT_ARCHIVE_NAME="buildroot-2022.02.tar.gz"
PHP_SCRIPT_NAME="phpscript.php"
SERVER_TIME_TO_WAIT="600"
CONFIG_FILE_NAME="BR_Whackamole.config"


## Preparing directories:
echo "${green}${dashes}[PREPARING TEMPORARY DIRECTORY]${dashes}${reset}"
rm -rf $TEMP_DIR
mkdir $TEMP_DIR


## Extracting solution archive: #For the solution zip, might change to using wget or git+github when I upload my solution after the results are out.
echo "${green}${dashes}[EXTRACTING SOLUTION ARCHIVE]${dashes}${reset}"
tar -xvf $SOLN_ARCHIVE --directory $TEMP_DIR


## Downloading and Setting up Buildroot:
echo "${green}${dashes}[SETTING UP BUILDROOT]${dashes}${reset}"
wget -P $TEMP_DIR https://buildroot.org/downloads/$BUILDROOT_ARCHIVE_NAME
tar -xvf $TEMP_DIR/$BUILDROOT_ARCHIVE_NAME  --directory $TEMP_DIR/
cp $TEMP_DIR/$CONFIG_FILE_NAME $TEMP_DIR/$BUILDROOT_FOLDER_NAME/.config

# Further setting up for Whackamole:
cp -R $TEMP_DIR/whackamole/package_stuff/whackamole $TEMP_DIR/$BUILDROOT_FOLDER_NAME/package/whackamole
cp $TEMP_DIR/whackamole/package_stuff/Config.in $TEMP_DIR/$BUILDROOT_FOLDER_NAME/package/Config.in
#mv ./whackamole/ $TEMP_DIR 

## make
echo "${green}${dashes}[Running \"make\"]${dashes}${reset}"
cd $TEMP_DIR/$BUILDROOT_FOLDER_NAME
make


## Reminder:
#echo "${red}Remember to use \"chmod +x script_path\" on your Raspberry Pi to be able to execute the script.${reset}."
#echo "Also, in case you haven't transferred the files to your Raspberry Pi already, use SCP or HTTP server+WGET to transfer."


## Changing dir back to where we started #APPARENTLY NOT NEEDED LOL
echo "${green}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}"
cd $BASE_DIR



#For running scripts on startup (init.d), refer to documentation.
#I already did it earlier, but don't have it here. I'm sure I used a filesystem overlay to put the script in the init.d directory automatically.
#Also showed it to the teacher back then.
#Should be somewhere on Malina
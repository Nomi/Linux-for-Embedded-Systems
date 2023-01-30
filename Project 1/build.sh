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
SOLN_ARCHIVE="LINES-final-fullyworking.tar.gz"
BUILDROOT_FOLDER_NAME="buildroot-2022.02"
BUILDROOT_ARCHIVE_NAME="buildroot-2022.02.tar.gz"
PHP_SCRIPT_NAME="phpscript.php"
SERVER_TIME_TO_WAIT="600"


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
cp $TEMP_DIR/withftp.config $TEMP_DIR/$BUILDROOT_FOLDER_NAME/.config


## make
echo "${green}${dashes}[Running \"make\"]${dashes}${reset}"
cd $TEMP_DIR/$BUILDROOT_FOLDER_NAME
make


## Start http server on port 8000:  #Can use SCP or HTTP with wget to transfer script.
echo "${green}${dashes}[STARTING HTTP SERVER (final step)]${dashes}${reset}"
#mkdir ./output/images/
cp ../$PHP_SCRIPT_NAME ./output/images/$PHP_SCRIPT_NAME
cd ./output/images/
python -m http.server 8000 &> /dev/null &        #server started as background process and no output.       #DEPRECATED/WRONG INFO:(Note that nohup command hasn't been used here. Server should still close down when closing the terminal even without "kill")
SERVPID=$!
echo "${magenta}HTTP server started at port 8000 (in the location containing the PHPScript and the Image)${reset}"
echo "${magenta}The server will be automatically closed after $SERVER_TIME_TO_WAIT seconds. After which, you can only manually transfer both parts.${reset}"
echo "${yellow}IF YOU CANCEL THIS VIA SIG-INT, SERVER WILL KEEP RUNNING (INDEFINITELY?). Use \"kill ${SERVPID}\" to kill the process then.${reset}"
sleep $SERVER_TIME_TO_WAIT
kill $SERVPID


## Reminder:
echo "${red}Remember to use \"chmod +x script_path\" on your Raspberry Pi to be able to execute the script.${reset}."
echo "Also, in case you haven't transferred the files to your Raspberry Pi already, use SCP or HTTP server+WGET to transfer."


### Changing dir back to where we started #APPARENTLY NOT NEEDED LOL
#echo "${green}${dashes}[RESETTING WORKING DIRECTORY]${dashes}${reset}"
#cd $BASE_DIR



#For running scripts on startup (init.d), refer to: https://www.oreilly.com/library/view/mastering-embedded-linux/9781787283282/207cbd4e-2e42-49f3-8d5c-f72f7b438b48.xhtml
#!/bin/sh

SCRIPTPATH="`dirname \"$BASH_SOURCE\"`"
SCRIPTPATH="`( cd \"$SCRIPTPATH\" && pwd )`"

export PATH="$PATH:$SCRIPTPATH/esp_toolchain/bin"
export IDF_PATH="$SCRIPTPATH/esp-idf"

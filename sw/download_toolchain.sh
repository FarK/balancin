#!/bin/sh

toolchain_url='https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz'

wget -O - $toolchain_url | tar xz
mv xtensa-esp32-elf esp_toolchain

echo -e "\nDONE\nDo not forget to install the following dependences:"
echo -e "\tgcc git make ncurses flex bison gperf python2-pyserial"

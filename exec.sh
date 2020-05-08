#!/bin/bash
function exeasm() {
    if [ -f $1 ]
    then
        ./asm.elf $1
    else
        echo "No Input file. Using INPUT_FILENAME from linker.h"
        ./asm.elf
    fi
}
if [ -n "$1" ]
then
    case "$1" in
        -A) exeasm $2
            ./dis.elf
            ./cpu.elf ;;
        -a) exeasm $2 ;;
        -d) ./dis.elf ;;
        -c) ./cpu.elf ;;
        *) echo "$1 is not an option" ;;
    esac
else
    echo "Specify parameter: -[A]ll, -[a]sm, -[d]isasm, -[c]pu"
fi
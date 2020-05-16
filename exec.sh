#!/bin/bash
function exeasm() {
    if [ -f $1 ]
    then
        ./asm.elf $1
    else
        echo "No Input file for asm.elf. Using INPUT_FILENAME from linker.h"
        ./asm.elf
    fi
}
function exetrans() {
    ./trans.elf $1 $2
}
if [ -n "$1" ]
then
    case "$1" in
        -A) exeasm $2
            ./dis.elf
            ./cpu.elf
            exetrans $3 $4 ;;
        -a) exeasm $2 ;;
        -d) ./dis.elf ;;
        -c) ./cpu.elf ;;
        -b) exetrans $2 $3 ;;
        *) echo "$1 is not an option" ;;
    esac
else
    echo "Specify parameter: -[A]ll, -[a]sm, -[d]isasm, -[c]pu, -[b]inary"
fi
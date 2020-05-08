#!/bin/bash
if [ -n "$1" ]
then
case "$1" in
-A) ./asm.elf
    ./dis.elf
    ./cpu.elf ;;
-a) ./asm.elf ;;
-d) ./dis.elf ;;
-c) ./cpu.elf ;;
*) echo "$1 is not an option" ;;
esac
else
echo "Specify parameter: -[A]ll, -[a]sm, -[d]isasm, -[c]pu"
fi
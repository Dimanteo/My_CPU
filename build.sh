#!/bin/bash
if [ -n "$1" ]
then
case "$1" in
-A) g++ -O3 asm.cpp -o asm.elf
    g++ -O3 DisAsm.cpp -o dis.elf
    g++ -O3 cpu.cpp -o cpu.elf
    g++ -O3 translator_x86_64.cpp -o trans.elf ;;
-a) g++ -O3 asm.cpp -o asm.elf ;;
-d) g++ -O3 DisAsm.cpp -o dis.elf ;;
-c) g++ -O3 cpu.cpp -o cpu.elf ;;
-b) g++ -O3 translator_x86_64.cpp -o trans.elf ;;
*) echo "$1 is not an option" ;;
esac
else
echo "Specify parameter: -[A]ll, -[a]sm, -[d]isasm, -[c]pu"
fi

#!/bin/bash

./generateToolchain.sh


generateToolChain()

if [ "$1" == "galileo" ]
then
   echo "Compiling for Galileo Gen 2"
else
    echo "Compiling for x86_multiboot"
fi



make -C libfreertos clean all || exit

make -C ../lib_odsi_demo clean all || exit

echo "Owner" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=OWNER clean all || exit
make -C pipcore-mp/src/partitions/x86/owner/ clean all || exit
echo "sp1" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=SP1 clean all || exit
make -C pipcore-mp/src/partitions/x86/sp1Task/ all || exit
echo "sp2" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=SP2 clean all || exit
make -C pipcore-mp/src/partitions/x86/sp2Task/ all || exit
echo "sp3" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=SP3 clean all || exit
make -C pipcore-mp/src/partitions/x86/sp3Task/ all || exit
echo "Network manager" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=NETWORK clean all || exit
make -C pipcore-mp/src/partitions/x86/NetworkMngr/ clean all || exit
echo "Secure partition" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=SECURE clean all || exit
make -C pipcore-mp/src/partitions/x86/secure/ all || exit
echo "Fault partition" && sleep 1
make -C libpip/ VARIANT=virtual SERIAL_PORT=FAULT clean all || exit
make -C pipcore-mp/src/partitions/x86/fault/ all || exit
echo "Compilation des sous partition terminée" && sleep 1

yes | cp pipcore-mp/src/partitions/x86/owner/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part1.bin
yes | cp pipcore-mp/src/partitions/x86/sp1Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part2.bin
yes | cp pipcore-mp/src/partitions/x86/sp2Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part3.bin
yes | cp pipcore-mp/src/partitions/x86/sp3Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part4.bin
yes | cp pipcore-mp/src/partitions/x86/NetworkMngr/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part5.bin

make -C libpip/ VARIANT=virtual SERIAL_PORT=FREERTOS clean all || exit
#make -C pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/ clean all
make -B -C pipcore-mp/src/partitions/x86/pip-freertos/ all || exit

yes | cp pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Build/FreeRTOS.bin pipcore-mp/src/partitions/x86/multiplexer/pip-freertos.bin
yes | cp pipcore-mp/src/partitions/x86/secure/secure.bin pipcore-mp/src/partitions/x86/multiplexer/secure.bin
yes | cp pipcore-mp/src/partitions/x86/fault/fault.bin pipcore-mp/src/partitions/x86/multiplexer/fault.bin

if [ "$1" == "galileo" ]
then
    make -C libpip/ VARIANT=galileo SERIAL_PORT=MULTIPLEXER clean all || exit
else
    make -C libpip/ clean all || exit
fi

make -B -C pipcore-mp/src/partitions/x86/multiplexer || exit

if [ "$1" == "galileo" ]
then
    make -C pipcore-mp TARGET=galileo PARTITION=multiplexer clean kernel
else
    make -C pipcore-mp TARGET=x86_multiboot PARTITION=multiplexer clean kernel
fi

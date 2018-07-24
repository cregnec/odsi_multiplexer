#!/bin/bash

./generateToolchain.sh


generateToolChain()

if [ "$1" == "galileo" ]
then
   echo "Compiling for Galileo Gen 2"
else
    echo "Compiling for x86_multiboot"
fi


make -C libpip/ VARIANT=virtual clean all


make -C libfreertos clean all

echo "Owner" && sleep 1
make -C pipcore-mp/src/partitions/x86/owner/ all || exit
echo "sp1" && sleep 1
make -C pipcore-mp/src/partitions/x86/sp1Task/ all || exit
echo "sp2" && sleep 1
make -C pipcore-mp/src/partitions/x86/sp2Task/ all || exit
echo "sp3" && sleep 1
make -C pipcore-mp/src/partitions/x86/sp3Task/ all || exit
echo "Network manager" && sleep 1
make -C pipcore-mp/src/partitions/x86/NetworkMngr/ all || exit
echo "Secure partition" && sleep 1
make -C pipcore-mp/src/partitions/x86/secure/ all || exit
echo "Normal partition" && sleep 1
make -C pipcore-mp/src/partitions/x86/normal/ all || exit
echo "Compilation des sous partition terminée" && sleep 1

yes | cp pipcore-mp/src/partitions/x86/owner/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part1.bin
yes | cp pipcore-mp/src/partitions/x86/sp1Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part2.bin
yes | cp pipcore-mp/src/partitions/x86/sp2Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part3.bin
yes | cp pipcore-mp/src/partitions/x86/sp3Task/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part4.bin
yes | cp pipcore-mp/src/partitions/x86/NetworkMngr/pip-freertos.bin pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Support_Files/partitions_images/part5.bin


#make -C pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/ clean all
make -B -C pipcore-mp/src/partitions/x86/pip-freertos/ all || exit

yes | cp pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/Build/FreeRTOS.bin pipcore-mp/src/partitions/x86/multiplexer/pip-freertos.bin
yes | cp pipcore-mp/src/partitions/x86/secure/secure.bin pipcore-mp/src/partitions/x86/multiplexer/secure.bin
yes | cp pipcore-mp/src/partitions/x86/normal/normal.bin pipcore-mp/src/partitions/x86/multiplexer/normal.bin

if [ "$1" == "galileo" ]
then
    make -C libpip/ VARIANT=galileo clean all
else
    make -C libpip/ clean all
fi


make -B -C pipcore-mp/src/partitions/x86/multiplexer || exit

if [ "$1" == "galileo" ]
then
    make -C pipcore-mp TARGET=galileo PARTITION=multiplexer clean partition kernel
else
    make -C pipcore-mp TARGET=x86_multiboot PARTITION=multiplexer clean partition kernel
fi

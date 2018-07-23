#!/bin/bash

if [ "$1" == "galileo" ]
then
   echo "Compiling for Galileo Gen 2"
else
    echo "Compiling for x86_multiboot"
fi



if [ "$1" == "galileo" ]
then
    make -C libpip/ VARIANT=galileo clean clean
else
    make -C libpip/ clean clean
fi


make -C pipcore-mp/src/partitions/x86/owner/ clean
make -C pipcore-mp/src/partitions/x86/sp1Task/ clean
make -C pipcore-mp/src/partitions/x86/sp2Task/ clean
make -C pipcore-mp/src/partitions/x86/sp3Task/ clean
make -C pipcore-mp/src/partitions/x86/NetworkMngr/ clean
make -C pipcore-mp/src/partitions/x86/secure/ clean
make -C pipcore-mp/src/partitions/x86/normal/ clean
make -C pipcore-mp/src/partitions/x86/multiplexer/ clean




#make -C pipcore-mp/src/partitions/x86/pip-freertos/Demo/pip-kernel/ clean clean
make -B -C pipcore-mp/src/partitions/x86/pip-freertos/ clean

make -C pipcore-mp mrproper

make -C libfreertos clean

#include "scheduler.h"
#include "stdlib.h"
#include "stdio.h"
#include <pip/api.h>
#include <pip/vidt.h>

#define NUM_VCPUS 3
static VCPU vcpus[NUM_VCPUS];
static uint32_t nb_vcpus = 0;
static VCPU* current_vcpu = NULL;

bool initialize_vcpu(uint32_t partition_entry)
{
    if (nb_vcpus >= NUM_VCPUS){
        return false;
    }

    vcpus[nb_vcpus].partition_entry = partition_entry;
    vcpus[nb_vcpus].suspended_caller = 0;
    vcpus[nb_vcpus].started = false;

    nb_vcpus ++;
    return true;
}

void switch_to_vcpu(VCPU* vcpu)
{
    if (!vcpu->started){
        vcpu->started = true;
        // printf("Starting partition: 0x%x\r\n", vcpu->partition_entry);
        Pip_Notify(vcpu->partition_entry, 0, 0, 0);
    } else {
        if (vcpu->suspended_caller){
            uint32_t suspended_caller = vcpu->suspended_caller;
            vcpu->suspended_caller = 0;
            // printf("Asking a parent partition to resume a child partition\r\n");
            Pip_Notify(vcpu->partition_entry, 0x81, suspended_caller, 0);
        } else {
            Pip_Resume(vcpu->partition_entry, 0);
        }
    }
}

uint32_t vcpu_index(VCPU* vcpu)
{
    return (((uint32_t)vcpu - (uint32_t)&vcpus) / sizeof(VCPU));
}

void save_vcpu_caller(uint32_t caller)
{
    /* if a sub partition is suspended */
    if (current_vcpu->partition_entry != caller){
        // printf("Save suspended child partition\r\n");
        if (current_vcpu->suspended_caller){
            printf("Error: a suspended paritition was not resumed\r\n");
        }
        current_vcpu->suspended_caller = caller;
    }
}

void schedule(uint32_t caller)
{
    uint32_t next_vcpu_id = 0;
    if (nb_vcpus <= 0){
        printf("No VCPU initialized, quit\r\n");
    }

    if (current_vcpu == NULL){
        current_vcpu = &vcpus[0];
    } else {
        save_vcpu_caller(caller);
        next_vcpu_id = (vcpu_index(current_vcpu) + 1) % nb_vcpus;
        current_vcpu = &vcpus[next_vcpu_id];
    }
    switch_to_vcpu(current_vcpu);
}

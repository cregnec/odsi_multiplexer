#include "scheduler.h"
#include "stdlib.h"
#include "stdio.h"
#include <pip/api.h>
#include <pip/paging.h>
#include <pip/vidt.h>
#include <pip/debug.h>

#define NUM_VCPUS 3
static VCPU vcpus[NUM_VCPUS];
static uint32_t nb_vcpus = 0;
static VCPU* current_vcpu = NULL;

void initialize_vcpu()
{
    nb_vcpus = 0;

    while (nb_vcpus < NUM_VCPUS){
        vcpus[nb_vcpus].task = NULL;
        vcpus[nb_vcpus].idle = true;
        vcpus[nb_vcpus].next = &vcpus[(nb_vcpus+1)%NUM_VCPUS];
        nb_vcpus ++;
    }
}

TASK* vcpu_runqueue_find(VCPU* vcpu, uint32_t partition)
{
    TASK *runqueue = vcpu->task;
    while (runqueue){
        if (runqueue->partition_entry == partition){
            break;
        }
        runqueue = runqueue->next;
    }

    return runqueue;
}

void vcpu_runqueue_append(VCPU* vcpu, TASK* task)
{
    TASK **runqueue = &(vcpu->task);
    while (*runqueue){
        if (*runqueue == task){
            return;
        }
        runqueue = &((*runqueue)->next);
    }
    *runqueue = task;
    task->next = NULL;

    runqueue = &(vcpu->task);
    while (*runqueue){
        DEBUG(TRACE, "runqueue: %x\r\n", *runqueue);
        runqueue = &((*runqueue)->next);
    }
}

TASK* vcpu_runqueue_remove_head(VCPU* vcpu)
{
    TASK *task = NULL;
    TASK **runqueue = &(vcpu->task);

    if (*runqueue){
        task = *runqueue;
        *runqueue = (*runqueue)->next;
        task->next = NULL;
    }
    return task;
}

void bind_partition_2_vcpu(uint32_t partition, uint32_t vcpu_id)
{
    if (vcpu_id >= NUM_VCPUS){
        return;
    }
    TASK* task = (TASK*) Pip_AllocPage();
    DEBUG(TRACE, "task at %x\r\n", task);

    task->partition_entry = partition;
    task->started = false;
    task->runnable = true;
    task->suspended_child = 0;
    task->next = NULL;
    task->vcpu_id = vcpu_id;
    vcpu_runqueue_append(&vcpus[vcpu_id], task);

    vcpus[vcpu_id].idle = false;
}


TASK* vcpu_internal_schedule(VCPU* vcpu)
{
    TASK *next_task = NULL;

    while (next_task = vcpu_runqueue_remove_head(vcpu)){
        DEBUG(TRACE, "next_task: %x\r\n", next_task);
        if (next_task && next_task->runnable){
            vcpu_runqueue_append(vcpu, next_task);
            DEBUG(TRACE, "Appending next task partition: %x\r\n", next_task->partition_entry);
            break;
        }
    }
    DEBUG(TRACE, "next task: %x\r\n", next_task);
    return next_task;
}

void switch_to_vcpu(VCPU* vcpu)
{
    TASK* next_task = vcpu_internal_schedule(vcpu);
    if (!next_task){
        DEBUG(CRITICAL, "no available task at current_vcpu\r\n");
    }
    DEBUG(TRACE, "next task partition: %x\r\n", next_task->partition_entry);
    if (!next_task->started){
        next_task->started = true;
        DEBUG(TRACE, "Starting partition: 0x%x\r\n", next_task->partition_entry);
        Pip_Notify(next_task->partition_entry, 0, 0, 0);
    } else {
        if (next_task->suspended_child){
            uint32_t suspended_child = next_task->suspended_child;
            next_task->suspended_child = 0;
            DEBUG(INFO, "Asking a parent partition (%x) to resume a child partition (%x)\r\n", next_task->partition_entry, suspended_child);
            Pip_Notify(next_task->partition_entry, 0x81, suspended_child, 0);
        } else {
            Pip_Resume(next_task->partition_entry, 0);
        }
    }
}

uint32_t vcpu_index(VCPU* vcpu)
{
    return (((uint32_t)vcpu - (uint32_t)&vcpus) / sizeof(VCPU));
}

void save_task_caller(uint32_t caller)
{
    TASK* task = current_vcpu->task;
    /* if caller is in runqueue of current vcpu */
    if (caller == 0 || vcpu_runqueue_find(current_vcpu, caller)){
        return;
    }

    // if caller is a child of multiplexer
    if (Pip_MappedInChild(caller)){
        /* if a sub partition is suspended.*/
        DEBUG(INFO, "Save suspended child partition 0x%x for 0x%x\r\n", caller, task->partition_entry);
        if (task->suspended_child){
            DEBUG(ERROR, "A suspended paritition was not resumed\r\n");
        }
        task->suspended_child = caller;
    } else { //otherwise, it could only be the multiplexer itself
        DEBUG(ERROR, "caller %x is not child of root partition\r\n", caller);
    }
}

void initlialze_current_vcpu()
{
    int i = 0;
    for (i = 0; i < NUM_VCPUS; i++){
        if (!vcpus[i].idle) {
            current_vcpu = &vcpus[i];
        }
    }

    if (!current_vcpu){
        DEBUG(CRITICAL, "All vcpus are idling...\r\n");
    }
}

void schedule(uint32_t caller)
{
    if (!current_vcpu){
        initlialze_current_vcpu();
    } else {
        save_task_caller(caller);
        while(current_vcpu->next){
            current_vcpu = current_vcpu->next;
            if (current_vcpu->task && !current_vcpu->idle){
                break;
            }
        }
    }
    DEBUG(TRACE, "current_vcpu: %x\r\n", current_vcpu);
    switch_to_vcpu(current_vcpu);
}

void update_vcpu_status(VCPU* vcpu)
{
    bool idle = true;
    TASK* task = vcpu->task;
    while (task) {
        if (task->runnable){
            idle = false;
            DEBUG(TRACE, "set vcpu not idle\r\n");
        }
        task = task->next;
    }
    vcpu->idle = idle;
}

void mark_task_unrunnable(uint32_t partition)
{
    TASK* task = vcpu_runqueue_find(current_vcpu, partition);
    if (task){
        DEBUG(INFO, "mark partition %x as unrunnable\r\n", partition);
        task->runnable = false;
    }
    update_vcpu_status(current_vcpu);
}

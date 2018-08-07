#ifndef __SCHEDULER__
#define __SCHEDULER__

#include <stdint.h>

/* bool */
typedef uint32_t bool;

#define true    1
#define false   0


typedef struct _task {
    struct _task *next;
    uint32_t partition_entry; //parent partition
    uint32_t suspended_child; // sub partition
    uint32_t kern_stack_id;
    bool started;
    bool runnable;
    uint32_t vcpu_id;
} TASK;

typedef struct _vcpu {
    struct _task* task;
    struct _task* current_task;
    struct _vcpu* next;
    bool idle;
} VCPU;

void initialize_vcpu();
void bind_partition_2_vcpu(uint32_t partition, uint32_t vcpu_id);

void schedule(uint32_t caller);
void mark_task_unrunnable(uint32_t partition);
#endif

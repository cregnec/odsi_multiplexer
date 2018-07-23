#ifndef __SCHEDULER__
#define __SCHEDULER__

#include <stdint.h>

/* bool */
typedef uint32_t bool;

#define true    1
#define false   0

typedef struct _vcpu {
    uint32_t partition_entry; //parent partition
    uint32_t suspended_caller; // sub partition
    bool started;
} VCPU;

bool initialize_vcpu(uint32_t partition_entry);

void schedule(uint32_t caller);
#endif

# Toolchain
include ./toolchain.mk

# Target
IMAGE=FreeRTOS.bin
LINKER=elf_ia32_efi.lds
MAPFILE=RTOSDemo.map



# ODSI_DEMO library

LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/)
LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/*/include/)
LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/*/*/include/)
LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/*/*/*/include/)
LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/*/*/*/*/include/)
LIB_ODSI_DEMO_INC_DIRS+=$(wildcard $(LIBODSIDEMO)/include/*/*/*/*/include/)

LIB_ODSI_DEMO_LIB=$(LIBODSIDEMO)/lib/

#FreeRTOS library

LIBFREERTOS_LIB=$(LIBFREERTOS)/lib/
LIBFREERTOS_INC=$(LIBFREERTOS)/include/
# LibPepin directory
LIBPIP_ROOT=$(LIBPIP)

LIBPIP_INC=$(LIBPIP_ROOT)/include/
LIBPIP_LIB=$(LIBPIP_ROOT)/lib/

# FreeRTOS directories
ROOTDIR=../..
RTOS_SRCDIR=$(ROOTDIR)/Source
RTOS_IA32DIR=$(RTOS_SRCDIR)/portable/GCC/pip-kernel
RTOS_MEMDIR=$(RTOS_SRCDIR)/portable/MemMang/

RTOS_SRC=$(wildcard $(RTOS_SRCDIR)/*.c)
RTOS_IA32SRC=$(wildcard $(RTOS_IA32DIR)/*.c)
RTOS_IA32ASRC+=$(wildcard $(RTOS_IA32DIR)/*.S)
RTOS_MEMSRC=$(RTOS_MEMDIR)/heap_4.c

# RTOS Object files
RTOS_OBJ=$(RTOS_SRC:.c=.o)
RTOS_IA32OBJ=$(RTOS_IA32SRC:.c=.o)
RTOS_IA32OBJ+=$(RTOS_IA32ASRC:.S=.o)
RTOS_MEMOBJ=$(RTOS_MEMSRC:.c=.o)

# IA32 RTOS directories
IA32DIR=$(shell pwd)
SUPPORT_DIR=$(IA32DIR)/Support_Files
BLINKY_DIR=$(IA32DIR)/Blinky_Demo
FULL_DIR=$(IA32DIR)/Full_Demo
FULL_DIR_TASK=$(ROOTDIR)/Demo/Common/Minimal/

SYSTEM_CALLS_IMPLEMENTATION_SRC=$(IA32DIR)/../../../Common/System/SystemCalls.c

# IA32 RTOS files
SUPPORT_SRC=$(wildcard $(SUPPORT_DIR)/*.c)
SUPPORT_ASRC=$(wildcard $(SUPPORT_DIR)/*.S)
BLINKY_SRC=$(wildcard $(BLINKY_DIR)/*.c)
FULL_SRC=$(wildcard $(FULL_DIR)/*.c)
FULL_ASRC=$(wildcard $(FULL_DIR)/*.S)
FULL_TASK_SRC=$(wildcard $(FULL_DIR_TASK)/*.c)
DEMO_SRC=$(wildcard $(IA32DIR)/*.c)

SYSTEM_CALLS_IMPLEMENTATION_OBJ=$(SYSTEM_CALLS_IMPLEMENTATION_SRC:.c=.o)

# IA32 Object files
SUPPORT_OBJ=$(SUPPORT_SRC:.c=.o)
SUPPORT_OBJ+=$(SUPPORT_ASRC:.S=.o)
BLINKY_OBJ=$(BLINKY_SRC:.c=.o)
FULL_OBJ=$(FULL_SRC:.c=.o)
FULL_OBJ+=$(FULL_ASRC:.S=.o)
FULL_TASK_OBJ=$(FULL_TASK_SRC:.c=.o)
DEMO_OBJ=$(DEMO_SRC:.c=.o)

# Skip some unnecessary files
SKIPOBJ=$(FULL_DIR_TASK)/sp_flop.o $(FULL_DIR_TASK)/comtest.o $(FULL_DIR_TASK)/comtest_strings.o $(FULL_DIR_TASK)/crhook.o $(FULL_DIR_TASK)/AltBlock.o $(FULL_DIR_TASK)/AltQTest.o $(FULL_DIR_TASK)/AltBlckQ.o $(FULL_DIR_TASK)/AltPollQ.o $(FULL_DIR_TASK)/crflash.o $(FULL_DIR_TASK)/flash.o $(FULL_DIR_TASK)/flash_timer.o

# Include arguments for gcc
CFLAGS=-I$(FULL_DIR)
CFLAGS+=-I$(SUPPORT_DIR)

CFLAGS+=$(foreach dir, $(LIB_ODSI_DEMO_INC_DIRS), -I$(dir))

CFLAGS+=-I$(BLINKY_DIR)
CFLAGS+=-I$(RTOS_SRCDIR)/include
CFLAGS+=-I$(ROOTDIR)/Demo/Common/include
CFLAGS+=-I$(IA32DIR)
CFLAGS+=-I$(RTOS_SRCDIR)/portable/GCC/pip-kernel
CFLAGS+=-I$(LIBPIP_INC)
CFLAGS+=-I$(LIBPIP)/arch/x86/include/
CFLAGS+=-I$(LIBFREERTOS_INC)
CFLAGS+=-I../../Source/include
CFLAGS+=$(MKARGS)

# Compilation flags
CFLAGS+= -m32 -g3 -Wall -Wextra -c -fmessage-length=0 -march=pentium -Wno-ignored-qualifiers -mno-ms-bitfields -ffunction-sections -ffreestanding -fomit-frame-pointer -fno-stack-protector -g -fno-pic -no-pie
CFLAGS+=-DLOGLEVEL=Info

# Assembler flags
ASFLAGS=-I$(IA32DIR)
ASFLAGS+=-I$(RTOS_SRCDIR)/portable/GCC/pip-kernel
ASFLAGS+=-I$(LIBPIP_INC)
ASFLAGS+=-I$(LIBPIP)/arch/x86/include/
ASFLAGS+=$(CFLAGS)
ASFLAGS+=-I$(LIBFREERTOS_INC)
ASFLAGS+=-I../../Source/include
ASFLAGS+= -m32 -Wa,--gdwarf2 -Wa,-march=pentium -c


LIBGCC=$(shell dirname $(shell gcc -print-libgcc-file-name -m32))
# Linker flags
LDFLAGS=-T$(LINKER) -Map=$(BUILD_DIR)/$(MAPFILE) --gc-sections -L$(LIBGCC) -L$(LIBPIP_LIB) -L$(LIBGCC) -L$(LIBFREERTOS_LIB) -L$(LIB_ODSI_DEMO_LIB) -melf_i386
LDFLAGS2=-Telf.ld -Map=$(BUILD_DIR)/$(MAPFILE) --gc-sections -L$(LIBGCC) -L$(LIBPIP_LIB) -L$(LIBGCC) -L$(LIBFREERTOS_LIB) -L$(LIB_ODSI_DEMO_LIB) -lpip -melf_i386


BUILD_DIR=Build/

all: $(IMAGE)
	echo "All done."

clean:
	rm -rf $(RTOS_OBJ) $(RTOS_IA32OBJ) $(SUPPORT_OBJ) $(BLINKY_OBJ) $(FULL_OBJ) $(FULL_TASK_OBJ) $(DEMO_OBJ) $(SYSTEM_CALLS_IMPLEMENTATION_OBJ)

	rm -rf $(BUILD_DIR) $(IMAGE)
	rm -rf function_addr.fun

$(IMAGE): $(RTOS_OBJ) $(RTOS_IA32OBJ) $(SUPPORT_OBJ) $(BLINKY_OBJ) $(FULL_OBJ) $(FULL_TASK_OBJ) $(DEMO_OBJ) $(SYSTEM_CALLS_IMPLEMENTATION_OBJ) $(SKIPOBJ) $(RTOS_MEMOBJ)

	mkdir -pv $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $@ -lgcc -lpip -lfreertos -lodsi
	$(LD) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@ -lgcc -lpip -lfreertos -lodsi
	$(LD) $(LDFLAGS2) $^ -o $(BUILD_DIR)/FreeRTOS.elf -lgcc -lpip  -lfreertos -lodsi

$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)/Full_Demo/Standard_Demo_Tasks/Minimal/
	mkdir -pv $(BUILD_DIR)/Support_Files
	mkdir -pv $(BUILD_DIR)/FreeRTOS_Source/portable/GCC/IA32_flat/
	mkdir -pv $(BUILD_DIR)/Blinky_Demo
	mkdir -pv $(BUILD_DIR)/FreeRTOS_Source/portable/MemMang/

$(SKIPOBJ):
	echo "" > /tmp/dummy.c
	$(CC) $(CFLAGS) /tmp/dummy.c -o $@

%.o: %.S
	$(CC) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: all binary clean test

BINARY_NAME = $(shell basename $(PWD)).elf

# Construct directory
INCLUDE_DIR = Device/CMSDK_CM7/Include \
							Cmsis/6.2.0/CMSIS/Core/Include \
							Cmsis/6.2.0/CMSIS/Driver/Include \
							Device \
							Cmsis-Compiler/CMSIS-Compiler/2.1.0/include \
							. \
							FreeRTOS/portable/GCC/ARM_CM7/r0p1 \
							FreeRTOS/include
BUILD_DIR = Build
OBJ_DIR = $(BUILD_DIR)/Obj

# Construct source files and object files
ASM_SRCS = Device/CMSDK_CM7/Source/GCC/startup_CMSDK_CM7.S
CC_SRCS = $(shell find . -maxdepth 1 -type f -name '*.c') \
					Device/CMSDK_CM7/Source/system_CMSDK_CM7.c \
					Device/USART_V2M-MPS2.c \
					Cmsis-Compiler/CMSIS-Compiler/2.1.0/source/gcc/retarget_syscalls.c
FREERTOS_SRCS = FreeRTOS/list.c \
								FreeRTOS/queue.c \
								FreeRTOS/tasks.c \
								FreeRTOS/timers.c \
								FreeRTOS/portable/GCC/ARM_CM7/r0p1/port.c \
								FreeRTOS/portable/MemMang/heap_4.c
SRCS += $(ASM_SRCS) $(CC_SRCS) $(FREERTOS_SRCS)
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SRCS)))
OBJS += $(patsubst %.S,$(OBJ_DIR)/%.o,$(filter %.S,$(SRCS)))

# Constructor Compiler Commands
COMPILER_PREFIX = arm-none-eabi-
CC = $(COMPILER_PREFIX)gcc
AS = $(COMPILER_PREFIX)gcc
LD = $(COMPILER_PREFIX)gcc

# Check Compiler
ifeq ($(shell command -v $(CC)),)
  $(error Compiler $(CC) not found in PATH. Please source setupenv.sh)
endif

# Determin Flags
INCLUDES = $(addprefix -I, $(INCLUDE_DIR))
DEFINES = $(addprefix -D, CMSDK_CM7_SP)
TARGET_FLAGS := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
COMMON_CFLAGS := $(TARGET_FLAGS) $(INCLUDES) $(DEFINES) -ffunction-sections -fdata-sections -ffreestanding -std=gnu11
DEBUG_FLAGS := -Os -g
CFLAGS = $(COMMON_CFLAGS) $(DEBUG_FLAGS) -Wall -Werror
ASFLAGS = $(COMMON_CFLAGS) $(DEBUG_FLAGS) -Werror
LDFLAGS = $(TARGET_FLAGS) -T Device/CMSDK_CM7/Source/GCC/gcc_arm.ld -Wl,--gc-sections --specs=nosys.specs --specs=nano.specs
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/output.map

# Rules
test:
	@echo CFLAGS:$(CFLAGS)
	@echo ASFLAGS:$(ASFLAGS)
	@echo $(INCLUDES)
	@echo $(OBJS)

binary: $(BUILD_DIR)/$(BINARY_NAME)

clean: 
	-rm -rf $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.c
	@echo +CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# use gcc to assemble .S files, it will first preprocess them and call as
$(OBJ_DIR)/%.o: %.S
	@echo +AS $<
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(BINARY_NAME): $(OBJS)
	@echo +LK $@
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

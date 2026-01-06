.PHONY: all clean test run-elf run-bin

# --- Project Structure ---
BINARY_NAME := $(shell basename $(PWD)).bin
ELF_NAME := $(shell basename $(PWD)).elf
BUILD_DIR := build
APP_DIR := src
LIB_DIR := lib
HW_DIR := hardware

# --- Tools ---
COMPILER_PREFIX := arm-none-eabi-
CC := $(COMPILER_PREFIX)gcc
AS := $(COMPILER_PREFIX)gcc
LD := $(COMPILER_PREFIX)gcc

# --- Source Files ---
# Application sources
C_SRCS := $(shell find $(APP_DIR) -name '*.c')

# Hardware Abstraction Layer sources
C_SRCS += $(filter-out %/startup_CMSDK_CM7.c, $(shell find $(HW_DIR)/board/device -name '*.c'))
C_SRCS += $(HW_DIR)/cmsis/CMSIS-Compiler/2.1.0/source/gcc/retarget_syscalls.c
ASM_SRCS := $(shell find $(HW_DIR)/board/device/CMSDK_CM7/Source/GCC -name '*.S')

# FreeRTOS sources
FREERTOS_KERNEL_DIR := $(LIB_DIR)/CMSIS-FreeRTOS/Source
FREERTOS_PORT_DIR := $(FREERTOS_KERNEL_DIR)/portable
C_SRCS += $(FREERTOS_KERNEL_DIR)/list.c
C_SRCS += $(FREERTOS_KERNEL_DIR)/queue.c
C_SRCS += $(FREERTOS_KERNEL_DIR)/tasks.c
C_SRCS += $(FREERTOS_KERNEL_DIR)/timers.c
C_SRCS += $(FREERTOS_PORT_DIR)/GCC/ARM_CM7/r0p1/port.c
C_SRCS += $(FREERTOS_PORT_DIR)/MemMang/heap_4.c

# CMSIS-RTOS2 Wrapper
CMSIS_RTOS_DIR := $(LIB_DIR)/CMSIS-FreeRTOS/CMSIS/RTOS2/FreeRTOS
C_SRCS += $(CMSIS_RTOS_DIR)/Source/cmsis_os2.c

# Construct object file paths, placing them inside the build directory
OBJS := $(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(C_SRCS))
OBJS += $(patsubst %.S,$(BUILD_DIR)/obj/%.o,$(ASM_SRCS))

# --- Include Directories ---
INCLUDE_DIRS := $(APP_DIR)/include
INCLUDE_DIRS += $(HW_DIR)/board/device
INCLUDE_DIRS += $(HW_DIR)/board/device/CMSDK_CM7/Include
INCLUDE_DIRS += $(HW_DIR)/cmsis/CMSIS/6.2.0/CMSIS/Core/Include
INCLUDE_DIRS += $(HW_DIR)/cmsis/CMSIS-Compiler/2.1.0/include
INCLUDE_DIRS += $(HW_DIR)/cmsis/CMSIS-Compiler/2.1.0/include/gcc
INCLUDE_DIRS += $(HW_DIR)/cmsis/CMSIS/6.2.0/CMSIS/Driver/Include
INCLUDE_DIRS += $(HW_DIR)/cmsis/CMSIS/6.2.0/CMSIS/RTOS2/Include
INCLUDE_DIRS += $(FREERTOS_KERNEL_DIR)/include
INCLUDE_DIRS += $(FREERTOS_PORT_DIR)/GCC/ARM_CM7/r0p1
INCLUDE_DIRS += $(CMSIS_RTOS_DIR)/Include

# --- Macro Definitions ---
DEFINES = CMSDK_CM7_SP

# --- Build Flags ---
# Check Compiler
ifeq ($(shell command -v $(CC)),)
  $(error Compiler $(CC) not found in PATH. Please source setupenv.sh or install the toolchain.)
endif

# --- Flags ---
TARGET_FLAGS := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
COMMON_CFLAGS := $(TARGET_FLAGS) $(addprefix -I,$(INCLUDE_DIRS)) $(addprefix -D, $(DEFINES)) -ffunction-sections -fdata-sections -ffreestanding -std=gnu11
DEBUG_FLAGS := -O0 -g
CFLAGS := $(COMMON_CFLAGS) $(DEBUG_FLAGS) -Wall -Werror
ASFLAGS := $(COMMON_CFLAGS) $(DEBUG_FLAGS) -Werror
LDFLAGS := $(TARGET_FLAGS) -T $(HW_DIR)/board/device/CMSDK_CM7/Source/GCC/gcc_arm.ld -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/output.map --specs=nosys.specs --specs=nano.specs

# --- Rules ---
all: $(BUILD_DIR)/$(ELF_NAME) $(BUILD_DIR)/$(BINARY_NAME)

run-elf: $(BUILD_DIR)/$(ELF_NAME)
	@qemu-system-arm -M mps2-an500 -kernel $(BUILD_DIR)/$(ELF_NAME) -nographic

# Method one: use more realistic loading method for binary files
# Method two: easier way (but less realistic)
run-bin: $(BUILD_DIR)/$(BINARY_NAME)
	@qemu-system-arm -M mps2-an500 -device loader,file=$(BUILD_DIR)/$(BINARY_NAME),addr=0x00000000 -nographic
	@# @qemu-system-arm -M mps2-an500 -kernel build/v2m-mps2.bin -nographic

# gdb: $(BUILD_DIR)/$(ELF_NAME)
# 	@arm-none-eabi-gdb $(BUILD_DIR)/$(ELF_NAME)
test:
	@echo "Found C sources: $(C_SRCS)"
	@echo "Found ASM sources: $(ASM_SRCS)"
	@echo "Object files: $(OBJS)"
	@echo "Include paths: $(INCLUDE_DIRS)"

clean: 
	-@rm -rf $(BUILD_DIR)

# Rule to compile a C source file
$(BUILD_DIR)/obj/%.o: %.c
	@echo "[CC] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule to assemble an Assembly source file
$(BUILD_DIR)/obj/%.o: %.S
	@echo "[AS] $<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

# Rule to link the final executable
$(BUILD_DIR)/$(ELF_NAME): $(OBJS)
	@echo "[LK] $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/$(BINARY_NAME): $(BUILD_DIR)/$(ELF_NAME)
	@echo "[POST] Generating binary: $(BINARY_NAME)"
	@arm-none-eabi-objcopy -O binary $< $@

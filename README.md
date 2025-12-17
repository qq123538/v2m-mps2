# Embedded Firmware for V2M-MPS2 (Cortex-M7)

## Features

*   **MCU Target:** Arm Cortex-M7 (V2M-MPS2).
*   **RTOS:** FreeRTOS kernel, managed as a Git submodule for easy versioning and updates.
*   **Toolchain:** GCC for Arm Embedded Processors.
*   **Build System:** A flexible `Makefile`-based system.
*   **Project Structure:** Clean, modular layout separating application logic, hardware abstractions, and third-party libraries.

Hardware packs:
* Keil::V2M-MPS2_CMx_BSP@1.8.2
* ARM::CMSIS@6.2.0
* ARM::CMSIS-Compiler@2.1.0

Libraries:
* FreeRTOS

## Getting Started

Follow these steps to set up your build environment and compile the project.

### 1. Prerequisites

Ensure you have the following tools installed on your system:
*   `git`
*   `make`
*   `arm-none-eabi-gcc` toolchain
*   `qemu-system-arm`

### 2. Cloning the Repository

Clone this repository and initialize the FreeRTOS submodule in one step:

```sh
git clone --recurse-submodules https://github.com/qq123538/v2m-mps2.git
cd v2m-mps2
```
> **Note:** If you have already cloned the repository without the `--recurse-submodules` flag, initialize the submodule with: `git submodule update --init --recursive`

### 3. Environment Setup

A setup script is provided to help configure your shell environment. If you are using `vcpkg` to manage your toolchain, this script will activate it. It should be **sourced**, not executed directly.

```sh
source tools/setupenv.sh
```

## Usage

### Building the Firmware

The `Makefile` is configured to create both an ELF file (for debugging) and a raw binary file (for production flashing).

*   **To build both `.elf` and `.bin` files:**
    ```sh
    make
    ```
*   **To clean all build artifacts:**
    ```sh
    make clean
    ```
The compiled firmware (`v2m-mps2.elf`, `v2m-mps2.bin`) and a map file will be placed in the `build/` directory.

### Running in QEMU

The `Makefile` provides convenient targets for running the firmware in QEMU.

*   **To run the ELF file (recommended for most testing):**
    ```sh
    make run-elf
    ```
*   **To run the raw binary:**
    This method more closely simulates how a bootloader would load a raw binary into memory.
    ```sh
    make run-bin
    ```

### Debugging

1.  **Start QEMU in Debug Mode:**
    The `-s` flag is a shortcut for `-gdb tcp::1234`, which opens a GDB server on port 1234. The `-S` flag freezes the CPU at startup, waiting for a debugger to connect.
    ```sh
    qemu-system-arm -M mps2-an500 -kernel build/v2m-mps2.elf -nographic -s -S
    ```

2.  **Start GDB and Connect:**
    In a separate terminal, launch `arm-none-eabi-gdb`, making sure to use the **`.elf`** file which contains the debug symbols.
    ```sh
    arm-none-eabi-gdb build/v2m-mps2.elf
    ```
    Inside GDB, connect to the QEMU GDB server:
    ```gdb
    target remote localhost:1234
    ```
    You can now use standard GDB commands (`c`, `b`, `step`, `next`, etc.) to debug the firmware.

# V2M-MPS2

   The project aims to build ARM **V2M-MPS2** in Linux and run it in QEMU.
   **V2M-MPS2** is an Evaluation Board from ARM.

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
*   An Arm cross-compiler toolchain (`arm-none-eabi-gcc`).

You can use the provided `setupenv.sh` script to check for and activate the required toolchain if you are using `vcpkg`.

```sh
source tools/setupenv.sh
```

### 2. Cloning the Repository

Clone this repository and initialize the FreeRTOS submodule in one step:

```sh
git clone --recurse-submodules https://github.com/qq123538/v2m-mps2.git
cd v2m-mps2
```

If you have already cloned the repository without the `--recurse-submodules` flag, you can initialize the submodule with:
```sh
git submodule update --init --recursive
```

### 3. Building the Project

To build the application, simply run the `make` command from the project root:

```sh
make
```

This will compile all source files and generate the final executable (`v2m-mps2.elf`) and a map file in the `build/` directory.

To clean the build artifacts, run:
```sh
make clean
```
### Running in QEMU

You can run the compiled firmware directly in QEMU:
```sh
qemu-system-arm -M mps2-an500 -kernel build/v2m-mps2.elf -nographic
```

### 4. Debugging with GDB and QEMU

1.  **Start QEMU in Debug Mode:**
    The `-s` flag is a shortcut for `-gdb tcp::1234`, which opens a GDB server on port 1234. The `-S` flag freezes the CPU at startup, waiting for a debugger to connect.
    ```sh
    qemu-system-arm -M mps2-an500 -kernel build/v2m-mps2.elf -nographic -s -S
    ```

2.  **Start GDB and Connect:**
    In a separate terminal, launch the ARM GDB client and connect to the QEMU GDB server.
    ```sh
    arm-none-eabi-gdb build/v2m-mps2.elf
    ```
    Inside GDB, run the following command:
    ```gdb
    target remote localhost:1234
    ```
    You can now use standard GDB commands (`c` to continue, `b` to set breakpoints, `step`, `next`, etc.) to debug the firmware.

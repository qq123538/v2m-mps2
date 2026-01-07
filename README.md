# Embedded Firmware for V2M-MPS2 (Cortex-M7)

## Features

*   **MCU Target:** Arm Cortex-M7 (V2M-MPS2).
*   **RTOS:** FreeRTOS kernel, accessed via **CMSIS-RTOS2 API**.
*   **Toolchain:** GCC for Arm Embedded Processors.
*   **Build System:** A flexible `Makefile`-based system.
*   **Debugging:** Advanced **FreeRTOS-aware GDB scripting** written in Python, supporting task lists, backtraces, and timer inspection.
*   **Project Structure:** Clean, modular layout separating application logic, hardware abstractions, and third-party libraries.

## Hardware & Libraries
*   **Board Support:** Keil::V2M-MPS2_CMx_BSP@1.8.2
*   **CMSIS:** ARM::CMSIS@6.2.0
*   **CMSIS-Compiler:** ARM::CMSIS-Compiler@2.1.0
*   **Kernel:** FreeRTOS (Git submodule)

## Getting Started

Follow these steps to set up your build environment and compile the project.

### 1. Prerequisites

Ensure you have the following tools installed on your system:
*   `git`
*   `make`
*   `arm-none-eabi-gcc` toolchain
*   `qemu-system-arm`
*   `gdb-multiarch` (Recommended for Python scripting support)

### 2. Cloning the Repository

Clone this repository and initialize the FreeRTOS submodule:

```sh
git clone --recurse-submodules https://github.com/qq123538/v2m-mps2.git
cd v2m-mps2
```

### 3. Environment Setup

If using `vcpkg` or a custom environment setup:

```sh
source tools/setupenv.sh
```

## Usage

### Building the Firmware

*   **To build both `.elf` and `.bin` files:**
    ```sh
    make
    ```
*   **To clean all build artifacts:**
    ```sh
    make clean
    ```

### Running in QEMU

*   **Run ELF (Recommended):**
    ```sh
    make run-elf
    ```
*   **Run Binary:**
    ```sh
    make run-bin
    ```

### Advanced Debugging (FreeRTOS Awareness)

This project includes a custom GDB Python script (`tools/freertos_gdb.py`) to inspect FreeRTOS state without external plugins.

1.  **Start QEMU in Debug Mode:**
    ```sh
    qemu-system-arm -M mps2-an500 -kernel build/v2m-mps2.elf -nographic -s -S
    ```

2.  **Start GDB and Load Script:**
    ```sh
    gdb-multiarch build/v2m-mps2.elf
    ```
    Inside GDB:
    ```gdb
    target remote localhost:1234
    
    # Load the FreeRTOS inspector script
    source tools/freertos_gdb.py
    ```

3.  **Available Commands:**

    Once the scheduler is running (you may need to `continue` and then `Ctrl+C` to break), you can use:

    *   **`freertos tasks`**
        Lists all tasks with their state, priority, and stack top.
        ```text
        Address            Name             Prio   State        Stack Top
        ---------------------------------------------------------------------------
        0x200021a8         IDLE             0      *Running     0x20002230
        0x20005430         Tmr Svc          2      Blocked      0x200054b8
        ```

    *   **`freertos bt <name|address>`**
        Safely unwinds the stack of a blocked/suspended task to show where it is waiting.
        ```gdb
        freertos bt IDLE
        freertos bt Task1
        ```

    *   **`freertos timers`**
        Lists active software timers, including their ID, overflow status, and callback symbol.
        ```text
        Timer                                         ID              Period     Overflow   Status                    Callback
        ------------------------------------------------------------------------------------------------------------------------
        AutoReload_1 (0x20002d48)                     0x6             200        No         Active,AutoReload         vTimerCallback
        ```

    *   **`freertos queues`**
        Lists registered queues (requires `vQueueAddToRegistry`).

## Test Code

The file `src/timer_test.c` initializes several CMSIS-RTOS2 timers (One-Shot, Auto-Reload) to populate the lists for testing the GDB commands.


## Todo

- [ ] Add test code for queue
- [ ] Porting eazylogger

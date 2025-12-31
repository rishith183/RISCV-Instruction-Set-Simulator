# RISC-V Simulator (RV32I)

A C++ simulator for the RISC-V 32-bit Integer (RV32I) instruction set architecture. This project implements a functional CPU core capable of loading ELF binaries, executing instructions, managing byte-addressable memory, and handling system calls.

## Features

* **Instruction Set:** Full support for the **RV32I** Base Integer ISA. This includes:
  * **Arithmetic & Logic:** `ADD`, `SUB`, `AND`, `OR`, `XOR`, `SLT`, `SLTU`
  * **Shifts:** `SLL`, `SRL`, `SRA` (Logical and Arithmetic)
  * **Loads & Stores:** `LB`, `LH`, `LW`, `LBU`, `LHU`, `SB`, `SH`, `SW`
  * **Branching:** `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU`
  * **Jumps:** `JAL`, `JALR`
  * **Upper Immediates:** `LUI`, `AUIPC`
* **ELF Loading:** Capable of parsing and loading real 32-bit RISC-V ELF executables using the `ELFIO` library.
* **Memory Model:** Implements a realistic 4MB fixed virtual memory space with boundary safety checks and automatic Stack Pointer (`x2`) initialization.
* **System Calls:** Implements `ECALL` support for basic interaction:
  * Print Integer (Syscall ID 1)
  * Print String (Syscall ID 4)
  * Exit Program (Syscall ID 10)
* **Interactive Debugger:** Includes a step-by-step execution mode (`-d`) to inspect register states (`x0`-`x31`) and the Program Counter (PC) in real-time.
* **Performance Metrics:** Tracks and reports the total number of instructions executed upon completion.

## Getting Started

### Prerequisites

* **Compiler:** A C++17 compliant compiler (GCC or Clang).
* **Make:** (Optional) For automated building.

### Build

**Using Make (Recommended):**
```bash
make
```

**Using G++ directly:**
```bash
g++ -std=c++17 main.cpp CPU.cpp -o riscv_sim
```

### Usage

**1. Run a RISC-V ELF binary:**
```bash
./riscv_sim program.elf
```

**2. Run in Interactive Debug Mode:**

This allows you to step through the program instruction-by-instruction.
```bash
./riscv_sim program.elf -d
```
*Press [ENTER] to execute the next instruction. Type 'q' to quit.*

## Testing & Verification

The project includes a comprehensive test suite that verifies CPU functionality without requiring a RISC-V toolchain.

**Run all tests:**
```bash
make test
```

**Or manually:**
```bash
g++ -std=c++17 test_runner.cpp CPU.cpp -o run_tests
./run_tests
```

**Current Test Coverage:**
* **Arithmetic & Logic Test:** Validates `ADD`, `SUB`, `OR`, `AND` operations
* **Memory Operations Test:** Verifies `SW`, `LW`, `LBU` with byte-level addressing
* **Fibonacci Test:** Iteratively calculates Fibonacci(10) using branches and loops
  * **Expected Output:** `55`
  * **Instructions Used:** `ADD`, `ADDI`, `BGE`, `BEQ`

## Technical Details

* **Architecture Scope:** User-Level Simulator (RV32I Base). 
  * *Supported:* ALU operations, branching, memory (load/store), jumps, system calls
  * *Not Supported:* Privileged instructions (CSR, MRET), FENCE, atomic extensions (A-extension). These are typically handled by the OS kernel and are outside the scope of this user-mode execution engine.
* **Fetch-Decode-Execute:** The core loop strictly follows standard CPU architecture phases.
* **Sign Extension:** The simulator correctly handles signed vs. unsigned logic for arithmetic shifts, comparisons, and memory loads (e.g., distinguishing `LB` vs `LBU`).
* **Endianness:** Simulates Little-Endian memory access patterns consistent with standard RISC-V implementations.
* **Safety:** All memory accesses are bounds-checked to prevent undefined behavior.

## Implementation Highlights

* **Clean Architecture:** Separation between CPU execution logic and I/O handling
* **Instruction Counting:** Built-in performance metrics for cycle analysis
* **Quiet Mode:** Supports headless testing without verbose output
* **Error Handling:** Graceful handling of invalid instructions and out-of-bounds memory access

## Project Structure
```
.
├── main.cpp           # Entry point and command-line interface
├── CPU.h / CPU.cpp    # Core CPU implementation
├── test_runner.cpp    # Automated test suite
├── Makefile           # Build automation
└── elfio/             # ELF parsing library (header-only)
```

## Future Enhancements

Potential extensions for this project:
* **M-Extension:** Multiply/divide instructions (`MUL`, `DIV`, `REM`)
* **Pipeline Visualization:** Visual representation of instruction pipeline stages
* **Cache Simulation:** L1/L2 cache modeling with hit/miss statistics
* **Disassembler:** Human-readable instruction output during execution

## License

This project is open source and available for educational purposes.
#include <iostream>
#include <vector>
#include <cassert>
#include "CPU.h"

using namespace std;

// Test 1: Fibonacci Sequence (Iterative)
bool runFibonacciTest() {
    cout << "[TEST] Fibonacci(10) - Iterative Implementation" << endl;
    
    vector<uint32_t> program = {
        0x00a00513, // ADDI x10, x0, 10   (n = 10)
        0x00000293, // ADDI x5, x0, 0     (a = 0)
        0x00100313, // ADDI x6, x0, 1     (b = 1)
        0x00000393, // ADDI x7, x0, 0     (i = 0)
        
        // LOOP START
        // BGE x7, x10, +24 (Jump to END if i >= n)
        0x00a3dc63, 
        
        // Body
        0x00530433, // ADD x8, x6, x5     (temp = a + b)
        
        // FIX 1: ADDI x5, x6, 0 (a = b)
        // Previous bad hex: 0x00600293 (set x5=6)
        // Correct hex:      0x00030293
        0x00030293, 
        
        // FIX 2: ADDI x6, x8, 0 (b = temp)
        // Previous bad hex: 0x00800313 (set x6=8)
        // Correct hex:      0x00040313
        0x00040313, 
        
        0x00138393, // ADDI x7, x7, 1     (i++)
        
        // Jump back to LOOP START (-20 offset)
        0xfe0006e3, 
        
        // END
        // FIX 3: ADDI x10, x5, 0 (Result = a)
        // Previous bad hex: 0x00500513 (set x10=5)
        // Correct hex:      0x00028513
        0x00028513, 
        
        0x00a00893, // ADDI x17, x0, 10 (Exit)
        0x00000073  // ECALL
    };

    CPU cpu;
    cpu.setQuiet(true); // Disable logs for clean pass/fail output
    cpu.loadRaw(program);
    
    int cycles = 0;
    while(cycles < 1000) {
        if (!cpu.executeNext()) break;
        cycles++;
    }
    
    uint32_t result = cpu.getReg(10);
    
    if (result == 55) {
        cout << "   [PASS] Result matches expected (55)." << endl;
        return true;
    } else {
        cout << "   [FAIL] Expected 55, got " << result << endl;
        return false;
    }
}

// Test 2: Arithmetic & Logic
bool runArithmeticTest() {
    cout << "[TEST] Basic Arithmetic & Logic" << endl;
    
    vector<uint32_t> program = {
        0x00500513, // ADDI x10, x0, 5
        0x00300593, // ADDI x11, x0, 3
        0x00b50633, // ADD x12, x10, x11  (5+3=8)
        0x40b506b3, // SUB x13, x10, x11  (5-3=2)
        0x00b56733, // OR x14, x10, x11   (5|3=7)
        0x00b577b3, // AND x15, x10, x11  (5&3=1)
        0x00a00893, // Exit
        0x00000073 
    };
    
    CPU cpu;
    cpu.setQuiet(true);
    cpu.loadRaw(program);
    
    while(cpu.executeNext());
    
    bool pass = true;
    if (cpu.getReg(12) != 8) { cout << "   [FAIL] ADD: Expected 8, got " << cpu.getReg(12) << endl; pass = false; }
    if (cpu.getReg(13) != 2) { cout << "   [FAIL] SUB: Expected 2, got " << cpu.getReg(13) << endl; pass = false; }
    if (cpu.getReg(14) != 7) { cout << "   [FAIL] OR: Expected 7, got " << cpu.getReg(14) << endl; pass = false; }
    if (cpu.getReg(15) != 1) { cout << "   [FAIL] AND: Expected 1, got " << cpu.getReg(15) << endl; pass = false; }
    
    if (pass) cout << "   [PASS] All operations correct." << endl;
    return pass;
}

// Test 3: Memory Operations
bool runMemoryTest() {
    cout << "[TEST] Memory Operations (SW, LW, LBU)" << endl;
    
    vector<uint32_t> program = {
        0x12345337, // LUI x6, 0x12345    (x6 = 0x12345000)
        0x67830313, // ADDI x6, x6, 0x678 (x6 = 0x12345678)
        
        // Store Word (SW) x6 at address 100
        0x06602223, 
        
        // Load Word (LW) from address 100 back into x7
        0x06402383, 
        
        // Load Byte Unsigned (LBU) from address 101 back into x8
        // 0x12345678 in Little Endian at 100: 78 56 34 12
        // Address 101 should be 0x56
        0x06504403,
        
        // Exit
        0x00a00893, 
        0x00000073
    };
    
    CPU cpu;
    cpu.setQuiet(true);
    cpu.loadRaw(program);
    
    while(cpu.executeNext());
    
    bool pass = true;
    
    // Check Register Setup
    if (cpu.getReg(6) != 0x12345678) {
         cout << "   [FAIL] Register setup. x6=0x" << hex << cpu.getReg(6) << endl;
         pass = false; 
    }
    
    // Check Load Word from Memory (x7)
    if (cpu.getReg(7) != 0x12345678) {
        cout << "   [FAIL] SW/LW mismatch. Expected 0x12345678, got 0x" << hex << cpu.getReg(7) << endl;
        pass = false;
    }

    // Check Load Byte (x8)
    if (cpu.getReg(8) != 0x56) {
        cout << "   [FAIL] LBU mismatch. Expected 0x56, got 0x" << hex << cpu.getReg(8) << endl;
        pass = false;
    }
    
    if (pass) cout << "   [PASS] Memory operations verified." << endl;
    return pass;
}

int main() {
    cout << "=== RISC-V SIMULATOR TEST SUITE ===" << endl;
    int passed = 0;
    int total = 0;

    total++; if (runArithmeticTest()) passed++;
    total++; if (runMemoryTest()) passed++;
    total++; if (runFibonacciTest()) passed++;
    
    cout << "=== RESULTS: " << passed << "/" << total << " Tests Passed ===" << endl;
    return (passed == total) ? 0 : 1;
}
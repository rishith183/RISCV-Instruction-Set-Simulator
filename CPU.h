#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

using namespace std;

class CPU {
private:
    uint32_t pc;
    uint32_t regs[32];
    vector<uint8_t> memory;
    
    // Resume-Ready Feature: Quiet Mode for Unit Testing
    bool quiet_mode = false;
    
    // Resume-Ready Feature: Instruction Counting
    uint64_t instruction_count = 0;

public:
    CPU();
    
    // Core Execution
    uint32_t fetch();
    bool executeNext();
    
    // Memory Loaders
    void loadRaw(const vector<uint32_t>& code);
    bool loadELF(const string& filename);
    
    // Debugging & Visualization
    void printStatus();
    
    // Testing Utilities (New!)
    uint32_t getReg(int idx) const { 
        if (idx >= 0 && idx < 32) return regs[idx]; 
        return 0; 
    }
    
    uint64_t getInstructionCount() const { return instruction_count; }
    
    void setQuiet(bool q) { quiet_mode = q; }
};

#endif
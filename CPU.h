#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>

using namespace std;

class CPU {
private:
    uint32_t pc;
    int32_t regs[32];
    vector<uint8_t> memory;

public:
    CPU();
    
    uint32_t fetch();
    bool executeNext();
    void printStatus();
    
    // loader
    bool loadELF(const string& filename); 
    
    // Helper to access PC 
    uint32_t getPC() const { return pc; }
};

#endif
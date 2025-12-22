#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <vector>
#include <cstdint>

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
    void loadProgram(const vector<uint32_t>& program);
    void printStatus();
};

#endif
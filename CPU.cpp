#include "CPU.h"
#include <iomanip>

CPU::CPU() {
    pc = 0;
    fill(begin(regs), end(regs), 0);
    memory.resize(4096, 0);
    regs[2] = 4096; // Stack Pointer
}

uint32_t CPU::fetch() {
    if (pc >= memory.size()) return 0;
    return memory[pc] | (memory[pc+1] << 8) | (memory[pc+2] << 16) | (memory[pc+3] << 24);
}

void CPU::loadProgram(const vector<uint32_t>& program) {
    for(size_t i=0; i<program.size(); i++) {
        uint32_t word = program[i];
        memory[i*4] = word & 0xFF;
        memory[i*4+1] = (word >> 8) & 0xFF;
        memory[i*4+2] = (word >> 16) & 0xFF;
        memory[i*4+3] = (word >> 24) & 0xFF;
    }
}

void CPU::printStatus() {
    cout << "--- CPU STATE (PC: 0x" << hex << pc << ") ---" << endl;
    for(int i=0; i<32; i+=4) {
        cout << "x" << dec << i << ": " << hex << "0x" << regs[i] << "\t";
        cout << "x" << dec << i+1 << ": " << hex << "0x" << regs[i+1] << "\t";
        cout << "x" << dec << i+2 << ": " << hex << "0x" << regs[i+2] << "\t";
        cout << "x" << dec << i+3 << ": " << hex << "0x" << regs[i+3] << endl;
    }
    cout << "------------------------------------------" << endl;
}

bool CPU::executeNext() {
    uint32_t inst = fetch();
    if (inst == 0) return false;

    uint32_t opcode = inst & 0x7F;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t funct3 = (inst >> 12) & 0x7;

    regs[0] = 0; // x0 is always 0

    switch(opcode) {
        case 0x13: // ADDI
            if (funct3 == 0x0) { 
                int32_t imm = (int32_t)(inst & 0xFFF00000) >> 20;
                regs[rd] = regs[rs1] + imm;
                cout << "EXEC: ADDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
            }
            break;
            
        case 0x33: // ADD & SUB
            if (funct3 == 0x0) {
                if (inst & 0x40000000) {
                    regs[rd] = regs[rs1] - regs[rs2];
                    cout << "EXEC: SUB x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                } else {
                    regs[rd] = regs[rs1] + regs[rs2];
                    cout << "EXEC: ADD x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                }
            }
            break;

        case 0x03: // LW
            if (funct3 == 0x2) { 
                int32_t imm = (int32_t)(inst & 0xFFF00000) >> 20;
                uint32_t addr = regs[rs1] + imm;
                if (addr + 3 < memory.size()) {
                    uint32_t val = memory[addr] | (memory[addr+1] << 8) | (memory[addr+2] << 16) | (memory[addr+3] << 24);
                    regs[rd] = val;
                    cout << "EXEC: LW x" << dec << rd << " <- MEM[0x" << hex << addr << "] (Value: 0x" << val << ")" << endl;
                }
            }
            break;

        case 0x23: // SW
            if (funct3 == 0x2) { 
                int32_t imm11_5 = (inst >> 25) & 0x7F;
                int32_t imm4_0  = (inst >> 7) & 0x1F;
                int32_t imm = (imm11_5 << 5) | imm4_0;
                if (imm & 0x800) imm |= 0xFFFFF000;
                
                uint32_t addr = regs[rs1] + imm;
                if (addr + 3 < memory.size()) {
                    uint32_t val = regs[rs2];
                    memory[addr] = val & 0xFF;
                    memory[addr+1] = (val >> 8) & 0xFF;
                    memory[addr+2] = (val >> 16) & 0xFF;
                    memory[addr+3] = (val >> 24) & 0xFF;
                    cout << "EXEC: SW MEM[0x" << hex << addr << "] <- x" << dec << rs2 << " (Value: 0x" << val << ")" << endl;
                }
            }
            break;

        case 0x63: // BNE
            if (funct3 == 0x1) { 
                int32_t imm12   = (inst >> 31) & 0x1;
                int32_t imm10_5 = (inst >> 25) & 0x3F;
                int32_t imm4_1  = (inst >> 8) & 0xF;
                int32_t imm11   = (inst >> 7) & 0x1;
                int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
                if (imm12) imm |= 0xFFFFE000;

                if (regs[rs1] != regs[rs2]) {
                    pc += (imm - 4);
                    cout << "EXEC: BNE (TAKEN) -> Jumping to PC 0x" << hex << (pc + 4) << endl;
                } else {
                    cout << "EXEC: BNE (NOT TAKEN)" << endl;
                }
            }
            break;
            
        default:
            cout << "Unknown Opcode: 0x" << hex << opcode << endl;
            break;
    }
    
    pc += 4;
    return true;
}
#include "CPU.h"
#include "elfio/elfio.hpp"
#include <iomanip>
#include <algorithm>

using namespace ELFIO;

CPU::CPU() {
    pc = 0;
    std::fill(std::begin(regs), std::end(regs), 0);
    memory.resize(4 * 1024 * 1024, 0);  // 4 MB fixed memory
    regs[2] = memory.size(); // Stack Pointer initialization

    // regs[10] = 5; // Initialize x10 to 5 for testing
}

uint32_t CPU::fetch() {
    // Bounds check
    if (pc + 3 >= memory.size()) {
        if (!quiet_mode) cout << "[ERROR] PC out of bounds: 0x" << hex << pc << endl;
        return 0;
    }
    
    // Little-Endian Load
    return memory[pc] | (memory[pc+1] << 8) | (memory[pc+2] << 16) | (memory[pc+3] << 24);
}

void CPU::printStatus() {
    cout << "--- CPU STATE (PC: 0x" << hex << pc << ") ---" << endl;
    for(int i=0; i<32; i+=4) {
        cout << "x" << dec << i << ": " << hex << "0x" << regs[i] << "\t";
        cout << "x" << dec << i+1 << ": " << hex << "0x" << regs[i+1] << "\t";
        cout << "x" << dec << i+2 << ": " << hex << "0x" << regs[i+2] << "\t";
        cout << "x" << dec << i+3 << ": " << hex << "0x" << regs[i+3] << endl;
    }
    cout << "Instructions Executed: " << dec << instruction_count << endl;
    cout << "------------------------------------------" << endl;
}

bool CPU::executeNext() {
    uint32_t inst = fetch();
    if (inst == 0) return false; // Halt on null instruction

    // Increment count 
    instruction_count++;

    uint32_t opcode = inst & 0x7F;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t funct3 = (inst >> 12) & 0x7;

    regs[0] = 0; // x0 always 0

    switch(opcode) {
        case 0x13: // I-Type Arithmetic
        {
            int32_t imm = (int32_t)inst >> 20;

            // For Shift instructions, immediate is only lower 5 bits
            uint32_t shamt = imm & 0x1F; 

            switch(funct3) {
                case 0x0: regs[rd] = regs[rs1] + imm; 
                    if(!quiet_mode) cout << "EXEC: ADDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x2: regs[rd] = ((int32_t)regs[rs1] < imm) ? 1 : 0; 
                    if(!quiet_mode) cout << "EXEC: SLTI x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x3: regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)imm) ? 1 : 0; 
                    if(!quiet_mode) cout << "EXEC: SLTIU x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x4: regs[rd] = regs[rs1] ^ imm; 
                    if(!quiet_mode) cout << "EXEC: XORI x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x6: regs[rd] = regs[rs1] | imm; 
                    if(!quiet_mode) cout << "EXEC: ORI x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x7: regs[rd] = regs[rs1] & imm; 
                    if(!quiet_mode) cout << "EXEC: ANDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl; break;
                case 0x1: regs[rd] = regs[rs1] << shamt; 
                    if(!quiet_mode) cout << "EXEC: SLLI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl; break;
                case 0x5: 
                    if (inst & 0x40000000) { regs[rd] = (int32_t)regs[rs1] >> shamt; if(!quiet_mode) cout << "EXEC: SRAI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl; }
                    else { regs[rd] = (uint32_t)regs[rs1] >> shamt; if(!quiet_mode) cout << "EXEC: SRLI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl; }
                    break;
                // FIX 3: Default case
                default:
                    if(!quiet_mode) cout << "[ERROR] Unknown I-Type funct3: " << funct3 << endl;
                    break;
            }
            break;
        }
        case 0x33: // R-Type Arithmetic
        {
            switch(funct3) {
                case 0x0: 
                    if (inst & 0x40000000) { regs[rd] = regs[rs1] - regs[rs2]; if(!quiet_mode) cout << "EXEC: SUB x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; }
                    else { regs[rd] = regs[rs1] + regs[rs2]; if(!quiet_mode) cout << "EXEC: ADD x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; }
                    break;
                case 0x1: regs[rd] = regs[rs1] << (regs[rs2] & 0x1F); if(!quiet_mode) cout << "EXEC: SLL x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                case 0x2: regs[rd] = ((int32_t)regs[rs1] < (int32_t)regs[rs2]) ? 1 : 0; if(!quiet_mode) cout << "EXEC: SLT x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                case 0x3: regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) ? 1 : 0; if(!quiet_mode) cout << "EXEC: SLTU x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                case 0x4: regs[rd] = regs[rs1] ^ regs[rs2]; if(!quiet_mode) cout << "EXEC: XOR x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                case 0x5: 
                    if (inst & 0x40000000) { regs[rd] = (int32_t)regs[rs1] >> (regs[rs2] & 0x1F); if(!quiet_mode) cout << "EXEC: SRA x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; }
                    else { regs[rd] = (uint32_t)regs[rs1] >> (regs[rs2] & 0x1F); if(!quiet_mode) cout << "EXEC: SRL x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; }
                    break;
                case 0x6: regs[rd] = regs[rs1] | regs[rs2]; if(!quiet_mode) cout << "EXEC: OR x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                case 0x7: regs[rd] = regs[rs1] & regs[rs2]; if(!quiet_mode) cout << "EXEC: AND x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl; break;
                // FIX 3: Default case
                default:
                    if(!quiet_mode) cout << "[ERROR] Unknown R-Type funct3: " << funct3 << endl;
                    break;
            }
            break;
        }
        case 0x03: // Loads
        {
            int32_t imm = (int32_t)inst >> 20;
            uint32_t addr = regs[rs1] + imm;
            // FIX 4: Halt on OOB
            if (addr + 3 >= memory.size()) { 
                if(!quiet_mode) cout << "[ERROR] Load OOB at 0x" << hex << addr << endl; 
                return false; 
            }
           switch(funct3) {
                case 0x0: regs[rd] = (int8_t)memory[addr]; if(!quiet_mode) cout << "EXEC: LB" << endl; break;
                case 0x1: regs[rd] = (int16_t)(memory[addr] | (memory[addr+1]<<8)); if(!quiet_mode) cout << "EXEC: LH" << endl; break;
                case 0x2: regs[rd] = memory[addr] | (memory[addr+1]<<8) | (memory[addr+2]<<16) | (memory[addr+3]<<24); if(!quiet_mode) cout << "EXEC: LW" << endl; break;
                case 0x4: regs[rd] = memory[addr]; if(!quiet_mode) cout << "EXEC: LBU" << endl; break;
                case 0x5: regs[rd] = memory[addr] | (memory[addr+1]<<8); if(!quiet_mode) cout << "EXEC: LHU" << endl; break;
                default: if(!quiet_mode) cout << "[ERROR] Unknown Load funct3" << endl; break;
            }
            break;
        }

        case 0x23: // STORE Instructions
        {
            int32_t imm11_5 = (inst >> 25) & 0x7F;
            int32_t imm4_0  = (inst >> 7) & 0x1F;
            int32_t imm = (imm11_5 << 5) | imm4_0;
            if (imm & 0x800) imm |= 0xFFFFF000;
            
            uint32_t addr = regs[rs1] + imm;
            uint32_t val = regs[rs2];
            // FIX 4: Halt on OOB
            if (addr + 3 >= memory.size()) { 
                if(!quiet_mode) cout << "[ERROR] Store OOB at 0x" << hex << addr << endl; 
                return false; 
            }
           switch(funct3) {
                case 0x0: memory[addr] = val & 0xFF; if(!quiet_mode) cout << "EXEC: SB" << endl; break;
                case 0x1: memory[addr] = val & 0xFF; memory[addr+1] = (val>>8) & 0xFF; if(!quiet_mode) cout << "EXEC: SH" << endl; break;
                case 0x2: memory[addr] = val & 0xFF; memory[addr+1] = (val>>8) & 0xFF; memory[addr+2] = (val>>16) & 0xFF; memory[addr+3] = (val>>24) & 0xFF; if(!quiet_mode) cout << "EXEC: SW" << endl; break;
                default: if(!quiet_mode) cout << "[ERROR] Unknown Store funct3" << endl; break;
            }
            break;
        }

        case 0x63: // BRANCH Instructions
        {
            int32_t imm12 = (inst >> 31) & 0x1;
            int32_t imm10_5 = (inst >> 25) & 0x3F;
            int32_t imm4_1 = (inst >> 8) & 0xF;
            int32_t imm11 = (inst >> 7) & 0x1;
            int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
            if (imm12) imm |= 0xFFFFE000;

            bool take = false;
            switch(funct3) {
                case 0x0: take = (regs[rs1] == regs[rs2]); break;
                case 0x1: take = (regs[rs1] != regs[rs2]); break;
                case 0x4: take = ((int32_t)regs[rs1] < (int32_t)regs[rs2]); break;
                case 0x5: take = ((int32_t)regs[rs1] >= (int32_t)regs[rs2]); break;
                case 0x6: take = (regs[rs1] < regs[rs2]); break;
                case 0x7: take = (regs[rs1] >= regs[rs2]); break;
                default: if(!quiet_mode) cout << "[ERROR] Unknown Branch funct3" << endl; break;
            }
            if(!quiet_mode) cout << "EXEC: BRANCH " << (take ? "TAKEN" : "NOT TAKEN") << endl;
            if (take) pc += (imm - 4);
            break;
        }
        
        case 0x37: // LUI
            regs[rd] = inst & 0xFFFFF000; 
            if(!quiet_mode) cout << "EXEC: LUI x" << dec << rd << endl;
            break;
            
        case 0x17: // AUIPC
            regs[rd] = pc + (inst & 0xFFFFF000); 
            if(!quiet_mode) cout << "EXEC: AUIPC x" << dec << rd << endl;
            break;
            
        case 0x6F: // JAL
       {
            int32_t imm20 = (inst >> 31) & 0x1;
            int32_t imm10_1 = (inst >> 21) & 0x3FF;
            int32_t imm11 = (inst >> 20) & 0x1;
            int32_t imm19_12 = (inst >> 12) & 0xFF;
            
            // Reassemble the 20-bit offset
            int32_t offset = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
            if (offset & 0x100000) offset |= 0xFFE00000;
            regs[rd] = pc + 4;
            pc += offset;
            if(!quiet_mode) cout << "EXEC: JAL -> 0x" << hex << pc << endl;
            // FIX 1: Return immediately, instruction count already incremented at start
            return true;
         }

        case 0x67: // JALR (Jump and Link Register) - Function Return
        {
            // FIX 2: Validate funct3 is 0
            if (funct3 != 0x0) {
                if(!quiet_mode) cout << "[ERROR] Invalid JALR funct3: " << funct3 << endl;
                break;
            }
            
            int32_t imm = (int32_t)(inst & 0xFFF00000) >> 20;
            uint32_t target = (regs[rs1] + imm) & ~1;
            regs[rd] = pc + 4;
            pc = target;
            if(!quiet_mode) cout << "EXEC: JALR -> 0x" << hex << pc << endl;
            // FIX 1: Return immediately
            return true;
         }

        case 0x73: // ECALL (System Calls)
        {
            uint32_t syscall = regs[17];
            if (syscall == 10) { 
                if(!quiet_mode) cout << "SYSCALL: EXIT" << endl; 
                return false; 
            }
            if (syscall == 1) { 
                if(!quiet_mode) cout << "SYSCALL: Print Int -> " << dec << (int32_t)regs[10] << endl; 
            }
            if (syscall == 4) { // Print String
                uint32_t addr = regs[10]; // Address of string is in x10
                string output = "";
                while(addr < memory.size()) {
                    char c = (char)memory[addr];
                    if (c == '\0') break; // Stop at null terminator
                    output += c;
                    addr++;
                }
                if(!quiet_mode) cout << "SYSCALL: Print Str -> " << output << endl; 
            }
           break;
        }
        
        default:
            if(!quiet_mode) cout << "[ERROR] Unknown Opcode: 0x" << hex << opcode << endl;
            return false;
   }
    
    pc += 4;
    return true;
}

void CPU::loadRaw(const vector<uint32_t>& code) {
    // Reset memory
    std::fill(memory.begin(), memory.end(), 0);
    
    // Copy code into memory (byte by byte)
    size_t addr = 0;
    for (uint32_t inst : code) {
        if (addr + 4 > memory.size()) break;
        memory[addr] = inst & 0xFF;
        memory[addr+1] = (inst >> 8) & 0xFF;
        memory[addr+2] = (inst >> 16) & 0xFF;
        memory[addr+3] = (inst >> 24) & 0xFF;
        addr += 4;
    }
    pc = 0;
    if(!quiet_mode) cout << "Loaded " << code.size() * 4 << " bytes raw." << endl;
}

bool CPU::loadELF(const string& filename) {
    elfio reader;
    if (!reader.load(filename)) return false;
    if (reader.get_machine() != EM_RISCV) return false;
    std::fill(memory.begin(), memory.end(), 0);
    for (const auto& segment : reader.segments) {
        if (segment->get_type() == PT_LOAD) {
            uint32_t addr = (uint32_t)segment->get_virtual_address();
            uint32_t fsize = (uint32_t)segment->get_file_size();
            uint32_t msize = (uint32_t)segment->get_memory_size();
            if (addr + msize > memory.size()) return false;
            if (fsize > 0) memcpy(&memory[addr], segment->get_data(), fsize);
        }
    }

    // Set the Program Counter (PC)
    pc = (uint32_t)reader.get_entry();
    if(!quiet_mode) cout << "Loaded ELF Entry: 0x" << hex << pc << endl;
    return true;
}
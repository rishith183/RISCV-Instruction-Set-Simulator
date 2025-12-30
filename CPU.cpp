#include "CPU.h"
#include "elfio/elfio.hpp"
#include <iomanip>
#include <algorithm>

using namespace ELFIO;

CPU::CPU() {
    pc = 0;
    std::fill(std::begin(regs), std::end(regs), 0);
    memory.resize(4096, 0); 
    regs[2] = 4096; // Stack Pointer initialization

    // regs[10] = 5; // Initialize x10 to 5 for testing
}

uint32_t CPU::fetch() {
    // Safety check to prevent out-of-bounds access
    if (pc >= memory.size()) return 0;
    
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
    cout << "------------------------------------------" << endl;
}

bool CPU::executeNext() {
    uint32_t inst = fetch();
    if (inst == 0) return false; // HALT on 0x00000000

    uint32_t opcode = inst & 0x7F;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t funct3 = (inst >> 12) & 0x7;

    regs[0] = 0; // Enforce x0 = 0 invariant

    switch(opcode) {

        case 0x13: // Immediate Arithmetic
        {
            int32_t imm = (int32_t)inst >> 20;
            // For Shift instructions, immediate is only lower 5 bits
            uint32_t shamt = imm & 0x1F; 

            switch(funct3) {
                case 0x0: // ADDI
                    regs[rd] = regs[rs1] + imm;
                    cout << "EXEC: ADDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x2: // SLTI (Set Less Than Immediate)
                    regs[rd] = (regs[rs1] < imm) ? 1 : 0;
                    cout << "EXEC: SLTI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x3: // SLTIU (Unsigned)
                    regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)imm) ? 1 : 0;
                    cout << "EXEC: SLTIU x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x4: // XORI
                    regs[rd] = regs[rs1] ^ imm;
                    cout << "EXEC: XORI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x6: // ORI
                    regs[rd] = regs[rs1] | imm;
                    cout << "EXEC: ORI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x7: // ANDI
                    regs[rd] = regs[rs1] & imm;
                    cout << "EXEC: ANDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                    break;
                case 0x1: // SLLI (Shift Left Logical)
                    regs[rd] = regs[rs1] << shamt;
                    cout << "EXEC: SLLI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl;
                    break;
                case 0x5: // SRLI & SRAI
                    if (inst & 0x40000000) { // Bit 30 checks if it's SRAI (Arithmetic Right)
                        regs[rd] = (int32_t)regs[rs1] >> shamt; // C++ handles arithmetic shift on signed types
                        cout << "EXEC: SRAI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl;
                    } else { // SRLI (Logical Right)
                        regs[rd] = (uint32_t)regs[rs1] >> shamt;
                        cout << "EXEC: SRLI x" << dec << rd << ", x" << rs1 << ", " << shamt << endl;
                    }
                    break;
                default:
                    cout << "Unknown I-Type Funct3: " << funct3 << endl;
                    break;
            }
            break;
        }
            
        case 0x33: // Register Arithmetic
        {
            switch(funct3) {
                case 0x0: // ADD & SUB
                    if (inst & 0x40000000) { // Bit 30 sets Sub
                        regs[rd] = regs[rs1] - regs[rs2];
                        cout << "EXEC: SUB x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    } else {
                        regs[rd] = regs[rs1] + regs[rs2];
                        cout << "EXEC: ADD x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    }
                    break;
                case 0x1: // SLL
                    regs[rd] = regs[rs1] << (regs[rs2] & 0x1F);
                    cout << "EXEC: SLL x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
                case 0x2: // SLT
                    regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;
                    cout << "EXEC: SLT x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
                case 0x3: // SLTU
                    regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) ? 1 : 0;
                    cout << "EXEC: SLTU x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
                case 0x4: // XOR
                    regs[rd] = regs[rs1] ^ regs[rs2];
                    cout << "EXEC: XOR x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
                case 0x5: // SRL & SRA
                    if (inst & 0x40000000) { // SRA
                        regs[rd] = (int32_t)regs[rs1] >> (regs[rs2] & 0x1F);
                        cout << "EXEC: SRA x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    } else { // SRL
                        regs[rd] = (uint32_t)regs[rs1] >> (regs[rs2] & 0x1F);
                        cout << "EXEC: SRL x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    }
                    break;
                case 0x6: // OR
                    regs[rd] = regs[rs1] | regs[rs2];
                    cout << "EXEC: OR x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
                case 0x7: // AND
                    regs[rd] = regs[rs1] & regs[rs2];
                    cout << "EXEC: AND x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                    break;
            }
            break;
        }

        case 0x03: // LW
            if (funct3 == 0x2) { 
                int32_t imm = (int32_t)inst >> 20;
                uint32_t addr = regs[rs1] + imm;
                if (addr + 3 < memory.size()) {
                    uint32_t val = memory[addr] | (memory[addr+1] << 8) | (memory[addr+2] << 16) | (memory[addr+3] << 24);
                    regs[rd] = val;
                    cout << "EXEC: LW x" << dec << rd << " <- MEM[0x" << hex << addr << "] (Value: 0x" << hex << val << ")" << endl;
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
                    cout << "EXEC: SW MEM[0x" << hex << addr << "] <- x" << dec << rs2 << " (Value: 0x" << hex << val << ")" << endl;
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

                cout << "EXEC: BNE Checking x" << dec << rs1 << "(" << (int32_t)regs[rs1] << ") vs x" << rs2 << "(" << (int32_t)regs[rs2] << ")" << endl;

                if (regs[rs1] != regs[rs2]) {
                    pc += (imm - 4);
                    cout << "   -> TAKEN. Jumping to PC 0x" << hex << (pc + 4) << endl;
                }
                else {
                    cout << "EXEC: BNE (NOT TAKEN)" << endl;
                }
            }
            break;
        
        case 0x37: // LUI (Load Upper Immediate)
        {
            int32_t imm = inst & 0xFFFFF000; // Extract top 20 bits
            regs[rd] = imm;
            cout << "EXEC: LUI x" << dec << rd << ", 0x" << hex << imm << endl;
            break;
        }

        case 0x17: // AUIPC (Add Upper Immediate to PC)
        {
            int32_t imm = inst & 0xFFFFF000;
            regs[rd] = pc + imm; // Adds offset to current PC
            cout << "EXEC: AUIPC x" << dec << rd << ", 0x" << hex << imm << endl;
            break;
        }

        case 0x6F: // JAL (Jump and Link) - Function Call
        {
            // J-Type Immediate Decoding (The scrambled bits)
            int32_t imm20 = (inst >> 31) & 0x1;
            int32_t imm10_1 = (inst >> 21) & 0x3FF;
            int32_t imm11 = (inst >> 20) & 0x1;
            int32_t imm19_12 = (inst >> 12) & 0xFF;
            
            // Reassemble the 20-bit offset
            int32_t offset = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
            
            // Sign-Extend to 32 bits
            if (offset & 0x100000) offset |= 0xFFE00000; 

            regs[rd] = pc + 4; // Save return address (link)
            pc += offset;      // Jump to target
            
            cout << "EXEC: JAL (Jump) -> PC is now 0x" << hex << pc << endl;
            return true; // Return immediately (Do not do pc += 4)
        }

        case 0x67: // JALR (Jump and Link Register) - Function Return
        {
            if (funct3 == 0x0) {
                int32_t imm = (int32_t)(inst & 0xFFF00000) >> 20;
                uint32_t target = regs[rs1] + imm;
                target &= ~1; // Clear LSB (Hardware requirement)
                
                regs[rd] = pc + 4; // Save return address
                pc = target;       // Jump
                
                cout << "EXEC: JALR (Return) -> PC is now 0x" << hex << pc << endl;
                return true; // Return immediately
            }
            break;
        }

        case 0x73: // ECALL (System Calls)
        {
            uint32_t syscall_num = regs[17]; 

            switch(syscall_num) {
                case 1: // Print Integer
                    cout << "SYSCALL: Print Int -> " << dec << (int32_t)regs[10] << endl;
                    break;
                    
                case 4: // Print String
                {
                    uint32_t addr = regs[10];
                    string output = "";
                    while(addr < memory.size()) {
                        char c = (char)memory[addr];
                        if (c == '\0') break;
                        output += c;
                        addr++;
                    }
                    cout << "SYSCALL: Print Str -> " << output << endl;
                    break;
                }
                    
                case 10: // Exit
                    cout << "SYSCALL: Exit called." << endl;
                    return false; 
                    
                default:
                    cout << "Unknown Syscall: " << syscall_num << endl;
                    break;
            }
            break;
        }
            
        default:
            cout << "Unknown Opcode: 0x" << hex << opcode << endl;
            break;
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
        memory[addr]   = inst & 0xFF;
        memory[addr+1] = (inst >> 8) & 0xFF;
        memory[addr+2] = (inst >> 16) & 0xFF;
        memory[addr+3] = (inst >> 24) & 0xFF;
        addr += 4;
    }
    pc = 0; // Reset PC
    cout << "Loaded " << code.size() * 4 << " bytes of raw machine code." << endl;
}

bool CPU::loadELF(const string& filename) {
    elfio reader;
    
    // 1. Load the file
    if (!reader.load(filename)) {
        cerr << "Error: Could not process ELF file: " << filename << endl;
        return false;
    }

    // 2. Check if it is a RISC-V binary
    if (reader.get_machine() != EM_RISCV) {
        cerr << "Error: Defined file is not a RISC-V binary." << endl;
        return false;
    }

    // 3. Clear existing memory
    std::fill(memory.begin(), memory.end(), 0);

    // 4. Iterate over "Segments" 
    for (const auto& segment : reader.segments) {
        if (segment->get_type() == PT_LOAD) {
            uint32_t address = (uint32_t)segment->get_virtual_address();
            uint32_t fileSize = (uint32_t)segment->get_file_size();
            uint32_t memSize = (uint32_t)segment->get_memory_size();

            // Check boundaries
            if (address + memSize > memory.size()) {
                if (address + memSize < 1024 * 1024 * 4) { // Limit to 4MB
                     memory.resize(address + memSize);
                } else {
                     cerr << "Error: Segment exceeds memory bounds." << endl;
                     return false;
                }
            }

            // Copy data from file to our memory vector
            if (fileSize > 0) {
                const char* data = segment->get_data();
                memcpy(&memory[address], data, fileSize);
            }
        }
    }

    // 5. Set the Program Counter (PC)
    pc = (uint32_t)reader.get_entry();
    cout << "Loaded ELF. Entry point set to: 0x" << hex << pc << endl;
    
    return true;
}
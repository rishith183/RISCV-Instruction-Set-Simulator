#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>

using namespace std;

// --- DEFINITIONS ---
// RISC-V has 32 registers. x0 is always 0.
// We use 'uint32_t' because registers are exactly 32 bits (4 bytes).
class CPU {
private:
    uint32_t pc;        // Program Counter (Points to current instruction)
    int32_t regs[32];   // 32 General Purpose Registers (x0 - x31)
    vector<uint8_t> memory; // Main Memory (RAM)

public:

    CPU() {
        pc = 0;
        fill(begin(regs), end(regs), 0);
        memory.resize(4096, 0); // Create 4KB of RAM, initialized to 0
        
        // Stack Pointer (x2) usually starts at the END of memory
        regs[2] = 4096; 
    }

    void printStatus() {
        cout << "--- CPU STATE (PC: 0x" << hex << pc << ") ---" << endl;
        for(int i=0; i<32; i+=4) {
            cout << "x" << dec << i << ": " << hex << "0x" << regs[i] << "\t";
            cout << "x" << dec << i+1 << ": " << hex << "0x" << regs[i+1] << "\t";
            cout << "x" << dec << i+2 << ": " << hex << "0x" << regs[i+2] << "\t";
            cout << "x" << dec << i+3 << ": " << hex << "0x" << regs[i+3] << endl;
        }
        cout << "------------------------------------------" << endl;
    }

    // 1. FETCH: Get the next instruction from memory
    uint32_t fetch() {
        if (pc >= memory.size()) {
            cout << "Error: PC out of bounds!" << endl;
            return 0; 
        }

        // We must combine 4 separate bytes into one 32-bit word.
        uint32_t instruction = 
            memory[pc] | 
            (memory[pc+1] << 8) | 
            (memory[pc+2] << 16) | 
            (memory[pc+3] << 24);
            
        return instruction;
    }

    // 2. EXECUTE: The Brains
    // Returns false if we should stop (Halt), true otherwise
    bool executeNext() {
        uint32_t inst = fetch();
        
        // If instruction is 0, just stop (simplified halt)
        if (inst == 0) return false;

        // --- DECODING (Bit Slicing) ---
        // Opcode is the last 7 bits (AND with 1111111 or 0x7F)
        uint32_t opcode = inst & 0x7F;
        
        // Destination Register (rd) is bits 7-11
        uint32_t rd = (inst >> 7) & 0x1F;
        
        // Source Register 1 (rs1) is bits 15-19
        uint32_t rs1 = (inst >> 15) & 0x1F;
        
        // Source Register 2 (rs2) is bits 20-24
        uint32_t rs2 = (inst >> 20) & 0x1F;

        // Funct3 is bits 12-14 (Helper to distinguish instructions)
        uint32_t funct3 = (inst >> 12) & 0x7;

        // --- EXECUTION SWITCH ---
        switch(opcode) {
            case 0x13: // I-Type
                if (funct3 == 0x0) { 
                    // example: ADDI: Add Immediate. (x1 = x2 + 5)
                    // extract the 12-bit "Immediate" number (bits 20-31)
                    int32_t imm = (int32_t)(inst & 0xFFF00000) >> 20; // Sign extend
                    
                    regs[rd] = regs[rs1] + imm;
                    cout << "EXEC: ADDI x" << dec << rd << ", x" << rs1 << ", " << imm << endl;
                }
                break;

            case 0x33: // R-Type
                if (funct3 == 0x0) { 
                    // example: ADD: (x1 = x2 + x3)
                    regs[rd] = regs[rs1] + regs[rs2];
                    cout << "EXEC: ADD x" << dec << rd << ", x" << rs1 << ", x" << rs2 << endl;
                }
                break;

            default:
                cout << "Unknown Opcode: 0x" << hex << opcode << endl;
                break;
        }

        // x0 is hardwired to 0. 
        // If code tried to write to x0, we force it back to 0.
        regs[0] = 0;

        // Move to next instruction
        pc += 4;
        return true;
    }

    // Load a program manually (for testing)
    void loadProgram(const vector<uint32_t>& prog) {
        for(size_t i=0; i<prog.size(); i++) {
            uint32_t word = prog[i];
            // Store 32-bit word as 4 bytes in memory
            memory[i*4] = word & 0xFF;
            memory[i*4+1] = (word >> 8) & 0xFF;
            memory[i*4+2] = (word >> 16) & 0xFF;
            memory[i*4+3] = (word >> 24) & 0xFF;
        }
    }
};

int main() {
    cout << "--- RISC-V SIMULATOR STARTING ---" << endl;
    
    CPU myCpu;

    // --- TEST PROGRAM ---
    // We are hand-assembling assembly into Hex code.
    // 1. ADDI x1, x0, 10  (Set x1 = 10) -> Hex: 0x00A00093
    // 2. ADDI x2, x0, 5   (Set x2 = 5)  -> Hex: 0x00500113
    // 3. ADD  x3, x1, x2  (Set x3 = x1 + x2 = 15) -> Hex: 0x002081B3
    // 4. 0x00000000 (Halt)
    vector<uint32_t> program = {
        0x00A00093, 
        0x00500113,
        0x002081B3,
        0x00000000 
    };

    myCpu.loadProgram(program);

    // Step through the program
    bool running = true;
    while(running) {
        char cmd;
        cout << "Press 's' to step, 'q' to quit: ";
        cin >> cmd;
        
        if (cmd == 's') {
            running = myCpu.executeNext();
            myCpu.printStatus();
        } else if (cmd == 'q') {
            break;
        }
    }

    return 0;
}
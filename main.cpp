#include <iostream>
#include "CPU.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: ./riscv_sim <elf_file>" << endl;
        return 1;
    }

    CPU cpu;
    string filename = argv[1];

    if (!cpu.loadELF(filename)) {
        return 1;
    }

    cout << "--- RISC-V SIMULATOR STARTING ---" << endl;
    
    // Main Execution Loop
    // In a real simulator, this might run until a HALT instruction or specific PC
    // For now, we cycle 1000 times or until the PC stops changing/hits 0
    int max_cycles = 1000;
    while(max_cycles > 0) {
        bool active = cpu.executeNext();
        if (!active) break; // HALT
        max_cycles--;
    }

    cout << "--- EXECUTION FINISHED ---" << endl;
    cpu.printStatus();

    return 0;
}
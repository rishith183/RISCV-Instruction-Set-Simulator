#include "CPU.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: ./riscv_sim <elf_file>" << endl;
        return 1;
    }

    CPU myCpu;

    cout << "--- RISC-V SIMULATOR STARTING ---" << endl;
    
    // Load the real binary file
    if (!myCpu.loadELF(argv[1])) {
        return 1;
    }

    bool running = true;
    while(running) {
        // Simple step execution for now
        // In the future, you can just loop executeNext() until false
        running = myCpu.executeNext();
        
        // Safety break if PC goes out of bounds or hits 0 (null) often
        if (myCpu.getPC() == 0) break; 
    }
    
    myCpu.printStatus();
    cout << "--- EXECUTION FINISHED ---" << endl;
    return 0;
}
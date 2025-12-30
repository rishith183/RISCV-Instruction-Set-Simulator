#include <iostream>
#include <string>
#include <vector>
#include "CPU.h"

using namespace std;

void printUsage() {
    cout << "Usage: ./riscv_sim <elf_file> [-d]" << endl;
    cout << "  -d : Enable Interactive Debug Mode (Step-by-step)" << endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    string filename = argv[1];
    bool debugMode = false;

    if (argc >= 3) {
        string flag = argv[2];
        if (flag == "-d") debugMode = true;
    }

    CPU cpu;
    if (!cpu.loadELF(filename)) {
        return 1;
    }

    cout << "--- RISC-V SIMULATOR STARTING ---" << endl;
    if (debugMode) {
        cout << "[DEBUG MODE ENABLED] Press ENTER to step. Type 'q' to quit." << endl;
    }
    
    // Run until Exit Syscall (returns false) or safety limit
    int max_cycles = 10000;
    while(max_cycles > 0) {
        if (debugMode) {
            cout << "\n>>> Press ENTER to step...";
            string input;
            getline(cin, input);
            if (input == "q") break;
        }

        bool active = cpu.executeNext();
        
        if (debugMode) cpu.printStatus();

        if (!active) break; // Stop on Exit Syscall
        max_cycles--;
    }

    cout << "--- EXECUTION FINISHED ---" << endl;
    if (!debugMode) cpu.printStatus();

    return 0;
}
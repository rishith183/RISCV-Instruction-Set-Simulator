#include "CPU.h"

int main() {
    cout << "--- RISC-V SIMULATOR STARTING ---" << endl;
    
    CPU myCpu;

    // Test Program: Countdown Loop
    vector<uint32_t> program = {
        0x00300093, // ADDI x1, x0, 3
        0x00100113, // ADDI x2, x0, 1
        0x402080B3, // SUB  x1, x1, x2
        0xFE009EE3, // BNE  x1, x0, -4
        0x00000000  // HALT
    };

    myCpu.loadProgram(program);

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
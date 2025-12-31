CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -I.

all: riscv_sim

riscv_sim: main.cpp CPU.cpp
	$(CC) $(CFLAGS) main.cpp CPU.cpp -o riscv_sim

test: test_runner.cpp CPU.cpp
	$(CC) $(CFLAGS) test_runner.cpp CPU.cpp -o run_tests
	./run_tests

clean:
	rm -f riscv_sim run_tests
import struct

def create_elf():
    # 1. RISC-V Machine Code (Countdown Loop)
    code = b'\x93\x00\x30\x00\x13\x01\x10\x00\xb3\x80\x20\x40\xe3\x9e\x00\xfe\x00\x00\x00\x00'

    # 2. ELF Header
    e_ident = b'\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    e_type_machine = struct.pack('<HH I', 2, 243, 1)
    
    # CHANGE 1: Set Entry Point to 0x00000000
    e_entry = 0x00000000 
    
    e_rest = struct.pack('<IIII HHHHHH', e_entry, 52, 0, 0, 52, 32, 1, 40, 0, 0)
    elf_header = e_ident + e_type_machine + e_rest

    # 3. Program Header
    code_offset = 52 + 32
    filesz = len(code)
    memsz = len(code)
    
    # CHANGE 2: Set Virtual Address (VAddr) and Physical Address (PAddr) to 0x00000000
    prog_header = struct.pack('<IIIIIIII', 1, code_offset, 0x00000000, 0x00000000, filesz, memsz, 5, 4)

    # 4. Write to file
    with open('test.elf', 'wb') as f:
        f.write(elf_header)
        f.write(prog_header)
        f.write(code)
    
    print(f"Successfully created 'test.elf' ({len(elf_header) + len(prog_header) + len(code)} bytes)")

if __name__ == "__main__":
    create_elf()
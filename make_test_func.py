import struct

def create_func_test():
    # Program:
    # 0x00: JAL x1, 28     (Call function at offset 28 -> PC 0x1C... wait, 0x1C? No, 0x20 is safer)
    # ...
    # 0x20: ADDI x10, x0, 5 (Set return value 5)
    # 0x24: JALR x0, x1, 0  (Return to caller)
    
    # We will put the function at offset 0x20
    # The JAL offset is relative. From 0x00 to 0x20 is +32 bytes.
    
    # 1. JAL x1, 32 (Jump to 0x20, save return addr in x1)
    # Opcode 0x6F. Imm=32.
    # 32 in binary is ...00100000. 
    # This is tricky to hand-assemble. 
    # Let's use simpler: 
    #   0x00: ADDI x10, x0, 10
    #   0x04: JAL x1, 12      (Jump to PC+12 -> 0x10)
    #   0x08: ADDI x10, x10, 1 (We should SKIP this if Jump works)
    #   0x0C: HALT
    #   0x10: ADDI x10, x10, 5 (Function: Add 5)
    #   0x14: JALR x0, x1, 0   (Return to 0x08)
    
    code = b''
    code += b'\x13\x05\xa0\x00' # 0x00: ADDI x10, x0, 10 (x10 = 10)
    code += b'\xef\x00\xc0\x00' # 0x04: JAL x1, 12 (Jump to 0x10)
    code += b'\x13\x05\x15\x00' # 0x08: ADDI x10, x10, 1 (Should execute AFTER return)
    code += b'\x00\x00\x00\x00' # 0x0C: HALT
    code += b'\x13\x05\x55\x00' # 0x10: ADDI x10, x10, 5 (x10 += 5)
    code += b'\x67\x80\x00\x00' # 0x14: JALR x0, x1, 0 (Return)
    
    # ELF Header logic (Same as before)
    e_ident = b'\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    e_type_machine = struct.pack('<HH I', 2, 243, 1)
    e_entry = 0x00000000 
    e_rest = struct.pack('<IIII HHHHHH', e_entry, 52, 0, 0, 52, 32, 1, 40, 0, 0)
    elf_header = e_ident + e_type_machine + e_rest

    code_offset = 52 + 32
    filesz = len(code)
    memsz = len(code)
    prog_header = struct.pack('<IIIIIIII', 1, code_offset, 0x00000000, 0x00000000, filesz, memsz, 5, 4)

    with open('test_func.elf', 'wb') as f:
        f.write(elf_header)
        f.write(prog_header)
        f.write(code)
        
    print("Created test_func.elf")

if __name__ == "__main__":
    create_func_test()
import struct

def create_final_test():
    # Goal: Calculate Sum of 1 to 5 (5+4+3+2+1 = 15)
    # Register Map:
    # x10 = n (Starts at 5)
    # x11 = sum (Starts at 0)
    
    code = b''
    # 0x00: ADDI x10, x0, 5    (n = 5)
    code += b'\x93\x05\x50\x00' 
    
    # 0x04: ADDI x11, x0, 0    (sum = 0)
    code += b'\x93\x05\x00\x00' 
    
    # --- Loop Start (Address 0x08) ---
    
    # 0x08: ADD x11, x11, x10  (sum = sum + n)
    # FIXED: Was targeting x10 (0x33), changed to target x11 (0xB3)
    code += b'\xb3\x85\xa5\x00' 
    
    # 0x0C: ADDI x10, x10, -1  (n = n - 1)
    code += b'\x13\x05\xf5\xff' 
    
    # 0x10: BNE x10, x0, -8    (If n != 0, jump back to 0x08)
    code += b'\xe3\x1c\x05\xff' 
    
    # 0x14: HALT
    code += b'\x00\x00\x00\x00' 

    # --- ELF Headers ---
    e_ident = b'\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    e_type_machine = struct.pack('<HH I', 2, 243, 1)
    e_entry = 0x00000000 
    e_rest = struct.pack('<IIII HHHHHH', e_entry, 52, 0, 0, 52, 32, 1, 40, 0, 0)
    elf_header = e_ident + e_type_machine + e_rest

    code_offset = 52 + 32
    filesz = len(code)
    memsz = len(code)
    prog_header = struct.pack('<IIIIIIII', 1, code_offset, 0x00000000, 0x00000000, filesz, memsz, 5, 4)

    with open('final_test.elf', 'wb') as f:
        f.write(elf_header)
        f.write(prog_header)
        f.write(code)
        
    print("Created final_test.elf")

if __name__ == "__main__":
    create_final_test()
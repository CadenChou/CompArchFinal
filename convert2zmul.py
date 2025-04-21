# import sys

# try:
#     filename = sys.argv[1]
# except IndexError:
#     print("usage: python3 convert2zmul.py </path/to/binaryfile>")
#     exit(-1)

# f = open(filename, 'rb')
# file_bytes = f.read()
# f.close()

# # TODO: manipulate file bytes here!


# f = open(filename, 'wb')
# f.write(file_bytes)
# f.close()


import sys

try:
    filename = sys.argv[1]
except IndexError:
    print("usage: python3 convert2zmul.py </path/to/binaryfile>")
    exit(-1)

f = open(filename, 'rb')
file_bytes = bytearray(f.read())
f.close()

# The mulw instruction we want to modify is at offset 0x10624
# The instruction bytes are: 02f707bb
MUL_OFFSET = 0x10624 - 0x10000  # Adjust for binary load address at 0x10000

# Modify the specific mulw instruction
i = MUL_OFFSET
if i + 4 <= len(file_bytes):
    # Note: for little endian, LSB is read first, so we need to read in "reverse order"
    inst = (file_bytes[i+3] << 24) | (file_bytes[i+2] << 16) | (file_bytes[i+1] << 8) | file_bytes[i]
    
    # Verify this is our mulw instruction (02f707bb)
    if inst == 0x02f707bb:
        # Convert to zmulw by changing funct3 to 0x1
        # Original: 02f707bb (mulw)
        # Modified: 02f717bb (zmul) - only changing funct3 field
        funct3_pos = 12 # funct3 field is bits 12 to 14
        funct3_mask = 0b111 << funct3_pos
        clear_funct3_mask = ~funct3_mask # Use this mask to clear funct3
        cleared_inst = inst & clear_funct3_mask # clear funct3
        new_funct3 = 0x1 << funct3_pos # Move new funct3 val (0x1) to correct position
        new_inst = cleared_inst | new_funct3 

        
        # Write back modified instruction
        # & with 0xFF to isolate single bytes
        file_bytes[i] = new_inst & 0xFF # LSB
        file_bytes[i+1] = (new_inst >> 8) & 0xFF
        file_bytes[i+2] = (new_inst >> 16) & 0xFF
        file_bytes[i+3] = (new_inst >> 24) & 0xFF # MSB
    else:
        print("Warning: Expected mulw instruction not found at offset")

f = open(filename, 'wb')
f.write(file_bytes)
f.close()
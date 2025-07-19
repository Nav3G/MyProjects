# Utilities 

def text_to_bits(s: str):
    bits: list[int] = []
    for c in s:
        byte = ord(c)
        for bit_pos in range(7, -1, -1):
            bits.append((byte >> bit_pos) & 1)
    
    return bits
        
def bits_to_text(bits: list[int]):
    if len(bits) % 8 != 0:
        raise ValueError("Bit list length must be multiple of 8")
    
    word: list[str] = []
    for i in range(0, len(bits), 8):
        byte = bits[i: i + 8]
        val = 0
        for bit in byte:
            val = (val << 1) | bit
        word.append(chr(val))

    return "".join(word)

def byte_to_bits(byte: int):
    if byte < 0 or byte > 255:
        raise ValueError("Invalid byte")
    
    bits: list[int] = []
    while byte > 0:
        bits.insert(0, byte % 2)    # Equivalently, byte & 1
        byte //= 2                  # Equivalently, bit shift by one: byte >>=1
    
    return [0] * (8 - len(bits)) + bits

def bits_to_byte(bits: list[int]):
    if len(bits) != 8:
        bits = [0] * (8 - len(bits)) + bits
    
    byte = 0
    for bit in bits:
        byte = (byte << 1) | bit
    
    return byte

def bits_to_bytes_list(bits: list[int]):
    if len(bits) % 8 != 0:
        raise ValueError("Bit list length must be multiple of 8")
    
    return [bits_to_byte(bits[i:i+8]) for i in range(0, len(bits), 8)]
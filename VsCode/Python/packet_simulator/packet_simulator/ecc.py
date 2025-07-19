from utility import *

def hamming74_encode(nibble: list[int]):
    if len(nibble) != 4:
        raise ValueError("Hamming payload can only be 4 bits long.")
    
    # Data bits
    d3, d2, d1, d0 = nibble[0], nibble[1], nibble[2], nibble[3]

    # Parity 
    p1 = d3 ^ d2 ^ d0
    p2 = d3 ^ d1 ^ d0
    p4 = d2 ^ d1 ^ d0

    # Codeword
    return [p1, p2, d3, p4, d2, d1, d0]

def hamming74_decode(nibble: list[int]):
    if len(nibble) != 7:
        raise ValueError("Hamming codeword can only be 7 bits long.")
    
    # Code bits
    p1, p2, d3, p4, d2, d1, d0 = nibble[0], nibble[1], nibble[2], nibble[3], nibble[4], nibble[5], nibble[6]

    # Parity check
    s1 = p1 ^ d3 ^ d2 ^ d0
    s2 = p2 ^ d3 ^ d1 ^ d0
    s4 = p4 ^ d2 ^ d1 ^ d0

    # Error correction
    syndrome = s1 | (s2<<1) | (s4<<2)
    corrected = None
    if syndrome != 0:
        nibble[syndrome - 1] ^= 1
        corrected = True
    else:
        corrected = False

    return [nibble[2], nibble[4], nibble[5], nibble[6]], corrected

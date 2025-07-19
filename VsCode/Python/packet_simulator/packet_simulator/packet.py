from utility import *
from ecc import *

PREAMBLE_BITS = byte_to_bits(0x7E)

def build_packet(payload_bits: list[int]):
    """
    Take raw data bits (multiples of 8) and produce a framed, ECC-protected packet:
    [Preamble(8)][Length(8)][ECC-encoded payload (n bytes)]
    """
     
    if len(payload_bits) % 4 != 0:
        pad = (4 - (len(payload_bits) % 4))
        payload_bits += [0] * pad
    
    # Split into 4-bit chunks and hamming encode
    ecc_bits = []
    for i in range(0, len(payload_bits), 4):
        nibble = payload_bits[i : i + 4]
        codeword = hamming74_encode(nibble)
        ecc_bits.extend(codeword)

    # Pad the bit stream
    if len(ecc_bits) % 8 != 0:
        pad = (8 - (len(ecc_bits) % 8))
        ecc_bits += [0] * pad

    # Compute length field
    length_in_bytes = len(ecc_bits) // 8

    # Build the final packet
    packet = []
    packet.extend(PREAMBLE_BITS)
    packet.extend(byte_to_bits(length_in_bytes))
    packet.extend(ecc_bits)

    return packet

def parse_packet(bit_stream: list[int]):
    """
    Take the frames, ECC-protected packet and extract and correct the original message
    [Preamble(8)][Length(8)][ECC-encoded payload (n bytes)]
    """
     
    # Sync to preamble
    stop = len(PREAMBLE_BITS)
    if bit_stream[:stop] == PREAMBLE_BITS:
        buf = bit_stream[stop:]
    else:
        raise ValueError("Invalid Packet!")
    
    # Read length field
    if len(buf) < 8:
        raise ValueError("Incomplete packet: no length field")
    length_in_bytes = bits_to_byte(buf[:8])
    buf = buf[8:]

    # Extract EEC payload
    payload_len_bits = length_in_bytes * 8
    if len(buf) < payload_len_bits:
        raise ValueError("Invomplete packet: payload too short")
    ecc_bits = buf[:payload_len_bits]

    pad_bits = len(ecc_bits) % 7
    if pad_bits:
        ecc_bits = ecc_bits[:-pad_bits]
    
    # Hamming decode
    data_bits = []
    crct_ct = 0
    for i in range(0, len(ecc_bits), 7):
        code = ecc_bits[i:i + 7]
        nibble, corrected = hamming74_decode(code)
        data_bits.extend(nibble)
        if corrected:
            crct_ct += 1
    
    # Remove tail padding to multiple of 8
    tail_length = len(data_bits) % 8
    if tail_length > 0:
        data_bits = data_bits[:-tail_length]

    return bits_to_text(data_bits)
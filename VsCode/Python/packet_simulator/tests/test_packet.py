from packet_simulator.utility import *
from packet_simulator.packet import * 
import random

def test_roundtrip_clean():
    payload = "Hello"
    bits = text_to_bits(payload)
    pkt = build_packet(bits)
    recovered = parse_packet(pkt)
    assert recovered == payload

def test_bit_flip():
    payload = "Hello"
    bits = text_to_bits(payload)
    pkt = build_packet(bits)
    pkt[random.randint(16, len(pkt) - 1)] ^= 1
    recovered = parse_packet(pkt)
    assert recovered == payload

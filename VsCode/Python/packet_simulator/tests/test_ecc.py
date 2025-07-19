from packet_simulator.ecc import hamming74_encode, hamming74_decode
import itertools
import pytest

def all_nibbles():
    """Generate all 16 possible 4-bit lists."""
    return [list(bits) for bits in itertools.product([0, 1], repeat=4)]

@pytest.mark.parametrize("nibble", all_nibbles())
def test_decode_no_error(nibble):
    """
    Encoding then immediately decoding with no bit flips
    should return the same nibble, and corrected==False.
    """
    codeword = hamming74_encode(nibble)
    decoded, corrected = hamming74_decode(codeword)
    assert decoded == nibble
    assert corrected is False

@pytest.mark.parametrize("nibble", all_nibbles())
def test_decode_single_bit_errors(nibble):
    """
    For each nibble, flip each of the 7 bits in its codeword
    and verify the decoder recovers the original nibble
    with corrected==True.
    """
    codeword = hamming74_encode(nibble)
    for bit_pos in range(7):
        corrupted = codeword.copy()
        corrupted[bit_pos] ^= 1
        decoded, corrected = hamming74_decode(corrupted)
        assert decoded == nibble, f"Failed to correct nibble {nibble} at pos {bit_pos}"
        assert corrected is True

def test_invalid_input_lengths():
    """
    Passing the wrong-sized lists should raise an error.
    """
    with pytest.raises(ValueError):
        hamming74_encode([0,1,0])      # too short
    with pytest.raises(ValueError):
        hamming74_decode([1,0,1,0,1,0]) # not 7 bits


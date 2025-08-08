import numpy as np

# =============================================================================
# Naive DFT
# =============================================================================
def naive_dft(x, nfft):
    """
    Direct O(N^2) implementation of the DFT.
    """
    x = x.astype(complex)
    N0 = len(x)

    nfft = N0 if nfft is None else nfft
    pad_amt = nfft - N0

    # Pad or truncate the signal
    if nfft < N0:
        x = x[:nfft]
    elif nfft > N0:
        x = np.concatenate([x, np.zeros(pad_amt, dtype=complex)])
        
    #  Precomputed twiddle
    twiddle = np.exp(-2j * np.pi / nfft)

    # DFT
    X = np.zeros(nfft, dtype=complex)
    for k in range(nfft):
        acc = 0+0j
        for n in range(nfft):
            acc += x[n] * twiddle**(k*n)
        X[k] = acc
    return X

# =============================================================================
# Recursive Cooley–Tukey FFT 
# =============================================================================
def naive_CT(x, nfft=None):
    """
    Recursive radix-2 FFT (divide & conquer) with O(N log N) complexity.
    """
    x = x.astype(complex)
    N0 = len(x)

    nfft = N0 if nfft is None else nfft
    nfft = 1 << (nfft-1).bit_length()    # next pow2 >= nfft
    pad_amt = nfft - N0

   # Pad or truncate the signal
    if nfft < N0:
        X = x[:nfft]
    elif nfft > N0:
        X = np.concatenate([x, np.zeros(pad_amt, dtype=complex)])
    else:
        X = x
    
    # Even-Odd recursive DFTs
    tw = np.exp(-2j * np.pi * np.arange(nfft) / nfft)
    # Base Case
    if nfft == 1:
        return np.array(X, dtype=complex)
    # Butterfly stages
    even = naive_CT(X[::2])
    odd  = naive_CT(X[1::2])
    X = np.zeros(nfft, dtype=complex)
    for k in range(nfft//2):
        X[k]       = even[k] + tw[k] * odd[k]
        X[k+nfft//2]  = even[k] - tw[k] * odd[k]
    return X
    
# =============================================================================
# In-place Radix-2 FFT
# =============================================================================
def bit_reverse(n, m):
    r = 0
    for _ in range(m):
        r = (r << 1) | (n & 1)
        n >>= 1
    return r

def radix2_fft(x, nfft=None):
    """
    In-place radix-2 decimation-in-time FFT. x is modified and returned.
    """
    x = x.astype(complex)
    N0 = len(x)

    nfft = N0 if nfft is None else nfft
    nfft = 1 << (nfft-1).bit_length()    # next pow2 >= N0
    pad_amt = nfft - N0

   # Pad or truncate the signal
    if nfft < N0:
        X = x[:nfft]
    elif nfft > N0:
        X = np.concatenate([x, np.zeros(pad_amt, dtype=complex)])
    else:
        X = x
    
    m = int(np.log2(nfft))     # Max bit length
    # Bit-reversal permutation
    for n in range(nfft):
        r = bit_reverse(n, m)
        if r > n:
            X[n], X[r] = X[r], X[n]
    # Butterfly stages
    half = 1
    for stage in range(m):
        L = half * 2
        W = np.exp(-2j * np.pi * np.arange(half) / L)
        for start in range(0, nfft, L):
            for k in range(half):
                a = X[start + k]
                b = X[start + k + half] * W[k]
                X[start + k]        = a + b
                X[start + k + half] = a - b
        half = L
    return X
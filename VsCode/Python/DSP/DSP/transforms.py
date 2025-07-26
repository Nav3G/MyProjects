import numpy as np

# =============================================================================
# Naive DFT
# =============================================================================
def naive_dft(x):
    """
    Direct O(N^2) implementation of DFT.
    """

    # Determing if signal is a power of 2 long
    L = len(x)
    if L & (L - 1) != 0:
        N = 2**np.ceil(np.log2(L))
    else:
        N = L
    # Pad signal up to a power of 2 with zeros
    x = np.concatenate([x, np.zeros(N - L)])
    X = np.zeros(N, dtype=complex)
    # DFT
    for k in range(N):
        acc = 0+0j
        for n in range(N):
            acc += x[n] * np.exp(-2j * np.pi * k * n / N)
        X[k] = acc
    return X

# =============================================================================
# Recursive Cooley–Tukey FFT 
# =============================================================================
def naive_CT(x):
    """
    Recursive radix-2 FFT (divide & conquer) with O(N log N) complexity.
    """
    L = len(x)
    N = 1 << (L-1).bit_length()

    # 0-padding to power of 2 length
    X = np.concatenate([x, np.zeros(N - L)])
    # Even-Odd recursive ffts
    if N == 1:
        return np.array(X, dtype=complex)
    even = naive_CT(X[::2])
    odd  = naive_CT(X[1::2])
    X = np.zeros(N, dtype=complex)
    for k in range(N//2):
        tw = np.exp(-2j * np.pi * k / N) * odd[k]
        X[k]       = even[k] + tw
        X[k+N//2]  = even[k] - tw
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

def radix2_fft(x):
    """
    In-place radix-2 decimation-in-time FFT. x is modified and returned.
    """
    N0 = len(x)
    N = 1 << (N0-1).bit_length()    # next pow2 >= N0
    pad_amt = N - N0

    # Pad out the data
    X = np.concatenate([ x.astype(complex), np.zeros(pad_amt, dtype=complex) ])
    assert N and (N & (N-1)) == 0, "Length must be a power of 2"
    m = int(np.log2(N))
    # Bit-reversal permutation
    for n in range(N):
        r = bit_reverse(n, m)
        if r > n:
            X[n], X[r] = X[r], X[n]
    # Butterfly stages
    half = 1
    for stage in range(m):
        L = half * 2
        W = np.exp(-2j * np.pi * np.arange(half) / L)
        for start in range(0, N, L):
            for k in range(half):
                a = X[start + k]
                b = X[start + k + half] * W[k]
                X[start + k]       = a + b
                X[start + k + half] = a - b
        half = L
    return X
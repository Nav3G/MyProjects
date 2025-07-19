import numpy as np
import matplotlib.pyplot as plt

# =============================================================================
# Domain Utilities
# =============================================================================
def domains(fs, L):
    """
    Generate sample indices and corresponding frequency bins.
    """
    idxs = np.arange(L)
    t = idxs / fs               # L samples, T = 1/fs -> t[n] = nT = n/fs
    freqs = idxs * (fs / L)
    return idxs, freqs, t

# =============================================================================
# Signal Generation
# =============================================================================
def generate_signal(L, freqs, amps, noise_level, t):
    """
    Create a synthetic signal: two sinusoids plus white Gaussian noise.
    """
    noise = np.random.randn(L) * noise_level
    signal = 0
    for i, freq in enumerate(freqs):
        signal += amps[i] * np.sin(2 * np.pi * freq * t)
    return signal + noise

# =============================================================================
# Windowing Functions
# =============================================================================
def make_window(L, window_type='rectangular'):
    """
    Return a window array of length N:
    - 'rectangular': all ones
    - 'hann'
    - 'hamming'
    """
    n = np.arange(L)
    if window_type == 'hann':
        return 0.5 * (1 - np.cos(2 * np.pi * n / (L - 1)))
    elif window_type == 'hamming':
        return 0.54 - 0.46 * np.cos(2 * np.pi * n / (L - 1))
    else:
        return np.ones(L)

# Various DFT techniques
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
    for f in range(N):
        acc = 0+0j
        for n in range(N):
            acc += x[n] * np.exp(-2j * np.pi * f * n / N)
        X[f] = acc
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
    # test
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

    # 2) pad out the data
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

# =============================================================================
# Generic Spectrum Generation
# =============================================================================
def spectrum(x, fs, method='fft'):
    # 1) compute your FFT
    if method=='fft':
        X = radix2_fft(x.astype(complex).copy())
    elif method=='CT':
        X = naive_CT(x)
    else:
        X = naive_dft(x)
    # 2) find the actual FFT length and the “half‑spectrum” length
    N_fft = len(X)
    half  = N_fft//2
    # 3) build the matching frequency axis
    freqs = np.arange(N_fft) * (fs / N_fft)     # freqs[n] = n*fs/Nfft
    # 4) compute magnitude in dB
    mags_db = 20*np.log10(np.abs(X[:half]) + 1e-12)
    # return just the positive‑freq half
    return freqs[:half], mags_db, N_fft

# =============================================================================
# Plotting Helpers
# =============================================================================
def plot_time(t, signal, start=0, end=None, title=None):
    """Plot time-domain signal in given index range."""
    end = end or len(signal)
    plt.figure(figsize=(8, 3))
    plt.plot(t[start:end], signal[start:end])
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title(title or f'Time Domain (n={start}:{end})')
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def plot_2time(t, s1, s2, start=0, end=None, title=None):
    """PLot 2 time-domain signals in a given index range."""
    if start:
        st1 = start[0]
        st2 = start[1]
    else:
        st1 = 0
        st2 = 0
    if end:
        e1 = end[0]
        e2 = end[1]
    else:
        e1 = len(s1)
        e2 = len(s2)
        
    plt.figure(figsize=(8,3))
    plt.plot(t[st1:e1], s1[st1:e1])
    plt.plot(t[st2:e2], s2[st2:e2])
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title(title or f'Time Domain (n={st1}:{e1})')
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def plot_2spectrum(x1, x2, fs, method='fft', labels=None, ax=None):
    """
    Compute and plot magnitude spectrum of signal x.
    method: 'naive' for naive_dft, 'fft' for radix2_fft
    window_type: as in make_window
    """
    # Transform
    f1, M1, N_fft1 = spectrum(x1, fs, method)
    f2, M2, N_fft2 = spectrum(x2, fs, method)

    assert len(f1)==len(M1) and len(f2)==len(M2)

    if ax is None:
        plt.figure(figsize=(8,4))
        ax = plt.gca()

    ax.plot(f1, M1, label=labels[0])
    ax.plot(f2, M2, label=labels[1])
    ax.set_xlabel('Frequency (Hz)')
    ax.set_ylabel('Magnitude (dB)')
    ax.set_xlim(0, fs/2)
    ax.grid(True)
    ax.legend()
    return ax

def plot_spectrum(x1, fs, method='fft', window_type='rectangular', label=None, ax=None):
    """
    Compute and plot magnitude spectrum of signal x.
    method: 'naive' for naive_dft, 'fft' for radix2_fft
    window_type: as in make_window
    """
    # Apply window

    # Transform
    f1, M1 = spectrum(x1, fs, method)

    assert len(f1)==len(M1)

    if ax is None:
        plt.figure(figsize=(8,4))
        ax = plt.gca()

    ax.plot(f1, M1, label=label)
    ax.set_xlabel('Frequency (Hz)')
    ax.set_ylabel('Magnitude (dB)')
    ax.set_xlim(0, fs/2)
    ax.grid(True)
    ax.legend()
    return ax
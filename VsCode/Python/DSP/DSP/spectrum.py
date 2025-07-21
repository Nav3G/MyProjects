import numpy as np

from DSP.transforms import *

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
    return freqs[:half], mags_db
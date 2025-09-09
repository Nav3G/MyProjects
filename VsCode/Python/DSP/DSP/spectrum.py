from DSP.transforms import *
from DSP.windows import *

import numpy as np
from scipy import signal

# =============================================================================
# Generic Spectrum Generation
# =============================================================================
def spectrum(x, fs, nfft=512, method='fft', window=None):
    """
    Compute a one-sided magnitude spectrum (dB) of a real-valued signal.

    Args:
        x: 1D time-domain signal.
        fs: Sampling frequency in Hz.
        method: Transform to use (default 'fft'):
            - 'fft': radix-2 Cooley-Tukey FFT.
            - 'CT': custom Cooley-Tukey FFT.
            - otherwise: naive O(N^2) DFT.
        window: If provided, applies this window before the transform:
            - 'rectangular', 'hann', or 'hamming'.
            - None means no windowing (default).

    Returns:
        freqs: Frequencies for the positive-half spectrum (0 <= f <= fs/2).
        mags_db: Magnitude in dB, 20*log10|X(f)|, same length as 'freqs'.
    """

    # Apply window
    w = make_plotting_window(len(x), window)
    x_win = x * w
    # Compute FFT
    if method=='fft':
        X = radix2_fft(x_win.astype(complex).copy(), nfft)
    elif method=='CT':
        X = naive_CT(x_win)
    elif method=='scipy_fft':
        X = np.fft.fft(x, nfft)
    else:
        X = naive_dft(x_win)
    # Find the “half‑spectrum” length (nyquist)
    half = nfft//2
    # Build the matching frequency axis
    freqs = np.arange(nfft) * (fs / nfft)     # freqs[n] = n*fs/nfft
    # Compute magnitude in dB
    mags_db = 20*np.log10(np.abs(X[:half]) + 1e-12)
    # Return just the positive‑freq half
    return freqs[:half], mags_db
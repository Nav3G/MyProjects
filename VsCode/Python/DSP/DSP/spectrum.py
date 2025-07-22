from DSP.transforms import *

import numpy as np

# =============================================================================
# Windowing functions for spectrum plotting
# =============================================================================
def make_plotting_window(L, window_type='rectangular'):
    """
    Generate a time-domain window for spectrum plotting.

    Args:
        L: int
            Length of the window to generate.
        window_type: str, optional
            Type of window to return (default 'rectangular'):
            - 'rectangular': all ones (no taper).
            - 'hann': Hann window, 0.5*(1-cos(2πn/(L-1))).
            - 'hamming': Hamming window, 0.54-0.46*cos(2πn/(L-1)).

    Returns: 
        w: Window coefficients.
   """   
    
    n = np.arange(L)
    if window_type == 'hann':
        return 0.5 * (1 - np.cos(2 * np.pi * n / (L - 1)))
    elif window_type == 'hamming':
        return 0.54 - 0.46 * np.cos(2 * np.pi * n / (L - 1))
    else:
        return np.ones(L)


# =============================================================================
# Generic Spectrum Generation
# =============================================================================
def spectrum(x, fs, method='fft', window=None):
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
        X = radix2_fft(x_win.astype(complex).copy())
    elif method=='CT':
        X = naive_CT(x_win)
    else:
        X = naive_dft(x_win)
    # Find the actual FFT length and the “half‑spectrum” length
    N_fft = len(X)
    half  = N_fft//2
    # Build the matching frequency axis
    freqs = np.arange(N_fft) * (fs / N_fft)     # freqs[n] = n*fs/Nfft
    # Compute magnitude in dB
    mags_db = 20*np.log10(np.abs(X[:half]) + 1e-12)
    # Return just the positive‑freq half
    return freqs[:half], mags_db
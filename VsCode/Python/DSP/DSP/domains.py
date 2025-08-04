import numpy as np

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

def times(fs, L):
    """
    Generate time domain only.
    """
    return np.arange(L)/ fs

# =============================================================================
# Signal Generation
# =============================================================================
def generate_signal(L, freqs, amps, noise_level, t):
    """
    Create a synthetic signal: Sinusoids plus white Gaussian noise.
    """
    noise = np.random.randn(L) * noise_level
    signal = 0
    for i, freq in enumerate(freqs):
        signal += amps[i] * np.sin(2 * np.pi * freq * t)
    return signal + noise
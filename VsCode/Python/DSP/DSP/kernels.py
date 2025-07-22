import numpy as np

def sinc_lpf(f_pass, numtaps, fs):
    """
    Design an ideal lowpass filter via the windowed‐sinc method.

    Args:
        f_pass: float
            Passband edge frequency in Hz.
        numtaps: int
            Number of taps (should be odd for linear phase).
        fs: float
            Sampling frequency in Hz.

    Returns:
        h: ndarray, shape (numtaps,)
            Ideal sinc filter impulse response (not windowed yet).
    """

    M = int((numtaps - 1)/2)
    h_ideal = np.zeros(numtaps)
    w_c = 2*np.pi*(f_pass/fs)

    for n in range(numtaps):
        m = n - M
        if m != 0:
            h_ideal[n] = (np.sin(w_c * m))/(m * np.pi)
        else:
            h_ideal[int(M)] = (w_c)/np.pi
    h_ideal /= h_ideal.sum()
    
    return h_ideal


import numpy as np
import matplotlib.pyplot as plt

# =============================================================================
# Windowing functions for spectrum plotting
# =============================================================================
def make_plotting_window(N, window_type='rectangular'):
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
    
    if window_type == 'hann':
        return hann(N)
    elif window_type == 'hamming':
        return hamming(N)
    elif window_type == 'kaiser':
        return kaiser_window(N, compute_kaiser_beta(60))
    else:
        return np.ones(N)
    
# =============================================================================
# Raised Cosine windows
# =============================================================================
def raised_cosine(a_0, N):
    n = np.arange(N)
    return a_0-(1-a_0)*np.cos(2*np.pi*n / N)

# Hann (Hanning) window
def hann(N):
    return raised_cosine(0.5, N)

# Hamming window
def hamming(N):
    return raised_cosine(0.53838, N)

# =============================================================================
# Blackman window
# =============================================================================
def blackman(N, alpha=0.16):
    n = np.arange(N)

    a_0 = (1-alpha) / 2
    a_1 = 0.5
    a_2 = alpha / 2

    return a_0-a_1*np.cos(2*np.pi*n/N)+a_2*np.cos(4*np.pi*n/N)

    
# =============================================================================
# Kaiser-Beta window
# =============================================================================
def compute_kaiser_beta(A_att):
    """
    Compute the Kaiser window beta parameter for the desired stop-band attenuation.
    A_att : float
        Desired stop-band attenuation in dB (e.g. 60).
    
    Returns
    -------
    beta : float
    """
    # For A_att > 50 dB: beta = 0.1102 * (A_att - 8.7)
    # Otherwise use the alternate formula.
    return 0.1102 * (A_att - 8.7)

def build_kaiser_bessel(vi, alpha, terms=30):
    """
    Compute the Kaiser-Bessel kernel values at normalized positions vi for shape parameter alpha.
    """
    vi = np.asarray(vi, float)
    d = alpha
    n = d * np.sqrt(1 - vi**2)

    accd = np.ones_like(n)
    accn = np.ones_like(n)
    fact = 1.0

    for i in range(1, terms):
        fact *= i
        coeff = 1.0 / (fact * fact)
        accd += (0.5 * d)**(2*i) * coeff
        accn += (0.5 * n)**(2*i) * coeff

    return accn / accd

def kaiser_window(N, beta, terms=15):
    """
    Generate an N-tap Kaiser window with shape parameter beta.
    """
    n = np.arange(N)
    # Map sample index to normalized position in [-1, +1]
    vi = 2*n/(N) - 1
    
    return build_kaiser_bessel(vi, beta, terms)

# =============================================================================
# Coherent gain computation
# =============================================================================
def coherent_gain(window):
    return np.sum(window) / len(window)
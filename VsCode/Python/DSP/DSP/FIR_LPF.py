from DSP.transforms import radix2_fft, naive_CT

import numpy as np
import matplotlib.pyplot as plt

# =============================================================================
# Response Kernel generation
# =============================================================================
def generate_response(w_c, M, N):
    h = np.zeros(N)

    for n in range(N):
        m = n - M
        if m != 0:
            h[n] = (np.sin(w_c * m))/(m * np.pi)
        else:
            h[M] = w_c/np.pi
    return h

# =============================================================================
# Building the window
# =============================================================================
def window(N):
    w = np.zeros(N)
    for n in range(N):
        w[n] = 0.54 - 0.46*(np.cos(2*np.pi*n/(N - 1)))
    
    return w

# =============================================================================
# Forming FIR coefficients
# =============================================================================
def FIR(h_ideal, w, N):
    h = np.zeros(N)
    for n in range(N):
        h[n] = h_ideal[n]*w[n]
    
    return h

# =============================================================================
# Convolution with filter response curve
# =============================================================================
def convolve(signal, h, L, N):
    y = np.zeros(L)
    for n in range(L):
        acc = 0
        for k in range(N):
            m = n - k
            if m > k:
                acc += h[k] * signal[m]
            else:
                continue
        y[n] = acc

    return y

# =============================================================================
# LPF
# =============================================================================
def LPF(signal, w_c, L, N=101, M=50):
    h_ideal = generate_response(w_c, M, N)
    w = window(N)
    h = FIR(h_ideal, w, N)
    y = convolve(signal, h, L, N)

    return y

# =============================================================================
# Plot dirichlet filter response
# =============================================================================
def plot_dirichlet_filter_response(h, fs, N_fft):
   # 0-padding the response to length N_fft
    L = len(h)
    h_padded = np.zeros(N_fft, dtype=complex)
    h_padded[:L] = h

    # full frequency axis 0…fs
    freqs = np.arange(N_fft) * (fs / N_fft)  #f[k] = k/N_fft * f_s

    # FFT of the padded impulse response
    H_full = naive_CT(h_padded)

    # keep only 0…fs/2
    half      = N_fft//2 + 1
    freqs_pos = freqs[:half]
    H_pos     = H_full[:half]

    # magnitude in dB
    H_db = 20 * np.log10(np.abs(H_pos))

    plt.plot(freqs_pos, H_db)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Magnitude [dB]')
    plt.title('Filter Response')
    plt.grid(True)

# Kaiser filter
# =============================================================================
# Kaiser Beta
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

def estimate_filter_length(A_att, fs, f_pass, f_stop):
    """
    Estimate the number of taps L for a Kaiser FIR.
    A_att  : float — stop-band attenuation (dB)
    fs     : float — sampling rate (Hz)
    f_pass : float — pass-band edge (Hz)
    f_stop : float — stop-band edge (Hz)
    Returns
    -------
    L : int
        Filter length (odd integer).
    """
    # Compute transition width df = f_stop - f_pass
    # Use Kaiser formula: L ≈ ((A_att-8)/(2.285*(2pi*delf/fs))) + 1
    # Round up to next odd integer.
    if f_stop <= f_pass:
        raise ValueError(f"Stopband edge ( {f_stop} Hz ) must exceed passband edge ( {f_pass} Hz )")
    df = f_stop - f_pass
    L = int(np.ceil(((A_att - 8)/(2.285*(2*np.pi*df / fs))) + 1))

    if L % 2 == 0:
        L += 1
    
    return L

def build_kaiser_bessel(vi, al, terms=30):
    """
    Compute the Kaiser-Bessel kernel values at normalized positions vi for shape parameter alpha.
    """
    vi = np.asarray(vi, float)
    d = al
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
    vi = 2*n/(N-1) - 1
    
    return build_kaiser_bessel(vi, beta, terms)

def design_kaiser_fir(fs, f_pass, f_stop, A_att):
    """
    Build the Kaiser-windowed low-pass filter taps.
    fs: sampling rate
    f_pass: pass band frequency
    f_stop: stop band frequency
    A_att: stop band attenuation
    
    Returns
    -------
    h : 1D np.ndarray
        FIR filter coefficients (length L).
    """
    # 1) Compute beta
    # 2) Compute L
    # 3) Compute ideal sinc impulse (centered at (L-1)/2)
    # 4) Compute Kaiser window
    # 5) Multiply to get h[n]

    beta = compute_kaiser_beta(A_att)
    L = estimate_filter_length(A_att, fs, f_pass, f_stop)
    M = int((L - 1)/2)

    h_ideal = np.zeros(L)
    w_c = 2*np.pi*(f_pass/fs)
    for n in range(L):
        m = n - M
        if m != 0:
            h_ideal[n] = (np.sin(w_c * m))/(m * np.pi)
        else:
            h_ideal[int(M)] = (w_c)/np.pi

    w = kaiser_window(L, beta)
    h = h_ideal * w
    h /= np.sum(h)  

    return h

def apply_zero_phase_filter(x, h):
    """
    Apply zero-phase FIR filtering via forward-backward.
    x : 1D np.ndarray
        Input signal to filter.
    h : 1D np.ndarray
        FIR taps.
    Returns
    -------
    y : 1D np.ndarray
        Filtered output, same length as x, zero-phase.
    """
    N_taps = len(h)
    L = len(x)
    pad_len = N_taps - 1
    
    # Signal relflect
    left_pad = x[1: N_taps][::-1]
    right_pad = x[-N_taps: -1][::-1]
    x_ext = np.concatenate([left_pad, x, right_pad])

    # Forward-convolve
    y_fwd = np.zeros(L)
    for n in range(L):
        acc = 0
        for k in range(N_taps):
                acc += h[k] * x_ext[n + k]
        y_fwd[n] = acc
    y_fwd = y_fwd[::-1]

    # Second reflection
    left_pad = y_fwd[1: N_taps][::-1]
    right_pad = y_fwd[-N_taps: -1][::-1]
    y_ext = np.concatenate([left_pad, y_fwd, right_pad])

    # Backward-convolve
    data_ext = np.zeros(L)
    for n in range(L):
        acc = 0
        for k in range(N_taps):
                acc += h[k] * y_ext[n + k]
        data_ext[n] = acc
    data_ext = data_ext[::-1]

    return data_ext[pad_len: -pad_len]
    
def plot_kaiser_filter_response(h, fs, N_fft):
    """
    Plot magnitude response of the FIR to verify specs.
    h  : FIR taps
    fs : sampling rate
    M  : Plotting points
    """
    # 0-padding the response to length N_fft
    L = len(h)
    h_padded = np.zeros(N_fft, dtype=complex)
    h_padded[:L] = h

    # full frequency axis 0…fs
    freqs = np.arange(N_fft) * (fs / N_fft)  #f[k] = k/N_fft * f_s

    # FFT of the padded impulse response
    H_full = radix2_fft(h_padded)

    # keep only 0…fs/2
    half      = N_fft//2 + 1
    freqs_pos = freqs[:half]
    H_pos     = H_full[:half]

    # magnitude in dB
    H_db = 20 * np.log10(np.abs(H_pos))

    plt.plot(freqs_pos, H_db)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Magnitude [dB]')
    plt.title('Filter Response')
    plt.grid(True)

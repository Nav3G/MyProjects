from DSP.spectrum import *


import warnings
import numpy as np
import matplotlib.pyplot as plt

def sinc_lpf(f_pass, numtaps, fs):
    """
    Design an ideal lowpass filter via the windowed-sinc method.

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

def sinc_hpf(f_cutoff, numtaps, fs):
    """
    Design an ideal highpass filter via an inversion of the sinc function.

    Args:
        f_pass: float
            Passband edge frequency in Hz.
        numtaps: int
            Number of taps (should be odd for linear phase).
        fs: float
            Sampling frequency in Hz.

    Returns:
        h: ndarray, shape (numtaps,)
            Ideal inverted sinc filter impulse response (not windowed yet).
    """

    M = int((numtaps - 1)/2)
    h_lp = sinc_lpf(f_cutoff, numtaps, fs)

    delta = np.zeros_like(h_lp)
    delta[M] = 1

    h_ideal = delta - h_lp

    return h_ideal

def sinc_bpf(f1, f2, numtaps, fs):
    """
    Design an ideal bandpass filter via the difference of two lpf sinc functions.

    Args:
        f1: float
            First passband edge frequency in Hz.
        f2: float
            Second passband edge frequency in Hz.
        numtaps: int
            Number of taps (should be odd for linear phase).
        fs: float
            Sampling frequency in Hz.

    Returns:
        h: ndarray, shape (numtaps,)
            Ideal difference sinc filter impulse response (not windowed yet).
    """

    h_lp1 = sinc_lpf(f1, numtaps, fs)
    h_lp2 = sinc_lpf(f2, numtaps, fs)
    h_ideal = h_lp2 - h_lp1

    return h_ideal

def sinc_brf(f1, f2, numtaps, fs):
    """
    Design an ideal bandreject filter via the inversion of the bandpass kernel.

    Args:
        f1: float
            First passband edge frequency in Hz.
        f2: float
            Second passband edge frequency in Hz.
        numtaps: int
            Number of taps (should be odd for linear phase).
        fs: float
            Sampling frequency in Hz.

    Returns:
        h: ndarray, shape (numtaps,)
            Ideal bandreject sinc filter impulse response (not windowed yet).
    """

    M = int((numtaps - 1)/2)
    h_bp = sinc_bpf(f1, f2, numtaps, fs)

    delta = np.zeros_like(h_bp)
    delta[M] = 1

    h_ideal = delta - h_bp

    return h_ideal

def remez_taps(filter_type, numtaps, fs, bands, desired, weights):
    MAX_ITERS = 50
    TOL = 1e-8
    converged = False

    if filter_type not in ['lowpass', 'highpass', 'bandpass', 'bandreject']:
        raise ValueError('Filter type not supported')
    if numtaps % 2 == 0:
        raise ValueError('Tap length must be odd')
    
    # Normalize frequency specs by fs so they lie in [0, 1] span
    nyq = fs / 2.0
    norm_bands = []
    for low, high in bands:
        norm_bands.append((low / nyq, high / nyq))

    # Linear-phase symmetry
    M = (numtaps - 1)//2

    # Frequency grid - Basically, discretizing the bands into grids
    K = 50
    G = max(2000, K * (M + 1))
    total_norm_bw = sum(fH - fL for fL, fH in norm_bands)
    points_per_band = [max(3, int(round(G * (fH - fL) / total_norm_bw))) 
                       for fL, fH in norm_bands]
    points_per_band[0] += G - sum(points_per_band)

    grid_freqs, grid_desired, grid_weights = [], [], []
    EDGE_MARGIN = 0.02
    # Flat array of the discretized bands
    for (fL, fH), di, wi, P in zip(norm_bands, desired, weights, points_per_band):
        bw = fH - fL
        core = np.linspace(fL, fH, P, endpoint=True)

        m = EDGE_MARGIN * bw
        n_edge = max(20, int(0.2 *P))
        left_skirt = np.linspace(max(0, fL - m), fL, n_edge, endpoint=True)
        right_skirt = np.linspace(fH, min(1, fH + m), n_edge, endpoint=True)

        all_pts = np.concatenate([left_skirt, core, right_skirt])
        grid_freqs.extend(all_pts)
        grid_desired.extend([di] * len(all_pts))
        grid_weights.extend([wi] * len(all_pts))

    # sort, dedup, covert
    grid_freqs = np.array(grid_freqs)
    grid_desired = np.array(grid_desired)
    grid_weights = np.array(grid_weights)

    order = np.argsort(grid_freqs)
    grid_freqs   = grid_freqs[order]
    grid_desired = grid_desired[order]
    grid_weights = grid_weights[order]

    uniq, ix = np.unique(np.round(grid_freqs, 8), return_index=True)
    grid_freqs   = grid_freqs[ix]
    grid_desired = grid_desired[ix]
    grid_weights = grid_weights[ix]

    # Initilize extremal frequencies 
    n_extrema = M + 2
    G = len(grid_freqs)
    delta_spacing = (G - 1)/(n_extrema - 1)
    ext_idx = [int(round(k * delta_spacing)) for k in range(n_extrema)]

    ext_freq = [grid_freqs[i] for i in ext_idx]
    ext_desired = [grid_desired[i] for i in ext_idx]
    ext_weights = [grid_weights[i] for i in ext_idx]

    # Ramez iteration
    prev_ext_idx = ext_idx.copy()
    prev_delta = None

    for j in range(1, MAX_ITERS + 1):
        A = np.zeros((M + 2, M + 2))
        D = np.zeros(n_extrema)
        eps = 1e-10

        for k in range(n_extrema):
            omega = ext_freq[k] * np.pi
            for m in range(M + 1):
                A[k, m] = np.cos(m*omega)
            A[k, -1] = ((-1)**k)/ext_weights[k]
            D[k] = ext_desired[k]
        A += np.eye(M+2)*eps
        x = np.linalg.solve(A, D)
        a = x[:M+1]                  # Coefs. of the Chebychev polynomial approx. of H
        d_ripple = x[M+1]
        delta = d_ripple

        # Response and error
        # We enforce that the weighted deviation of H from the desired output
        # must, in the discrete sense, alternate in sign by a swing of at most
        # 2*delta (a ripple amplitude of delta).
        H = np.zeros(G)
        for i, f in enumerate(grid_freqs):
            omega = f * np.pi
            H[i] = np.dot(a, np.cos(np.arange(M+1) * omega))
        E = np.array(grid_weights) * (np.array(grid_desired) - H)

        # Extrema of Error
        cand = []
        for i in range(1, len(E) - 1):
            if (E[i] < E[i + 1] and E[i] < E[i - 1]) or \
            (E[i] > E[i + 1] and E[i] > E[i - 1]):
                cand.append(i)
        cand_sorted = sorted(cand, key=lambda i: abs(E[i]), reverse=True)

        # Pick top M + 2 extrema whose signs strictly alternate. Stop at Err = M + 2
        new_ext_idx = []
        last_sign = None
        for i in cand_sorted:
            sign = np.sign(E[i])
            if last_sign is None or sign != last_sign:
                new_ext_idx.append(i)
                last_sign = sign
            if len(new_ext_idx) == M + 2:
                break

        if len(new_ext_idx) < n_extrema:
            new_ext_idx = prev_ext_idx.copy()
        new_ext_idx.sort()

        # Update extremal sets
        ext_idx = new_ext_idx
        ext_freq = [grid_freqs[i] for i in ext_idx]
        ext_desired = [grid_desired[i] for i in ext_idx]
        ext_weights = [grid_weights[i] for i in ext_idx]

        # Convergence check
        same_extrema = (prev_ext_idx is not None and prev_ext_idx == new_ext_idx)
        small_delta = (prev_delta is not None and abs(delta - prev_delta) < TOL)

        if same_extrema and small_delta:
            converged = True
            break

        prev_delta = delta
        prev_ext_idx = ext_idx.copy()

    if not converged:
        warnings.warn(
            f"Parks-McClellan did not converge in {MAX_ITERS} iterations; "
            f"last ripple change = {abs(delta - prev_delta):.2e}",
            UserWarning
        )
    else: 
        print("Iters for convergence: " + str(j))

    h = np.zeros(2*M+1)
    h[M] = a[0]
    for k in range(1, M+1):
        h[M+k] = h[M-k] = a[k]/2
        
    return h

#===============================
# PLotting 
#===============================
def plot_kernel(w, **plot_kwargs):

    plt.plot(np.arange(len(w)), w, **plot_kwargs)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title('Sinc Kernel')

def plot_filter_response(h, fs, **plot_kwargs):

    freq, H_db = spectrum(h, fs, 'fft', 'hann')
    plt.plot(freq, H_db, **plot_kwargs)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Magnitude [dB]')
    plt.title('Filter Response')
    plt.grid(True)

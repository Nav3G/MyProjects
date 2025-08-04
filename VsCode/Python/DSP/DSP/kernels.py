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

#================================================
# Parks-Maclelan Algorithm for adaptive filtering
#================================================
def normalize_bands(bands, fs):
    """
    Normalizes the chosen bands by the sampling rate so they span [0, 1]

    Args:
        bands: tuple of ints
            Band from start freqeuncy to end frequency (Hz).
        fs: int
            Sampling rate (Hz)

    Returns: 
        bands: array of tuples
            Normalized bands
    """
    nyq = fs / 2.0

    return [(fL / nyq, fH/ nyq) for fL, fH in bands]

def allocate_grid(norm_bands, G, min_pts=20):
    """
    Allocates a discrete grid of frequencies over which Chebyshev problem is solved.

    Args:
        norm_bands: tuple of ints
            Band from start freqeuncy to end frequency (Hz) scaled to [0, 1].
        G: int
            Total number of grid points across all bands.
        min_pts: int
            The minimum number of samples given to a single band.
        
    Returns: 
        bands: array of ints
            The number of points allocated to each band.
    """
    # Sum all the bandwidths up
    total_bw = sum(fH-fL for fL, fH in norm_bands)
    # Allocate points to the grid, at least min_pts. Wider bands get more points
    pts = [max(min_pts, int(round(G * (fH - fL) / total_bw))) for fL, fH in norm_bands]
    pts[0] += G - sum(pts)

    return pts

def make_grid(norm_bands, desired, weights, pts_per_band, edge_margin):
    freqs, des, wts = [], [], []
    for (fL, fH), d, wt, P in zip(norm_bands, desired, weights, pts_per_band):
        # Core points uniformly inside [fL, fH]
        core = np.linspace(fL, fH, P, endpoint=True)
        # Compute small skirt
        bw = fH - fL
        m = edge_margin*(bw)
        # Ensure a few skirt points
        n_edge = max(2, int(0.22 * P))
        # Left and right skirts
        left_skirt = np.linspace(max(0, fL - m), fL, n_edge, endpoint=True)
        right_skirt = np.linspace(fH, min(1, fH + m), n_edge, endpoint=True)
        # Concatenate
        band_freqs = np.concatenate([left_skirt, core, right_skirt])
        # Append
        freqs.extend(band_freqs)
        des.extend([d] * len(band_freqs))
        wts.extend([wt] *  len(band_freqs))
    # Remove duplicates (overlap)
    unique_freqs, indices = np.unique(freqs, return_index=True)
    freqs = np.array(freqs)[indices]    # Numpy slicing trick
    des = np.array(des)[indices]
    wts = np.array(wts)[indices]
    # Sort by freq
    order = np.argsort(freqs)

    return freqs[order], des[order], wts[order]

def solve_remez_system(ext_freqs, ext_desired, ext_weights, M, eps=1e-10):
    N_ext = len(ext_freqs)
    # Build the (m+2)x(M+2) system matrix A and RHS D
    A = np.zeros((N_ext, N_ext))
    D = np.zeros(N_ext)

    for k, omega in enumerate(ext_freqs):
        for m in range(M + 1):
            A[k, m] = np.cos(m*np.pi*omega)
        A[k, -1] = ((-1)**k) / ext_weights[k]
        D[k] = ext_desired[k]
    A += eps*np.eye(N_ext)
    A_reg = np.vstack([
        A,
        np.sqrt(eps)*np.eye(N_ext)
    ])
    D_reg = np.concatenate([D, np.zeros(N_ext)])
    x, *_ = np.linalg.lstsq(A_reg, D_reg, rcond=None)

    return x[:M+1], x[M+1]

def compute_error(a, grid_freqs, grid_desired, grid_weights):
    omega = grid_freqs * np.pi
    # (M+1)xG cosine-matrix
    cos_mat = np.cos(np.outer(np.arange(len(a)), omega))
    A = a @ cos_mat
    # Error
    E = grid_weights * (grid_desired - A)

    return E

def find_extrema(E, n_extrema):
    N = len(E)
    # Locate all “true” extrema inside the grid
    cand = [i for i in range(1, N-1)
            if (E[i]>E[i-1] and E[i]>E[i+1]) or
               (E[i]<E[i-1] and E[i]<E[i+1])]
    # Sort by descending |E[i]|
    cand.sort(key=lambda i: abs(E[i]), reverse=True)
    # Pick the strongest alternating extrema
    ext = [0]
    last_sign = np.sign(E[0])
    for i in cand:
        s = np.sign(E[i])
        if s != last_sign:
            ext.append(i)
            last_sign = s
        if len(ext) == n_extrema-1:
            break
    ext.append(N-1)
    # PADDING
    # First pad with with the remaining true peaks from the last pass
    if len(ext) < n_extrema:
        for i in cand:
            if i not in ext:
                ext.append(i)
            if len(ext) == n_extrema:
                break
    # Then fill uniformly until the end (should be very few points)
    if len(ext) < n_extrema:
        uniform = np.round(np.linspace(0, N-1, n_extrema)).astype(int)
        for i in uniform:
            if len(ext) == n_extrema:
                break
            if i not in ext:
                ext.append(i)
    
    return sorted(ext[:n_extrema])

def remez_taps(numtaps, fs, bands, desired, weights, 
               eps, grid_mul, edge_margin, max_iters, 
               return_delta_history=False):
    norm_bands = normalize_bands(bands, fs)

    M = (numtaps - 1)//2
    G = max(200, grid_mul*(M+1))

    pts_per_band = allocate_grid(norm_bands, G, min_pts=10)

    grid_freqs, grid_des, grid_wts = make_grid(
        norm_bands, desired, weights, pts_per_band, edge_margin,
    )

    n_extrema = M + 2
    # Initial extremal indices
    N = len(grid_freqs)
    k        = np.arange(n_extrema)
    xk       = np.cos(np.pi * k/(n_extrema-1))
    ext_idx  = np.round((1 - xk) * (N-1)/2).astype(int) 

    delta_hist = []
    prev_delta = None
    converged = None
    tol = 1e-4
    for j in range(1, max_iters + 1):
        assert len(ext_idx) == n_extrema, f"wrong #extrema: {len(ext_idx)} != {M+2}"
        a, delta = solve_remez_system(
            grid_freqs[ext_idx],
            grid_des[ext_idx],
            grid_wts[ext_idx],
            M, 
            eps
        )  
        delta_hist.append(delta)

        E = compute_error(a, grid_freqs, grid_des, grid_wts)
        cand_ext_idx = find_extrema(E, n_extrema)

        # Convergence checks
        same_ext = np.array_equal(ext_idx, cand_ext_idx)
        if prev_delta is not None:
            rel_change = abs(abs(delta) - abs(prev_delta)) / abs(delta)
            mag_stable = rel_change < tol
        else:
            mag_stable = False

        if j >= 5 and mag_stable and same_ext:
            converged = True
            break
 
        ext_idx = cand_ext_idx
        prev_delta = delta

    h = np.zeros(2*M + 1)
    h[M] = a[0]
    for k in range(1, M+1):
        h[M+k] = h[M-k] = a[k]/2

    return h, converged, delta_hist


def adaptive_remez_taps(numtaps, fs, bands, desired, weights, 
                        init_eps=1e-6, init_grid_mul=20, init_edge=0.001,
                        max_stages=5):
    eps = init_eps
    grid_mul = init_grid_mul
    edge_margin = init_edge

    h = None
    for stage in range(max_stages):
        try:
            h, converged, delta_hist = remez_taps(
                numtaps, fs, bands, desired, weights,
                eps=eps,
                grid_mul=grid_mul,
                edge_margin=edge_margin,
                max_iters=100,
                return_delta_history=True
            )
        except IndexError:
            converged = False

        plt.plot(delta_hist, marker='o')
        plt.xlabel('Iteration')
        plt.ylabel('Ripple amplitude δ')
        plt.title('Convergence history')
        plt.grid(True)
        plt.show()

        if converged:
            print(f"Coverged at stage {stage + 1}")
            return h
        
        print(f"Stage {stage+1} failed: tweaking params")
        eps *= 10
        edge_margin = min(0.015, edge_margin * 1.5)

    return h

#===============================
# Plotting 
#===============================
def plot_kernel(w, **plot_kwargs):

    plt.plot(np.arange(len(w)), w, **plot_kwargs)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title('Sinc Kernel')

def plot_filter_response(h, fs, nfft, **plot_kwargs):

    freq, H_db = spectrum(h, fs, nfft, 'fft')
    plt.plot(freq, H_db, **plot_kwargs)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Magnitude [dB]')
    plt.title('Filter Response')
    plt.grid(True)

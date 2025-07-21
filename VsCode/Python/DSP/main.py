from DSP.FIR_LPF import *
from DSP.spectrum import *
from DSP.plotting import *
from DSP. domains import *
from DSP.AM_demo import *

import numpy as np
import matplotlib.pyplot as plt

# =============================================================================
# Configuration and Signal Parameters
# =============================================================================
# Signal Params
fs = 4096                           # Sampling frequency in Hz
L = 8192                            # Number of samples (power of 2) {Nfft}
# Note: Width of fft bins is fs/Nfft. Multiply by n to get nth bin                         
tones = np.array([180, 200, 220])   # Tone frequencies (Hz)
amps = np.array([0.5, 1, 0.2])      # Tones amplitudes
noise_level = 0.2                   # Standard deviation of Gaussian noise

# Filtering Params
fc = 200            # Kernel frequency
N = 101             # No. of taps
M = 50              # (N - 1)/2 = 50
w_c1 = 2*np.pi*fc/fs # Stop band frequency
w_c2 = 2*np.pi*260/fs # Stop band frequency

# AM Params
A_c = 1.2
f_c = 1200
A_m = 0.5
f_m = 10
k = 0.8

idxs, frqs, t = domains(fs, L)

# =============================================================================
# Generating arrays
# =============================================================================
m = generate_signal(L, tones, amps, 0.001, t)
s = generate_AM(m, carrier(A_c, f_c, t), k)
# m_env = detect_env(s, 2*np.pi* f_m/fs, L, k)

# lo = LO(A_c, f_c, t)    # Just the carrier, in principle
# m_sync = np.append(sync_detect(s, lo, A_c, k, 1.2*w_c2, L)[M:], np.zeros(M))

# =============================================================================
# Generating filter response
# =============================================================================
# y = LPF(m, w_c1, L, N, M)
k = design_kaiser_fir(fs, 200, 220, 60)

m_filt = apply_zero_phase_filter(m, k)

# =============================================================================
# Time and spectrum plots
# =============================================================================
# plot_time(t, s) 
# plot_time(t, m)
# plot_spectrum(frqs, s, L, 'naive', 'hamming', 'AM spectrum')
# plot_2time(t, m, m_sync)
plot_2spectrum(m, apply_zero_phase_filter(m, k), fs, 'CT', ['Raw Signal', 'LPF'])
# plot_spectrum(frqs, m_sync, L, 'naive', 'hamming', 'AM spectrum')
plt.show()
import numpy as np
import matplotlib.pyplot as plt

from DSP.FIR_LPF import LPF

# =============================================================================
# Signal generation
# =============================================================================
def msg(A_m, f_m, t):
    return A_m*np.cos(2*np.pi*f_m*t)

def carrier(A_c, f_c, t):
    return A_c*np.cos(2*np.pi*f_c*t) 

def generate_AM(m, c, k):
    return (1 + k*m)*c

# =============================================================================
# Envelope detector demodulation
# =============================================================================
def detect_env(signal, w_c, L, k):
    m_rect = LPF(np.abs(signal), w_c, L)
    A_c_est = np.mean(m_rect)

    return (m_rect - A_c_est) / (A_c_est * k)
# |cos| has a nice expansion like pi/2 + cos(2w) + cos(4w) + ... 
# so the amplitude + message part stays behind as that DC comp after LPF

# =============================================================================
# Synchronous detector demodulation
# =============================================================================
def LO(A_lo, f_lo, t):
    return A_lo * np.cos(2*np.pi*f_lo*t)

def sync_detect(signal, lo, A_lo, k, w_c, L):
    x = signal * lo
    y = LPF(x, w_c, L)
    dc = np.mean(y)

    return 2*y/(A_lo**2*k) - dc

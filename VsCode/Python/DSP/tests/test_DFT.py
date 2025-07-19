import numpy as np
from DSP.DFT import *

def test_naive_dft():
    L = 2048
    fs = 4096
    idxs = np.arange(L)
    t = idxs / fs 
    signal = np.sin(2*np.pi*100*t)
    naive_fft = naive_dft(signal)
    fft = np.fft.fft(signal)
    for i in range(len(naive_fft)):
        assert abs(fft[i] - naive_fft[i]) < 1e-5
    

import numpy as np
from DSP.spectrum import *
from DSP.domains import *

def test_naive_dft():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    naive_fft = naive_dft(s)
    fft = np.fft.fft(s)
    for i in range(len(naive_fft)):
        assert abs(fft[i] - naive_fft[i]) < 1e-5

def test_CT_dft():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    naive_fft = naive_CT(s)
    fft = np.fft.fft(s)
    for i in range(len(naive_fft)):
        assert abs(fft[i] - naive_fft[i]) < 1e-5
    

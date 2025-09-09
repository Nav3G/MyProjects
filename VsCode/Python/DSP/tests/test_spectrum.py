from DSP.transforms import *
from DSP.spectrum import *
from DSP.plotting import *

import numpy as np
import matplotlib.pyplot as plt

def test_spectrum_windowing_0():
    fs = 4096                          
    L = 8192                                            
    tones = [180, 400, 301] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_spectrum((s, fs, 2048, 'fft', 'hann'), labels=['Hann Window'])
    plt.show()

def test_spectrum_windowing_1():
    fs = 4096                          
    L = 8192                                            
    tones = [180, 400, 301] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_spectrum((s, fs, 2048, 'fft', 'kaiser'), labels=['Kaiser Window'])
    plt.show()

def test_spectrum_windowing_2():
    fs = 4096                         
    L = 8192                                         
    tones = [184, 400, 300.1] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_spectrum((s, fs, 8192, 'fft', 'kaiser'), (s, fs, 8192, 'fft', 'rect'), 
                            labels=['Kaiser Window', 'Rectangular Window'])
    plt.show()
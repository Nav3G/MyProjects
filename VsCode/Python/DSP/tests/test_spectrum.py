from DSP.transforms import *
from DSP.spectrum import *
from DSP.plotting import *

import numpy as np
import matplotlib.pyplot as plt

def test_spectrum_windowing_0():
    fs = 4096                          
    L = 8192                                            
    tones = [180, 200.5, 220] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal([s], fs, 'freq', None, None, ['fft'], ['hann'], ['Hann'])
    ax.plot()
    plt.legend()
    plt.show()

def test_spectrum_windowing_1():
    fs = 4096                          
    L = 8192                                            
    tones = [180, 200.5, 220.4] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal([s], fs, 'freq', None, None, ['fft'], [None], ['Rect.'])
    ax.plot()
    plt.legend()
    plt.show()

def test_spectrum_windowing_2():
    fs = 4096                          
    L = 8192                                            
    tones = [180, 200.5, 220.4] 
    amps = [0.5, 1, 0.2]
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal([s, s], fs, 'freq', None, (0, 400), ['fft', 'fft'], 
                          ['hann', None], ['Hann', 'Rect.'])
    ax.plot()
    plt.legend()
    plt.show()
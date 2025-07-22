from DSP.plotting import *
from DSP.domains import *
from DSP.FIR_LPF import *

import numpy as np

def test_plotting_one_time_signal_default():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal(s, fs, 'time')
    ax.plot()
    plt.show()

def test_plotting_two_time_signals_xlim():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s1 = generate_signal(L, tones, amps, 0.001, times(fs, L))
    s2 = generate_signal(L, [130], [0.8], 0.001, times(fs, L))

    fig, ax = plot_signal([s1, s2], fs, 'time', (0, 0.5))
    ax.plot()
    plt.show()

def test_plotting_one_freq_signal_default():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal(s, fs, 'freq', ['naive'])
    ax.plot()
    plt.legend()
    plt.show()

def test_plotting_one_freq_signal_xlims():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal(s, fs, 'freq', None, (100, 300), ['fft'])
    ax.plot()
    plt.legend()
    plt.show()

def test_plotting_two_freq_signals():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    k = design_kaiser_fir(fs, 200, 220, 60)
    s_filt = apply_zero_phase_filter(s, k)

    fig, ax = plot_signal([s, s_filt], fs, 'freq', ['fft', 'fft'], ['Raw', 'LPF'])
    ax.plot()
    plt.legend()
    plt.show()
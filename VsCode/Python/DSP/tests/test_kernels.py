from DSP.kernels import *
from scipy import signal

import numpy as np

def test_plot_hpf_sinc():
    plot_kernel(sinc_hpf(200, 101, 2048))
    plt.show()

def test_plot_bpf_sinc():
    plot_kernel(sinc_bpf(200, 300, 201, 2048), label='bpf sinc')
    plt.grid()
    plt.legend()
    plt.show()

def test_plot_brf_sinc():
    plot_kernel(sinc_brf(200, 300, 201, 2048), label='brf sinc')
    plot_kernel(sinc_bpf(200, 300, 201, 2048), label='bpf sinc')
    plt.grid()
    plt.legend()
    plt.show()

def test_lpf_response():
    plot_filter_response(sinc_lpf(350, 101, 4096), 4096, 4096)
    plt.show()

def test_hpf_response():
    plot_filter_response(sinc_hpf(500, 101, 4096), 4096, 4096)
    plt.show()

def test_bpf_response():
    plot_filter_response(sinc_bpf(1000, 1500, 101, 4096), 4096, 4096)
    plt.ylim(-100, 10)
    plt.show()

def test_brf_response():
    plot_filter_response(sinc_brf(200, 300, 301, 4096), 4096, 4096)
    plt.show()

def test_equiripple_remez_taps_bpf():
    fs = 4096
    bands = [(0, 520), (600, 1420), (1500, fs/2)]
    desired = [0, 1, 0]
    weights = [10, 1, 10]
    h = adaptive_remez_taps(101, fs, bands, desired, weights, 1e-6, 16, 0.0, 4)  
    plot_filter_response(h, fs, 4096)
    plt.ylim((-100, 10))
    plt.show()

def test_equiripple_remez_taps_lpf():
    fs = 4096
    bands = [(0, 500), (600, fs/2)]
    desired = [1, 0]
    weights = [1, 10]
    h = adaptive_remez_taps(101, fs, bands, desired, weights, 1e-6, 20, 0.0, 1)  
    plot_filter_response(h, fs, 4096)
    plt.show()

def test_equiripple_remez_taps_lpf2():
    fs = 4096
    bands = [(0, 500), (600, fs/2)]
    desired = [1, 0]
    weights = [1, 10]
    h = adaptive_remez_taps(101, fs, bands, desired, weights, 1e-6, 20, 0.0, 1)  
    w, H = signal.freqz(h,   worN=4096, fs=fs)
    plt.plot(w, 20*np.log10(np.abs(H)), '-', label='mine', alpha=0.8)
    plt.title('Filter Comparison')
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Magnitude [dB]')
    plt.show()

def test_scipy_remez_lpf():
    numtaps = 101
    fs = 4096
    bands = [0, 500, 600, fs/2]
    desired = [1, 0]
    weights = [1, 10]

    h_ref = signal.remez(numtaps, bands, desired, weight=weights, fs=fs)
    plot_filter_response(h_ref, fs, 4096)
    plt.show()

def test_scipy_remez_bpf():
    numtaps = 101
    fs = 4096
    bands = [0, 520, 600, 1420, 1500, fs/2]
    desired = [0, 1, 0]
    weights = [10, 1, 10]

    h_ref = signal.remez(numtaps, bands, desired, weight=weights, fs=fs)
    plot_filter_response(h_ref, fs, 4096)
    plt.show()
    

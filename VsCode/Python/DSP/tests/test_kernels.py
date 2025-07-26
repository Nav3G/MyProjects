from DSP.kernels import *
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
    plot_filter_response(sinc_lpf(200, 301, 2048), 2048)
    plt.show()

def test_hpf_response():
    plot_filter_response(sinc_hpf(200, 301, 2048), 2048)
    plt.show()

def test_bpf_response():
    plot_filter_response(sinc_bpf(200, 300, 301, 2048), 2048)
    plt.show()

def test_brf_response():
    plot_filter_response(sinc_brf(200, 300, 301, 2048), 2048)
    plt.show()

def test_equiripple_remez_taps():
    fs = 2048
    bands = [(0, 300), (600, fs/2)]
    desired = [0, 1]
    weights = [5, 1]
    h = remez_taps('lowpass', 501, fs, bands, desired, weights)  
    plot_filter_response(h, fs)
    plt.show()

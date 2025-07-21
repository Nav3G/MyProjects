import numpy as np
import matplotlib.pyplot as plt

from DSP.spectrum import *
from DSP.domains import *

# =============================================================================
# Plotting Helpers
# =============================================================================
def plot_time(t, signal, start=0, end=None, title=None):
    """Plot time-domain signal in given index range."""
    end = end or len(signal)
    plt.figure(figsize=(8, 3))
    plt.plot(t[start:end], signal[start:end])
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title(title or f'Time Domain (n={start}:{end})')
    plt.grid(True)
    plt.tight_layout()
    plt.show()

def plot_2time(t, s1, s2, start=0, end=None, title=None):
    """PLot 2 time-domain signals in a given index range."""
    if start:
        st1 = start[0]
        st2 = start[1]
    else:
        st1 = 0
        st2 = 0
    if end:
        e1 = end[0]
        e2 = end[1]
    else:
        e1 = len(s1)
        e2 = len(s2)
        
    plt.figure(figsize=(8,3))
    plt.plot(t[st1:e1], s1[st1:e1])
    plt.plot(t[st2:e2], s2[st2:e2])
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title(title or f'Time Domain (n={st1}:{e1})')
    plt.grid(True)
    plt.tight_layout()
    plt.show()


def plot_2spectrum(x1, x2, fs, method='fft', labels=None, ax=None):
    """
    Compute and plot magnitude spectrum of signal x.
    method: 'naive' for naive_dft, 'fft' for radix2_fft
    window_type: as in make_window
    """
    # Transform
    f1, M1 = spectrum(x1, fs, method)
    f2, M2 = spectrum(x2, fs, method)

    assert len(f1)==len(M1) and len(f2)==len(M2)

    if ax is None:
        plt.figure(figsize=(8,4))
        ax = plt.gca()

    ax.plot(f1, M1, label=labels[0])
    ax.plot(f2, M2, label=labels[1])
    ax.set_xlabel('Frequency (Hz)')
    ax.set_ylabel('Magnitude (dB)')
    ax.set_xlim(0, fs/2)
    ax.grid(True)
    ax.legend()
    return ax

def plot_spectrum(x1, fs, method='fft', window_type='rectangular', label=None, ax=None):
    """
    Compute and plot magnitude spectrum of signal x.
    method: 'naive' for naive_dft, 'fft' for radix2_fft
    window_type: as in make_window
    """
    # Apply window

    # Transform
    f1, M1 = spectrum(x1, fs, method)

    assert len(f1)==len(M1)

    if ax is None:
        plt.figure(figsize=(8,4))
        ax = plt.gca()

    ax.plot(f1, M1, label=label)
    ax.set_xlabel('Frequency (Hz)')
    ax.set_ylabel('Magnitude (dB)')
    ax.set_xlim(0, fs/2)
    ax.grid(True)
    ax.legend()
    return ax

def plot_signal(x_list, fs, domain ='time', methods=None, labels=None, ax=None):
    """
    Plot one or more signals in time or frequency.

    Args:
        x_list: array or list of arrays of length N.
        fs: sample rate in Hz.
        domain: 'time' or 'freq'.
        methods: list of FFT methods (only for freq).
        labels: list of strings for legend.
        ax: existing matplotlib Axes (optional).
        **plot_kwargs: styling passed to ax.plot().

    Returns:
        fig: matplotlib Figure object.
        ax: matplotlib Axes object.
    """

    if not isinstance(x_list, list):
        x_list = [x_list]
    if domain not in ('time','freq'):
        raise ValueError(f"domain must be 'time' or 'freq', got {domain!r}")
    if labels is None:
        labels = [None] * len(x_list)
    elif isinstance(labels, str):
        labels = [labels] + [None] * (len(x_list) - 1)
    elif not isinstance(labels, list):
        raise ValueError("lables must be None, a string, or a lost of strings")
    if len(labels) != len(x_list):
        raise ValueError("lables length must match x_list")

    if ax is None:
        fig, ax = plt.subplots(figsize=(8, 4))
    else:
        fig = ax.get_figure()

    for i, x in enumerate(x_list):
        if domain == 'time':
            t = times(fs, len(x))
            data_list = [(t, x)]
            method_labels = [None]
        else:
            data_list = []
            method_labels = []
            for m in methods[i]:
                f, M = spectrum(x, fs, m)
                data_list.append((f, M))
                method_labels.append(m)
        for (t, data), lab in zip(data_list, method_labels):
            ax.plot(t, data, label=labels[i] or lab)

    ax.grid(True, linestyle='--', alpha=0.5)
    ax.set_xlabel('Time (s)' if domain =='time' else 'Frequecy (Hz)')
    ax.set_ylabel('Amplitude' if domain =='time' else 'Magnitude (dB)')
    if any(labels):
        ax.legend(loc='best', frameon=False)
    ax.set_title('Time Domain' if domain =='time' else 'Magnitude Spectrum')

    return fig, ax

        
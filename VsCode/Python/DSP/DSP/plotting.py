import numpy as np
import matplotlib.pyplot as plt

from DSP.spectrum import *
from DSP.domains import *

# =============================================================================
# Plotting Helpers
# =============================================================================
def plot_signal(x_list, fs, domain ='time', tlim: tuple[float,float]=None,  
                flim: tuple[float,float]=None, methods=None, window=None, labels=None, ax=None, **plot_kwargs):
    """
    Plot one or more signals in time or frequency.

    Args:
        x_list: array or list of arrays of length N.
        fs: sample rate in Hz.
        domain: 'time' or 'freq'.
        methods: list of FFT methods (only for freq).
        tlim: 2-tuple (t0, t1), only for domain='time'; if set, zooms to that time window.
        flim: 2-tuple (f0, f1), only for domain='freq'; if set, zooms to that frequency window.
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
    # Label normalization
    if labels is None:
        labels = [None] * len(x_list)
    elif isinstance(labels, str):
        labels = [labels] + [None] * (len(x_list) - 1)
    elif not isinstance(labels, list):
        raise ValueError("lables must be None, a string, or a lost of strings")
    if len(labels) != len(x_list):
        raise ValueError("lables length must match x_list")
    # Method normalization
    if methods is None:
        methods = [['fft'] for _ in x_list]
    elif all(isinstance(m, str) for m in methods):
        methods = [[m] for m in methods]
    elif any(not isinstance(m, list) for m in methods):
        raise ValueError("methods must be None, a flat list of strings, or a list of list of strings")
    if len(methods) != len(x_list):
        raise ValueError("methods must match shape of x_list")
    # Window checking
    if domain == 'time' and flim is not None:
        raise ValueError("If plotting in the time domain, flim does not apply")
    elif domain == 'freq' and tlim is not None:
        raise ValueError("If plotting in the frequency domain, tlim does not apply")

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
            data_list, method_labels =[], []
            for m in methods[i]:
                f, M = spectrum(x, fs, m, window[i])
                data_list.append((f, M))
                method_labels.append(m)
        for (t, data), lab in zip(data_list, method_labels):
            ax.plot(t, data, label=labels[i] or lab, **plot_kwargs)

    ax.grid(True, linestyle='--', alpha=0.5)
    ax.set_xlabel('Time (s)' if domain =='time' else 'Frequecy (Hz)')
    ax.set_ylabel('Amplitude' if domain =='time' else 'Magnitude (dB)')
    if any(labels):
        ax.legend(loc='best', frameon=False)
    ax.set_title('Time Domain' if domain =='time' else 'Magnitude Spectrum')

    # Windowing
    if domain == 'time' and tlim is not None:
        ax.set_xlim(*tlim)
    elif domain == 'freq' and flim is not None:
        ax.set_xlim(*flim)

    return fig, ax

        
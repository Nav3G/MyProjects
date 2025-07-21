from DSP.plotting import *
from DSP.domains import *

import numpy as np

def test_plotting():
    fs = 4096                          
    L = 8192                                            
    tones = np.array([180, 200, 220])   
    amps = np.array([0.5, 1, 0.2]) 
    s = generate_signal(L, tones, amps, 0.001, times(fs, L))

    fig, ax = plot_signal(s, fs, 'time')
    ax.plot()
    plt.show()
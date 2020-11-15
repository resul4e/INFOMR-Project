import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

volumes = np.loadtxt('volume_norm.csv', dtype='float', delimiter=',')
surface_area = np.loadtxt('surface_area_norm.csv', dtype='float', delimiter=',')
compactness = np.loadtxt('compactness_norm.csv', dtype='float', delimiter=',')
bounds = np.loadtxt('bounds_norm.csv', dtype='float', delimiter=',')
bounds_area = np.loadtxt('bounds_area_norm.csv', dtype='float', delimiter=',')
bounds_volume = np.loadtxt('bounds_volume_norm.csv', dtype='float', delimiter=',')
eccentricity = np.loadtxt('eccentricity_norm.csv', dtype='float', delimiter=',')

def ComputeDistribution(values):
    mean = values[0]
    stddev = values[1]
    values = np.divide(np.subtract(values[2:], mean), stddev)

    _ = plt.hist(values, bins=np.linspace(-3, 3, 35), rwidth=0.9)
    plt.title("Volume feature values standardized over database")
    plt.xlabel('Standard deviations')
    plt.ylabel("Number of shapes")
    plt.show()

ComputeDistribution(volumes)
ComputeDistribution(surface_area)
ComputeDistribution(compactness)
ComputeDistribution(bounds)
ComputeDistribution(bounds_area)
ComputeDistribution(bounds_volume)
ComputeDistribution(eccentricity)

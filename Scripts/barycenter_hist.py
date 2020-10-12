import matplotlib.pyplot as plt
import numpy as np

my_data = np.genfromtxt('barycenters.csv', delimiter=',')
print(my_data)
before = my_data[:, 0]
after = my_data[:, 1]

_ = plt.hist(after, bins=np.linspace(0, 1, 30), rwidth=0.9)  # arguments are passed to np.histogram
plt.title("Distance of barycenter to origin")
plt.xlabel('Distance')
plt.ylabel("Number of shapes")
plt.xlim(0, 1)
plt.show()

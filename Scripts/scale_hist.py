import matplotlib.pyplot as plt
import numpy as np

my_data = np.genfromtxt('scale.csv', delimiter=',')
print(my_data)
before = my_data[:, 0]
after = my_data[:, 1]

_ = plt.hist(before, bins=np.linspace(0.9, 1.1, 10), rwidth=0.9)  # arguments are passed to np.histogram
plt.title("Length of longest AABB edge")
plt.xlabel('Length')
plt.ylabel("Number of shapes")
plt.show()

_ = plt.hist(after, bins=np.linspace(0.9, 1.1, 10), rwidth=0.9)  # arguments are passed to np.histogram
plt.title("Length of longest AABB edge")
plt.xlabel('Length')
plt.ylabel("Number of shapes")
plt.show()

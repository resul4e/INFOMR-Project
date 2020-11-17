import matplotlib.pyplot as plt
import numpy as np

perf_knn = np.loadtxt('perf_knn.csv', delimiter=',')
perf_ann = np.loadtxt('perf_ann.csv', delimiter=',')
print(perf_ann)
K = np.arange(1, len(perf_knn)+1)
fig, ax = plt.subplots()
plt.plot(K, perf_knn)
plt.plot(K, perf_ann)
plt.xlim(K[0], K[-1])
plt.yscale('log')
plt.ylim(np.exp(0), 1000000)
plt.xlabel('Query size (k)')
plt.ylabel("Time (µs)")
plt.title("Average k-query time over all shapes")
plt.legend(['k-NN Query', 'FLANN Query'])
plt.grid()
plt.show()

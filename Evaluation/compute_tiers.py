import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

precision_knn = np.loadtxt('statistics_knn_1813.csv', dtype='str', delimiter=',')
precision_ann = np.loadtxt('statistics_ann_1813.csv', dtype='str', delimiter=',')

query_knn = precision_knn[:, 0]
results_knn = precision_knn[:, 1:]

query_ann = precision_ann[:, 0]
results_ann = precision_ann[:, 1:]

numShapes = len(query_knn)

classCounts = {}
for i in range(0, numShapes):
    if query_knn[i] in classCounts:
        classCounts[query_knn[i]] += 1
    else:
        classCounts[query_knn[i]] = 0

def ComputeKTiers(query_nn, results_nn):
    classTiers = {}
    for i in range(0, numShapes):
        query = query_nn[i]
        results = results_nn[i]
        numShapesInClass = classCounts[query]

        tierQuality = np.zeros(6, dtype=np.float32)
        for t in range(0, len(tierQuality)):
            tier = results[t*numShapesInClass:(t+1)*numShapesInClass]
            for j in range(0, numShapesInClass):
                if tier[j] == query:
                    tierQuality[t] += 1
            tierQuality[t] /= numShapesInClass
        #print(tierQuality[0])
        if (query not in classTiers):
            classTiers[query] = []
        else:
            classTiers[query].append(tierQuality)
    for c in classTiers:
        classTiers[c] = np.mean(classTiers[c], axis=0)
    return classTiers

classTiers = ComputeKTiers(query_knn, results_knn)
print(classTiers)

def plotClass(c):
    print(classTiers[c])
    cumTier = np.cumsum(classTiers[c])
    print(cumTier)
    plt.bar([1, 2, 3, 4, 5, 6], cumTier * 100, width=0.9, zorder=3)
    plt.title("Percentage of total humans correctly returned in first six tiers")
    plt.ylim(0, 100)
    plt.xlabel("Tier")
    plt.ylabel("Percentage returned in tier")
    plt.grid(zorder=0)
    plt.show()

plotClass('human')

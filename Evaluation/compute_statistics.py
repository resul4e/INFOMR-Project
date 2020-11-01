import matplotlib.pyplot as plt
import numpy as np

precision_knn = np.loadtxt('statistics_knn_1813.csv', dtype='str', delimiter=',')
precision_ann = np.loadtxt('statistics_ann_1813.csv', dtype='str', delimiter=',')
print(precision_ann.shape)

query_knn = precision_knn[:, 0]
results_knn = precision_knn[:, 1:]

classCounts = {}
for i in range(0, len(query_knn)):
    if query_knn[i] in classCounts:
        classCounts[query_knn[i]] += 1
    else:
        classCounts[query_knn[i]] = 0

print(classCounts)

def ComputeStatistics(query_nn, results_nn, k):
    precisions = np.zeros(len(query_nn))
    recalls = np.zeros(len(query_nn))
    specificities = np.zeros(len(query_nn))

    for i in range(0, len(query_nn)):
        query = query_nn[i]
        results = results_nn[i]
        
        correct = 0
        for j in range(0, k):
            if (query == results[j]):
                correct += 1

        TP = correct
        FP = k - correct
        FN = classCounts[query] - correct
        TN = len(query_nn) - FP

        precisions[i] = TP / (TP + FP)
        recalls[i] = TP / (TP + FN)
        specificities[i] = TN / (FP + TN)

    return (np.mean(precisions), np.mean(recalls), np.mean(specificities))

for k in range(1, 30+1):
    print(k)
    ComputeStatistics(query_knn, results_knn, k)
# K = np.arange(1, len(precision_ann)+1)

# plt.plot(K, precision_ann)
# plt.plot(K, recall_ann)
# plt.xlabel('K')
# plt.ylabel("Number of shapes")
# plt.xlim(1, len(precision_ann))
# plt.ylim(0, 1)
# plt.show()

# plt.plot(recall_ann, precision_ann)
# plt.xlabel('Recall')
# plt.ylabel("Precision")
# plt.xlim(0, 1)
# plt.ylim(0, 1)
# plt.show()
# _ = plt.hist(after, bins=np.linspace(0, 1, 30), rwidth=0.9)  # arguments are passed to np.histogram
# plt.title("Absolute Cosine Similarity between Major Eigenvector and X-axis")

# plt.xlim(0, 1)
# plt.show()

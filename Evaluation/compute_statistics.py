import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

precision_knn = np.loadtxt('statistics_knn_1813.csv', dtype='str', delimiter=',')
precision_ann = np.loadtxt('statistics_ann_1813.csv', dtype='str', delimiter=',')
print(precision_ann.shape)

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

print(classCounts)

def ComputeStatistics(query_nn, results_nn, k):
    precisions = np.zeros(numShapes)
    recalls = np.zeros(numShapes)
    specificities = np.zeros(numShapes)
    accuracies = np.zeros(numShapes)

    for i in range(0, numShapes):
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
        accuracies[i] = (TP + TN) / numShapes

    return (np.mean(precisions), np.mean(recalls), np.mean(specificities), np.mean(accuracies))

column_names = ["K", "Precision", "Recall", "Specificity", "Accuracy"]
df_knn = pd.DataFrame(columns = column_names)
df_ann = pd.DataFrame(columns = column_names)
for k in range(1, numShapes):
    print(k)
    (precision, recall, specificities, accuracies) = ComputeStatistics(query_knn, results_knn, k)
    df_knn.loc[k] = [k, precision, recall, specificities, accuracies]

    (precision, recall, specificities, accuracies) = ComputeStatistics(query_ann, results_ann, k)
    df_ann.loc[k] = [k, precision, recall, specificities, accuracies]

print(df_knn)

# K = np.arange(1, len(precision_ann)+1)

# plt.plot(df_knn['K'], df_knn['Precision'])
# plt.plot(df_knn['K'], df_knn['Recall'])
# plt.plot(df_ann['K'], df_ann['Precision'])
# plt.plot(df_ann['K'], df_ann['Recall'])
# plt.xlabel('K')
# plt.ylabel("Number of shapes")
# plt.xlim(df_knn['K'].iloc[0], df_knn['K'].iloc[-1])
# plt.ylim(0, 0.5)
# plt.show()

# plt.plot(df_knn['Recall'], df_knn['Precision'])
# plt.plot(df_ann['Recall'], df_ann['Precision'])
# plt.xlabel('Recall')
# plt.ylabel("Precision")
# plt.xlim(0, 1)
# plt.ylim(0, 1)
# plt.show()

fig = plt.figure()
ax = fig.add_subplot(111)
plt.plot(df_knn['Recall'], df_knn['Specificity'])
plt.plot(df_ann['Recall'], df_ann['Specificity'])
plt.axline((0, 1), (1, 0), linestyle='--', color='grey')
plt.title("ROC Curve for Queries on All Models in the Database")
plt.xlabel('Sensitivity')
plt.ylabel("Specificity")
plt.xlim(0, 1)
plt.ylim(0, 1)
ax.set_aspect('equal', adjustable='box')
plt.legend(['Naive k-NN', 'FLANN k-NN'])
plt.grid()
plt.show()
# _ = plt.hist(after, bins=np.linspace(0, 1, 30), rwidth=0.9)  # arguments are passed to np.histogram
# plt.title("Absolute Cosine Similarity between Major Eigenvector and X-axis")

# plt.xlim(0, 1)
# plt.show()

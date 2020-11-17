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
        classCounts[query_knn[i]] = 1

print(classCounts)

column_names = ["Model#", "Precision", "Recall", "Specificity", "Accuracy"]

def ComputeStatisticsPerClass(query_nn, results_nn):
    class_dfs = {}

    for i in range(0, numShapes):
        query = query_nn[i]
        results = results_nn[i]

        if (query not in class_dfs):
            class_dfs[query] = pd.DataFrame(columns = column_names)
        
        maxK = 20
        precisions = np.zeros(maxK)
        recalls = np.zeros(maxK)
        specificities = np.zeros(maxK)
        accuracies = np.zeros(maxK)
        for k in range(1, maxK):
            correct = 0
            for j in range(0, k):
                if (query == results[j]):
                    correct += 1

            TP = correct
            FP = k - correct
            FN = classCounts[query] - correct
            TN = len(query_nn) - FP
            
            precisions[k] = TP / (TP + FP)
            recalls[k] = TP / (TP + FN)
            specificities[k] = TN / (FP + TN)
            accuracies[k] = (TP + TN) / numShapes

        class_dfs[query].loc[i] = [i, np.mean(precisions), np.mean(recalls), np.mean(specificities), np.mean(accuracies)]
    return class_dfs

class_dfs = ComputeStatisticsPerClass(query_knn, results_knn)

print(class_dfs['plant'].shape)
print(classCounts['plant'])

def ComputeMeanAveragePrecisionGlobal(class_dfs):
    global_df = pd.DataFrame(columns = column_names)
    for c in class_dfs:
        global_df = global_df.append(class_dfs[c])
    print(global_df)
    return global_df["Precision"].mean()

def ComputeMeanAveragePrecisionPerClass(class_dfs):
    class_maps = {}
    global_mean = 0
    for c in class_dfs:
        class_maps[c] = round(class_dfs[c]["Precision"].mean(), 2)
        global_mean += class_dfs[c]["Precision"].mean()

    return {k: v for k, v in sorted(class_maps.items(), key=lambda item: item[1], reverse=True)}

class_map = ComputeMeanAveragePrecisionPerClass(class_dfs)
global_map = ComputeMeanAveragePrecisionGlobal(class_dfs)
print(class_map.keys())
print(class_map.values())
print(global_map)

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

# plt.plot(df_knn['Recall'], df_knn['Specificity'])
# plt.plot(df_ann['Recall'], df_ann['Specificity'])
# plt.axline((0, 1), (1, 0), linestyle='--', color='grey')
# plt.title("ROC Curve for Queries on All Models in the Database")
# plt.xlabel('Sensitivity')
# plt.ylabel("Specificity")
# plt.xlim(0, 1)
# plt.ylim(0, 1)
# plt.legend(['k-NN', 'ANN'])
# plt.grid()
# plt.show()

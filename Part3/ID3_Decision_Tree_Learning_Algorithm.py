import pandas as pd
import math
from sklearn.tree import DecisionTreeClassifier, plot_tree
import matplotlib.pyplot as plt

# Load your dataset from a CSV file
df = pd.read_csv('final_dataset.csv')

# Define a function to calculate entropy for a given dataset
def entropy(dataset):
    labels = dataset['Action Choice'].unique()
    entropy_val = 0
    for label in labels:
        label_count = len(dataset[dataset['Action Choice'] == label])
        prob_label = label_count / len(dataset)
        entropy_val -= prob_label * math.log2(prob_label)
    return entropy_val

# Define a function to calculate information gain for a given attribute
def information_gain(dataset, attribute):
    attribute_values = dataset[attribute].unique()
    entropy_after_split = 0
    for value in attribute_values:
        subset = dataset[dataset[attribute] == value]
        prob_value = len(subset) / len(dataset)
        entropy_after_split += prob_value * entropy(subset)
    return entropy(dataset) - entropy_after_split

# Define the ID3 decision tree learning algorithm with max leaf nodes
def ID3(dataset, attributes, target_attribute='Action Choice', max_leaf_nodes=None):
    # If all examples have the same target value, return a leaf node with that value
    if len(dataset[target_attribute].unique()) == 1 or len(attributes) == 0 or (max_leaf_nodes is not None and len(dataset) <= max_leaf_nodes):
        return dataset[target_attribute].mode().iloc[0]
    
    # Choose the best attribute based on information gain
    best_attribute = max(attributes, key=lambda attr: information_gain(dataset, attr))
    
    # Create a decision tree node with the best attribute
    tree = {best_attribute: {}}
    remaining_attributes = [attr for attr in attributes if attr != best_attribute]
    
    # Recursively split the dataset and build the tree
    for value in dataset[best_attribute].unique():
        subset = dataset[dataset[best_attribute] == value]
        tree[best_attribute][value] = ID3(subset, remaining_attributes, target_attribute, max_leaf_nodes)
    
    return tree

# List of attributes in your dataset excluding the target attribute
attributes = ['Same Room', 'Distance To Char', 'Distance To Room Center']

# Build the decision tree with a maximum of 6 leaf nodes
max_leaf_nodes = 6
decision_tree = ID3(df, attributes, max_leaf_nodes=max_leaf_nodes)

# Plot the decision tree
plt.figure(figsize=(10, 7))
clf = DecisionTreeClassifier(max_leaf_nodes=max_leaf_nodes)
plot_tree(clf.fit(df[attributes], df['Action Choice']), feature_names=attributes, class_names=['1', '2', '3', '4'], filled=True)
plt.show()

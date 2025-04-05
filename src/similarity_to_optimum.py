import pathlib

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


def hamming_similarity(a, b):
    """Calculate the Hamming distance between two lists."""
    return np.sum(np.array(a) == np.array(b)) / len(a)


def get_list(s):
    """Convert a string of numbers into a list of integers."""
    return [int(x) for x in s.split()]


if __name__ == "__main__":
    for file in pathlib.Path("results").glob("*_ls.csv"):
        instance = file.name.split("_")[0]
        with open(f"data/qaplib/{instance}.sln") as f:
            lines = f.readlines()
            best_score = int(lines[0].split()[1])
            best_sol = get_list("".join(lines[1:]))

        fig = plt.figure(figsize=(10, 6))

        df = pd.read_csv(f"results/{instance}_ls.csv")
        df["hamming"] = df["solution"].apply(
            lambda x: hamming_similarity(get_list(x), best_sol)
        )

        sns.scatterplot(
            x="score",
            y="hamming",
            data=df,
        )
        plt.xlabel("Score")
        plt.ylabel("Hamming similarity to optimal solution")
        plt.axvline(x=best_score, color="r", linestyle="--")
        plt.title("Similarity to optimal solution for " + instance)
        plt.tight_layout()

        plt.savefig(f"plots/{instance}_scatter.png")

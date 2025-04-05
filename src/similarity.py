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
        print(f"Plotting similarity to optimal solution for {file.name}")
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

        plt.savefig(f"plots/{instance}_similarity_global.png")

        # similarity between local optima
        print(f"Plotting similarity between local optima for {instance}")
        fig = plt.figure(figsize=(10, 6))
        df_full = pd.DataFrame()
        for algorithm in df["alg"].unique():
            dfa = df[df["alg"] == algorithm][["score", "solution"]]
            dfa = dfa.sample(50, random_state=42)
            df_cross = dfa.reset_index().merge(
                dfa.reset_index(), how="cross", suffixes=("_a", "_b")
            )
            df_cross = df_cross[df_cross["index_a"] != df_cross["index_b"]]
            df_cross.drop(columns=["index_a", "index_b"], inplace=True)
            df_cross["similarity"] = df_cross.apply(
                lambda x: hamming_similarity(
                    get_list(x["solution_a"]), get_list(x["solution_b"])
                ),
                axis=1,
            )
            df_cross["alg"] = algorithm
            df_full = pd.concat([df_full, df_cross], ignore_index=True)

        df_full["score_diff"] = (df_full["score_a"] - df_full["score_b"]).abs()

        sns.scatterplot(x="score_diff", y="similarity", data=df_full, hue="alg")
        plt.xlabel("Difference in score")
        plt.ylabel("Hamming similarity")
        plt.title("Similarity between local optima for " + instance)
        plt.tight_layout()

        plt.savefig(f"plots/{instance}_similarity_local.png")

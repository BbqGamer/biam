import pathlib

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

if __name__ == "__main__":
    for file in pathlib.Path("results").glob("*[!ls].csv"):
        problem = file.name.split(".")[0]

        with open(f"data/qaplib/{problem}.sln") as f:
            lines = f.readlines()
            best = float(lines[0].split()[1])

        df = pd.read_csv(file)
        df["dist_from_best"] = df["score"] - best
        df["quality_over_time"] = df["score"] / df["time"]

        fig, axs = plt.subplots(1, 5, figsize=(24, 6))

        axs[0].set_ylabel("time (ms)")
        sns.boxplot(x="alg", y="time", data=df, ax=axs[0])

        axs[1].set_ylabel("distance from best solution")
        sns.boxplot(x="alg", y="dist_from_best", data=df, ax=axs[1])

        axs[2].set_ylabel("efficiency (score/time) - log scale")
        axs[2].set_yscale("log")
        sns.boxplot(x="alg", y="quality_over_time", data=df, ax=axs[2])

        steps = df[df["alg"].isin(["G", "S"])]

        axs[3].set_ylabel("number of steps")
        sns.boxplot(x="alg", y="steps", data=steps, ax=axs[3])

        evals = df[df["alg"].isin(["G", "S", "RS", "RW"])]
        axs[4].set_ylabel("number of solution evaluations")
        sns.boxplot(x="alg", y="evals", data=evals, ax=axs[4])

        for ax in axs:
            ax.set_xlabel("")

        fig.suptitle(problem)
        plt.tight_layout()
        plt.savefig(f"plots/{problem}.png")

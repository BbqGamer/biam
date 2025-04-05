import pathlib

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


def cum_mean(arr):
    cum_sum = np.cumsum(arr, axis=0)
    for i in range(cum_sum.shape[0]):
        if i == 0:
            continue
        cum_sum[i] = cum_sum[i] / (i + 1)
    return cum_sum


def get_list(s):
    """Convert a string of numbers into a list of integers."""
    return [int(x) for x in s.split()]


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
        plt.close(fig)

    for file in pathlib.Path("results").glob("*ls.csv"):
        instance = file.name.split(".")[0][:-3]
        with open(f"data/qaplib/{instance}.sln") as f:
            lines = f.readlines()
            best_score = float(lines[0].split()[1])
            best_sol = get_list(lines[1].strip())

        problem = file.name.split(".")[0][:-3]
        df = pd.read_csv(file)
        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.set_ylabel("Score")
        ax.set_xlabel("Repetition")
        sns.lineplot(
            np.minimum.accumulate(df[df["alg"] == "S"]["score"].values.tolist()),
            label="best S",
            color="orange",
            linewidth=2,
        )
        sns.lineplot(
            cum_mean(df[df["alg"] == "S"]["score"].values),
            label="average S",
            color="orange",
            linestyle="--",
        )

        sns.lineplot(
            np.minimum.accumulate(df[df["alg"] == "G"]["score"].values.tolist()),
            label="best G",
            color="blue",
            linewidth=2,
        )
        sns.lineplot(
            cum_mean(df[df["alg"] == "G"]["score"].values),
            label="average G",
            color="blue",
            linestyle="--",
        )
        ax.axhline(y=best_score, color="r", linestyle="--", label="global optimum")

        plt.title(problem)
        plt.savefig(f"plots/{problem}_multistart_ls.png")
        plt.close(fig)

        fig = plt.figure()
        plt.tight_layout()
        plt.title(f"Starting vs ending score for {instance}")
        sns.scatterplot(x="start_score", y="score", data=df, hue="alg")
        plt.xlabel("Starting score")
        plt.ylabel("Ending score")
        plt.axhline(y=best_score, color="r", linestyle="--")
        plt.axvline(x=best_score, color="r", linestyle="--")
        plt.savefig(f"plots/{problem}_start_vs_end.png")
        plt.close(fig)

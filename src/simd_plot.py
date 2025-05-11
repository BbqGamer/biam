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


METRIC = "time"

if __name__ == "__main__":
    all_df = pd.DataFrame()
    for file in pathlib.Path("results").glob("*[!_]*.csv"):
        if "_" in file.name:
            continue
        print(file.name)
        problem = file.name.split(".")[0]
        df = pd.read_csv(file)
        df = df[df["alg"].isin(["G", "S"])]
        df["instance"] = problem
        df["opt"] = "no optimization"
        all_df = pd.concat([all_df, df], ignore_index=True)

    df_simd = pd.DataFrame()
    for file in pathlib.Path("results").glob("*_simd.csv"):
        problem = file.name.split("_")[0]
        df = pd.read_csv(file)
        df = df[df["alg"].isin(["G", "S"])]
        df["instance"] = problem
        df["opt"] = "simd"
        all_df = pd.concat([all_df, df], ignore_index=True)

    legend_added = False
    fig, axs = plt.subplots(1, all_df["instance"].nunique(), figsize=(20, 10))
    axs = axs.flatten()
    for i, instance in enumerate(
        sorted(
            all_df["instance"].dropna().unique(),
            key=lambda a: int("".join([x for x in a if x.isnumeric()])),
        )
    ):
        fig.suptitle(
            "Comparison of performance of original implementation and SIMD accelerated version"
        )
        plt.tight_layout()
        axs[i].get_yaxis()
        axs[i].set_title(instance)

        sns.boxplot(
            all_df[all_df["instance"] == instance],
            x="alg",
            y="time",
            hue="opt",
            ax=axs[i],
        )
        axs[i].set_ylabel("")
        axs[i].set_xlabel("")

        if legend_added:
            axs[i].get_legend().remove()
        else:
            axs[i].legend(title="Optimization", loc="upper right")
            legend_added = True
    plt.savefig("plots/times_simd.png")
    plt.close(fig)

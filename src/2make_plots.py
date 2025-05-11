import pathlib

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def get_list(s):
    """Convert a string of numbers into a list of integers."""
    return [int(x) for x in s.split()]


ALGS = ["G", "S", "SA", "TS"]

if __name__ == "__main__":
    df_all = pd.DataFrame()
    for file in pathlib.Path("results").glob("2*.csv"):
        print(file.name)
        problem = file.name.split(".")[0].split("_")[0][1:]

        with open(f"data/qaplib/{problem}.sln") as f:
            lines = f.readlines()
            best = float(lines[0].split()[1])

        df = pd.read_csv(file)
        df["dist_from_best"] = df["score"] - best
        df["quality_over_time"] = df["score"] / df["time"]
        df["instance"] = problem
        df_all = pd.concat([df_all, df], ignore_index=True)

    metric_map = {
        "time": "Time (ms)",
        "score": "Score",
        "dist_from_best": "Difference in score from optimum",
        "quality_over_time": "Quality over time (log scale)",
        "steps": "Number of steps",
        "evals": "Number of evaluations",
    }
    for metric in metric_map.keys():
        fig, axs = plt.subplots(1, df_all["instance"].nunique(), figsize=(20, 5))
        for i, instance in enumerate(
            sorted(
                df_all["instance"].dropna().unique(),
                key=lambda a: int("".join([x for x in a if x.isnumeric()])),
            )
        ):
            fig.suptitle(metric_map[metric])
            plt.tight_layout()
            axs[i].get_yaxis()
            axs[i].set_title(instance)

            if metric == "quality_over_time":
                axs[i].set_yscale("log")

            df_plot = df_all[df_all["alg"].isin(ALGS)]

            sns.boxplot(
                df_plot[df_plot["instance"] == instance],
                x="alg",
                y=metric,
                ax=axs[i],
                order=ALGS,
            )
            axs[i].set_ylabel("")
            axs[i].set_xlabel("")
        plt.savefig(f"plots/2{metric}.png")
        plt.close(fig)

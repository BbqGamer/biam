import multiprocessing
import os

instances = [
    "bur26a",
    "chr25a",
    "els19",
    "esc128",
    "had20",
    "lipa90b",
    "tai256c",
    "wil100",
]
instances_msls = ["tai256c", "lipa90b"]


def benchmark(instance):
    command = f"./build/test_qap data/qaplib/{instance}.dat > results/{instance}.csv"
    os.system(command)


def benchmark_ls(instance):
    command = f"./build/test_qap -l -K 350 data/qaplib/{instance}.dat > results/{instance}_ls.csv"
    os.system(command)


if __name__ == "__main__":
    with multiprocessing.Pool(processes=16) as pool:
        pool.map(benchmark, instances)
        pool.map(benchmark_ls, instances_msls)

    print("All instances have been processed.")

import multiprocessing
import os

instances = [
    "bur26a",
    "chr15c",
    "chr25a",
    "els19",
    "had20",
    "lipa90a",
    "tai256c",
    "wil100",
]


def benchmark(instance):
    command = f"nice -n 20 ./build/test_qap data/qaplib/{instance}.dat > results/{instance}.csv"
    os.system(command)


def benchmark_ls(instance):
    command = f"nice -n 20 ./build/test_qap -l -K 300 data/qaplib/{instance}.dat > results/{instance}_ls.csv"
    os.system(command)

def benchmark_sa(instance):
    command = f"nice -n 20 ./build/test_sa data/qaplib/{instance}.dat > results/{instance}_sa.csv"
    os.system(command)

def benchmark_ts(instance):
    command = f"nice -n 20 ./build/test_ts data/qaplib/{instance}.dat > results/{instance}_ts.csv"
    os.system(command)

if __name__ == "__main__":
    with multiprocessing.Pool(processes=16) as pool:
        # pool.map(benchmark, instances)
        # pool.map(benchmark_ls, instances)
        pool.map(benchmark_sa, instances)
        # pool.map(benchmark_ts, instances)

    print("All instances have been processed.")

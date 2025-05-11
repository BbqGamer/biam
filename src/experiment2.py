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

ts_patiences = [50, 1000, 1000, 500, 100, 100, 100, 100]
sa_alphas = [0.5, 0.8, 0.8, 0.7, 0.6, 0.75, 0.99, 0.8]

K = 20


def benchmark_ls(instance):
    command = f"nice -n -20 ./build/test_qap -l -K {K} data/qaplib/{instance}.dat > results/2{instance}_ls.csv"
    os.system(command)


def benchmark_sa(data):
    instance, alpha = data
    command = f"nice -n -20 ./build/test_sa -K {K} -a {alpha} data/qaplib/{instance}.dat > results/2{instance}_sa.csv"
    os.system(command)


def benchmark_ts(data):
    instance, patience = data
    command = f"nice -n -20 ./build/test_ts -K {K} -p {patience} data/qaplib/{instance}.dat > results/2{instance}_ts.csv"
    os.system(command)


if __name__ == "__main__":
    with multiprocessing.Pool(processes=16) as pool:
        pool.map(benchmark_ls, instances)
        pool.map(benchmark_sa, zip(instances, sa_alphas))
        pool.map(benchmark_ts, zip(instances, ts_patiences))

    print("All instances have been processed.")

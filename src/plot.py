import pandas as pd
import matplotlib.pyplot as plt

# Experiment 1: FPR vs Number of Hash Functions
def plot_exp1():
    df = pd.read_csv("results/exp1.csv")
    plt.figure()
    plt.plot(df["number_of_hash_functions"], df["bloom_filter_fpr"], marker="o", label="Bloom Filter")
    plt.plot(df["number_of_hash_functions"], df["blocked_bloom_filter_fpr"], marker="o", label="Blocked Bloom Filter")
    plt.xlabel("Number of hash functions (k)")
    plt.ylabel("False positive rate")
    plt.title("FPR vs Number of Hash Functions")
    plt.legend()
    plt.grid(True)
    plt.savefig("results/exp1.png")
    plt.close()

# Experiment 2: FPR vs Bits per Element
def plot_exp2():
    df = pd.read_csv("results/exp2.csv")
    plt.figure()
    plt.plot(df["bits_per_element"], df["bloom_filter_fpr"], marker="o", label="Bloom Filter")
    plt.plot(df["bits_per_element"], df["blocked_bloom_filter_fpr"], marker="o", label="Blocked Bloom Filter")
    plt.xlabel("Bits per element")
    plt.ylabel("False positive rate")
    plt.title("FPR vs Bits per Element")
    plt.legend()
    plt.grid(True)
    plt.savefig("results/exp2.png")
    plt.close()

# Experiment 3: Throughput vs Filter Size
def plot_exp3():
    df = pd.read_csv("results/exp3.csv")
    bf = df[df["filter_t"] == "bloom_filter"]
    bbf = df[df["filter_t"] == "blocked_bloom_filter"]

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    fig.suptitle("Throughput vs Filter Size")

    for ax, col, title in zip(axes, ["insert_ns", "hit_ns", "miss_ns"], ["Insert", "Hit", "Miss"]):
        ax.plot(bf["filter_sz_bits"],  bf[col],  marker="o", label="Bloom Filter")
        ax.plot(bbf["filter_sz_bits"], bbf[col], marker="o", label="Blocked Bloom Filter")
        ax.set_xscale("log", base=2)
        ax.set_xlabel("Filter size (bits)")
        ax.set_ylabel("ns/op")
        ax.set_title(title)
        ax.legend()
        ax.grid(True)

    plt.tight_layout()
    plt.savefig("results/exp3.png")
    plt.close()

# Experiment 4: Throughput vs Load Factor
def plot_exp4():
    df = pd.read_csv("results/exp4.csv")
    bf = df[df["filter_t"] == "bloom_filter"]
    bbf = df[df["filter_t"] == "blocked_bloom_filter"]

    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    fig.suptitle("Throughput vs Load Factor")

    for ax, col, title in zip(axes.flat, ["ins_ns", "hit_ns", "miss_ns"], ["Insert", "Hit", "Miss"]):
        ax.plot(bf["load"],  bf[col],  marker="o", label="Bloom Filter")
        ax.plot(bbf["load"], bbf[col], marker="o", label="Blocked Bloom Filter")
        ax.set_xlabel("Load factor")
        ax.set_ylabel("ns/op")
        ax.set_title(title)
        ax.legend()
        ax.grid(True)

    # FPR subplot
    axes[1][1].plot(bf["load"],  bf["fpr"],  marker="o", label="Bloom Filter")
    axes[1][1].plot(bbf["load"], bbf["fpr"], marker="o", label="Blocked Bloom Filter")
    axes[1][1].set_xlabel("Load factor")
    axes[1][1].set_ylabel("FPR")
    axes[1][1].set_title("FPR")
    axes[1][1].legend()
    axes[1][1].grid(True)

    plt.tight_layout()
    plt.savefig("results/exp4.png")
    plt.close()

if __name__ == "__main__":
    plot_exp1()
    plot_exp2()
    plot_exp3()
    plot_exp4()
    print("Plots saved to results/")
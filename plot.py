"""Analyze the spotify data."""

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

N_CORES = 16

sns.set_theme()


# PLOT
def load_data(name):
    ret = pd.read_csv(name.lower() + ".csv")
    ret["Barrier"] = [name] * len(ret)
    return ret


centralized = load_data("Centralized")
dissemination = load_data("Dissemination")
mcs = load_data("MCS")

df = centralized.merge(dissemination, how="outer").merge(mcs, how="outer")

splot = sns.scatterplot(data=df, x="Threadcount", y="Time", hue="Barrier")
splot.set(
    yscale="log",
    xlabel="Thread Count",
    ylabel="Time (ns)",
)

splot.set_title("Barrier Runtime", fontsize=18)

plt.savefig("bench.png", bbox_inches="tight")
plt.clf()


# WITH UNUSED
overhead = load_data("Overhead")

with_overhead = df.merge(overhead, how="outer")

splot = sns.scatterplot(data=with_overhead, x="Threadcount", y="Time", hue="Barrier")
splot.set(
    yscale="log",
    xlabel="Thread Count",
    ylabel="Time (ns)",
)

splot.set_title("Barrier Runtime with Overhead", fontsize=18)

plt.savefig("bench-with-overhead.png", bbox_inches="tight")
plt.clf()


# SMALL
under17 = with_overhead[with_overhead["Threadcount"] <= N_CORES]

splot = sns.scatterplot(data=under17, x="Threadcount", y="Time", hue="Barrier")
splot.set(
    xlabel="Thread Count",
    ylabel="Time (ns)",
)

splot.set_title("Barrier Runtime (Small Threadcount)", fontsize=18)

plt.savefig("bench-small.png", bbox_inches="tight")
plt.clf()


# SMALL, RESIDUALS
centralized["Time"] = centralized["Time"] - overhead["Time"]
dissemination["Time"] = dissemination["Time"] - overhead["Time"]
mcs["Time"] = mcs["Time"] - overhead["Time"]

df = centralized.merge(dissemination, how="outer").merge(mcs, how="outer")

residuals = df[df["Threadcount"] <= N_CORES]

splot = sns.scatterplot(data=residuals, x="Threadcount", y="Time", hue="Barrier")
splot.set(
    xlabel="Thread Count",
    ylabel="Time (ns)",
)

splot.set_title("Residual Runtime (Small Threadcount)", fontsize=18)

plt.savefig("bench-residuals.png", bbox_inches="tight")
plt.clf()

# LARGE
large = df[df["Threadcount"] > N_CORES]

splot = sns.scatterplot(data=large, x="Threadcount", y="Time", hue="Barrier")
splot.set(
    xlabel="Thread Count",
    ylabel="Time (ns)",
)

splot.set_title("Barrier Runtime (Large Threadcount)", fontsize=18)

plt.savefig("bench-large.png", bbox_inches="tight")
plt.clf()

from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

repo = Path("/home/aditya/Desktop/advanceWirelessNetworks")
csv  = repo / "Lab-01-Propagation" / "submission" / "lab1_results.csv"
outd = repo / "Lab-01-Propagation" / "submission"
outd.mkdir(parents=True, exist_ok=True)  # will raise if no perms

df = pd.read_csv(csv).sort_values(["model", "distance_m"])

# All models
plt.figure()
for m in df["model"].unique():
    sub = df[df["model"] == m]
    plt.plot(sub["distance_m"], sub["throughput_bps"]/1e6, marker='o', label=m)
plt.xlabel("Distance (m)"); plt.ylabel("Throughput (Mbps)")
plt.title("Throughput vs Distance (Friis vs TwoRay vs COST231)")
plt.legend(); plt.grid(True, linestyle="--", alpha=0.4); plt.tight_layout()
plt.savefig(outd / "throughput_vs_distance_all.png", dpi=150)

# Per model
for m in df["model"].unique():
    sub = df[df["model"] == m]
    plt.figure()
    plt.plot(sub["distance_m"], sub["throughput_bps"]/1e6, marker='o')
    plt.xlabel("Distance (m)"); plt.ylabel("Throughput (Mbps)")
    plt.title(f"Throughput vs Distance ({m})")
    plt.grid(True, linestyle="--", alpha=0.4); plt.tight_layout()
    plt.savefig(outd / f"throughput_vs_distance_{m.lower()}.png", dpi=150)

print(f"Plots written under {outd}")

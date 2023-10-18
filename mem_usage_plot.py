import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, List, Tuple

import matplotlib.animation as animation
import matplotlib.patches as patches
import matplotlib.pyplot as plt

BLUE = "#99d2ec"
MAGENTA = "#eaa0d2"
YELLOW = "#fdf584"
RED = "#ea8080"
GREEN = "#a6ec98"
ORANGE = "#f7c790"

COLORS = [
    BLUE,
    MAGENTA,
    YELLOW,
    RED,
    GREEN,
    ORANGE,
]


def read_log(log_file_path: Path) -> Tuple[int, List[Tuple[str, Any]]]:
    with open(log_file_path, "r", encoding="utf-8") as log_fd:
        mem_size = int(log_fd.readline()[:-1])
        logs = []
        success = False
        for line in log_fd.readlines():
            if line == "end":
                success = True
                break
            data = line[:-1].split()
            value = (
                (int(data[1]), int(data[2]), int(data[3]), int(data[4]))
                if data[0] == "o"
                else int(data[1])
            )
            logs.append((data[0], value))
    assert success, "Test case ended in error"
    return mem_size, logs


def plot_memory_usage(log_file_path: Path):
    mem_size, logs = read_log(log_file_path)
    max_height = 100_000
    curr_height = 1
    w_mem_uses: dict = defaultdict(int)
    r_mem_uses: dict = defaultdict(int)
    usage: list = []

    fig, ((o_ax, use_ax), (r_ax, _), (w_ax, _)) = plt.subplots(3, 2)

    o_ax.set(
        title="Ownership",
        xlabel="Memory",
        xlim=[0, mem_size],
        ylim=[0, 1],
    )
    r_ax.set(
        title="Reads",
        xlabel="Memory",
        xlim=[0, mem_size],
        ylim=[0, curr_height + 0.2],
    )
    w_ax.set(
        title="Writes",
        xlabel="Memory",
        xlim=[0, mem_size],
        ylim=[0, curr_height + 0.2],
    )

    o_ax.add_patch(
        patches.Rectangle(
            xy=(0, 0),  # point of origin.
            width=mem_size,
            height=max_height,
            linewidth=1,
            color="#f0f0f0",
            fill="#f0f0f0",
        )
    )

    def update(log):
        nonlocal curr_height
        nonlocal w_mem_uses
        nonlocal r_mem_uses
        nonlocal usage

        lt = log[0]
        val = log[1]
        mem_usage = usage[-1] if usage else 0

        if lt == "o":
            o_ax.add_patch(
                patches.Rectangle(
                    xy=(val[1], 0),  # point of origin.
                    width=val[2] - val[1] + 1,
                    height=1,
                    linewidth=1,
                    color=COLORS[val[0]] if val[0] != -1 else "#f0f0f0",
                )
            )
            mem_usage = mem_size - val[3]

        usage.append(mem_usage)
        use_ax.clear()
        use_ax.set(
            ylim=[0, mem_size + mem_size * 0.1],
            xlabel="Time",
            title=f"Used space {usage[-1] / mem_size:.2%}",
        )
        use_ax.grid()
        use_ax.plot(usage)

        if lt == "w":
            uses = w_mem_uses[val] = w_mem_uses[val] + 1
            w_ax.add_patch(
                patches.Rectangle(
                    xy=(val, uses - 1),  # point of origin.
                    width=1,
                    height=1,
                    linewidth=1,
                    color=RED,
                )
            )
            if uses > curr_height:
                curr_height = uses
                w_ax.set_ylim(0, curr_height + 0.2)

        if lt == "r":
            uses = r_mem_uses[val] = r_mem_uses[val] + 1
            r_ax.add_patch(
                patches.Rectangle(
                    xy=(val, uses - 1),  # point of origin.
                    width=1,
                    height=1,
                    linewidth=1,
                    color=GREEN,
                )
            )
            if uses > curr_height:
                curr_height = uses
                r_ax.set_ylim(0, curr_height + 0.2)

        return o_ax, w_ax, r_ax

    ani = animation.FuncAnimation(
        fig=fig, func=update, frames=logs, interval=100, repeat=False
    )
    plt.tight_layout()
    plt.show()

    # FFwriter = animation.FFMpegWriter(fps=10, extra_args=["-loop", "0"])
    # ani.save("animation.mp4", writer=FFwriter, dpi=150)


if len(sys.argv) < 2:
    print("Missing log file path argument")
    exit(1)

log_file_path = Path(sys.argv[1])
plot_memory_usage(log_file_path)

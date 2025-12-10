import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.patches import Polygon


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv_file", type=Path)
    args = parser.parse_args()
    with args.csv_file.open("r") as csv_io:
        reader = csv.reader(csv_io.read().split("\n"))
        points = list(
            map(lambda x: tuple(y for y in map(int, x)), filter(lambda x: x, reader))
        )
        mx = 0
        my = 0
        for x, y in points:
            mx = max(x, mx)
            my = max(my, y)

    # Create a figure and an axes object
    fig, ax = plt.subplots()

    # Create a Polygon patch
    polygon = Polygon(points, closed=True, edgecolor="blue", facecolor="lightblue")

    # Add the polygon to the axes
    ax.add_patch(polygon)

    # Set axis limits and aspect ratio
    ax.set_xlim(0, mx)
    ax.set_ylim(0, my)
    ax.set_aspect("equal", adjustable="box")

    # Display the plot
    plt.show()


if __name__ == "__main__":
    main()

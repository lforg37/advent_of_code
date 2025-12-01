from random import randint


def main():
    directions = ["L", "R"]

    def get_line():
        direction = directions[randint(0, 1)]
        steps = randint(1, 99)
        return f"{direction}{steps}\n"

    with open("input.txt", "w") as f:
        for i in range(16000000):
            f.write(get_line())


if __name__ == "__main__":
    main()

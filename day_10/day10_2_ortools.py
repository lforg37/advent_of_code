from ortools.linear_solver import pywraplp
from argparse import ArgumentParser
from pathlib import Path


class Machine:
    def __init__(self, initLine: str):
        rbracket_idx = initLine.index("]")
        self.nb_joltages = rbracket_idx - 1
        self.button_idx_for_joltage = {i: [] for i in range(self.nb_joltages)}
        free_button_idx = 0
        self.per_button = []
        for substr in initLine[rbracket_idx + 1 :].split(" "):
            if not substr:
                continue
            if substr[0] == "(":
                self.per_button.append(list(map(int, substr[1:-1].split(","))))
                for id in self.per_button[-1]:
                    self.button_idx_for_joltage[id].append(free_button_idx)
                free_button_idx += 1
            if substr[0] == "{":
                self.target_joltages = list(map(int, substr[1:-2].split(",")))

    def getMin(self) -> int:
        max_joltage = max(self.target_joltages)
        solver = pywraplp.Solver.CreateSolver("SAT")
        if not solver:
            raise RuntimeError("Could not create solver")
        buttons_vars = [
            solver.IntVar(0, max_joltage, f"button_{i}")
            for i in range(len(self.per_button))
        ]
        for joltage_id, joltage in enumerate(self.target_joltages):
            constraint = solver.RowConstraint(joltage, joltage, f"joltage_{joltage_id}")
            for button_idx in self.button_idx_for_joltage[joltage_id]:
                constraint.SetCoefficient(buttons_vars[button_idx], 1)
        objective = solver.Objective()
        for button_var in buttons_vars:
            objective.SetCoefficient(button_var, 1)
        objective.SetMinimization()
        status = solver.Solve()

        if status != pywraplp.Solver.OPTIMAL:
            raise RuntimeError("No optimal solution found")
        return int(solver.Objective().Value())


def main():
    parser = ArgumentParser()
    parser.add_argument("input_file", type=Path)
    args = parser.parse_args()
    sum = 0
    with args.input_file.open() as f:
        for line in f:
            machine = Machine(line)
            sum += machine.getMin()
    print(f"Sum of minimums: {sum}")


if __name__ == "__main__":
    main()

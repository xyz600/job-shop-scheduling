# -*- coding: utf-8 -*-

import sys
import os.path

def generate_problem(filepath: str, split: int):
    filename = os.path.basename(filepath)
    with open(filepath, 'r') as fin:
        lines = fin.readlines()
        problem_size = len(lines) // split
        for i in range(problem_size):
            with open(f"{filename[:-4]}_{i}.txt", 'w') as fout:
                fout.writelines(lines[i*split:(i+1)*split])


if __name__ == "__main__":
    filepath = sys.argv[1]
    job_size = int(sys.argv[2])
    split_size = job_size * 2 + 4
    generate_problem(filepath, split_size)
import sys, argparse, os
from mca2raw_core import MCA2RawCore

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("region")
    parser.add_argument("output")
    parser.add_argument("seed")
    args = parser.parse_args()

    core = MCA2RawCore()

    def print_table(lines):
        for l in lines:
            print(l)

    def progress(pct):
        print(pct, flush=True)

    core.export(args.region, args.output, args.seed, on_table=print_table, on_progress=progress)

if __name__ == "__main__":
    main()
import argparse
import csv

parser = argparse.ArgumentParser(description='Process time output for compiler')

parser.add_argument('log_file', type=str, help='input file')

args = parser.parse_args()
if __name__ == "__main__":
    lines = []
    with open(args.log_file, "r") as f:
        lines = f.readlines()

    i = 0
    units = []
    while i < len(lines):
        if "Building" in lines[i] or "Linking CXX executable" in lines[i]:
            unit = {}
            try:
                unit["work_type"] = "build" if "Building" in lines[i] else "linking"
                unit["work_type"] += ":" + lines[i].split(" ")[-1].rstrip()
                i += 2
                timeline = lines[i].rstrip().split("\t")
                # unit["sys_time"] = float(timeline[0])
                # unit["user_time"] = float(timeline[1])
                unit["total"] =  round(float(timeline[0]) + float(timeline[1]), 2)
            except IndexError:
                break
            except Exception:
                continue
            units.append(unit)

        i += 1
    csv_filename = args.log_file.split(".")[0] + ".csv"
    with open(csv_filename, 'w') as csvfile:
        fieldnames = ['work_type', 'total']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        # writer.writeheader()

        for u in units:
            writer.writerow(u)
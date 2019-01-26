import argparse
import csv
import os

parser = argparse.ArgumentParser(description='Process time output for compiler')

parser.add_argument('log_file', type=str, help='input file')

args = parser.parse_args()
if __name__ == "__main__":
    lines = []
    with open(args.log_file, "r") as f:
        lines = f.readlines()

    i = 0
    units = {}
    while i < len(lines):
        if "Building" in lines[i] or "Linking CXX executable" in lines[i]:
            unit = {}
            try:
                unit["work_type"] = "build" if "Building" in lines[i] else "linking"
                unit["work_type"] += ":" + lines[i].split(" ")[-1].rstrip()
                i += 2
                timeline = lines[i].rstrip().split("\t")
                unit["total"] =  round(float(timeline[0]) + float(timeline[1]), 2)
                units[unit["work_type"]] = unit["total"]
                with open('reports/' + os.path.basename(lines[i-2].split(" ")[-1].rstrip()) + '.csv', 'w') as csvfile:
                    fieldnames = unit.keys()
                    fieldvalues = unit.values()
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    writer.writeheader()
                    writer.writerow(unit)
            except IndexError:
                break
            except Exception:
                continue

        i += 1
    # csv_filename = args.log_file.split(".")[0] + ".csv"
    # with open(csv_filename, 'w') as csvfile:
    #     fieldnames = units.keys()
    #     fieldvalues = units.values()
    #     print(fieldvalues)
    #     writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    #     writer.writeheader()
        
    #     writer.writerow(units)
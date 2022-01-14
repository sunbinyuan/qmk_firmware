import subprocess
import os
import sys
import re
BOARDS = []

def main():
    for line in sys.stdin:
        if line.strip() != "":
            if re.match("^(gmmk)",line.strip()):
                BOARDS.append(line.strip()+"/rev2")
                BOARDS.append(line.strip()+"/rev3")
            if re.match("^(keychron/k)",line.strip()):
                BOARDS.append(line.strip())
                BOARDS.append(line.strip()+"/via")
                BOARDS.append(line.strip()+"/optical")
                BOARDS.append(line.strip()+"/optical_via")
            else: BOARDS.append(line.strip())

if __name__ == '__main__':
    main()

error = False
for kb in BOARDS:
    if subprocess.run(f"qmk compile -kb {kb} -km all -j{os.cpu_count()}", shell=True).returncode != 0:
        error = True
if error:
    sys.exit(1)

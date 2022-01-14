import subprocess
import os
import sys
BOARDS = []

def main():
    for line in sys.stdin:
        if line.strip() != "":
            BOARDS.append(line.strip())

if __name__ == '__main__':
    main()

error = False
for kb in BOARDS:
    if subprocess.run(f"qmk compile -kb {kb} -km all -j{os.cpu_count()}", shell=True).returncode != 0:
        error = True
if error:
    sys.exit(1)

import numpy as np
from subprocess import check_output
import sys

runlist = np.genfromtxt(sys.argv[1]).T[0].astype(int)

notfinished = []

for run in runlist:

    try:
        check_output(f"grep 'Delete the regular expression variables' logs/*{run}*", shell=True)
    except:
        notfinished.append(run)

print(f"{len(notfinished)}/200 ({len(notfinished)/2}%) still running.")
print(f"Runs still running: {notfinished}") 

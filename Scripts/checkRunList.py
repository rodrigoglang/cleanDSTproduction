import numpy as np
import os,sys
import uproot
from subprocess import check_output

runlist = np.genfromtxt(sys.argv[1]).T[0].astype(int)
running = 0
worked = 0
broken_other = 0
broken_time = 0
not_submitted = 0
broken_ROOT = 0
for run in runlist:

    if not os.path.exists(f"logs/Offrun_{run}_parameter_cleaningDST.log"):
        not_submitted+=1
        continue

    try:
        check_output(f"squeue -u rglang | grep {run}", shell=True)
        running+=1
        continue
    except:
        bla = 1

    try:
        check_output(f"grep 'Delete the regular expression variables' logs/*{run}*", shell=True)
        broken = True
    except:
        try:
            check_output(f"grep 'DUE TO TIME LIMIT' logs/*{run}*", shell=True)
            broken_time+=1
        except:
            broken_other+=1
        continue

    if not os.path.exists(f"Cleaned/Offrun_{run}.root"):
#        print(f"{run}: Non existing file! Check log: logs/Offrun_{run}_parameter_cleaningDST.log")
        broken_ROOT+=1
        continue

    try:
        file = uproot.open(f"Cleaned/Offrun_{run}.root")
    except:
        #print(f"{run}: Problem opening file Cleaned/Offrun_{run}.root, even though it exists!")
        broken_ROOT+=1
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TailcutsSTD_5/fKnown']
    except:
        #print(f"{run}: Tailcuts non present in file!")
        broken_ROOT+=1
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningPerformance3D_5/fKnown']
    except:
        #print(f"{run}: Time cleaning performance not present in file!")
        broken_ROOT+=1
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningDetection3D_5/fKnown']
    except:
        #print(f"{run}: Time cleaning detection 3D not present in file!")
        broken_ROOT+=1
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningDetection4D_5/fKnown']
    except:
        #print(f"{run}: Time cleaning detection 4D not present in file!")
        broken_ROOT+=1
        continue

    try:
        tailcuts = file['DST_tree']['IntensityData_Extended0407_5']
    except:
        #print(f"{run}: ImPACT intensity data not present in file!")
        broken_ROOT+=1
        continue

    worked+=1

print(f"List {sys.argv[1]}")
print(f"{not_submitted}/{len(runlist)} ({100*not_submitted/len(runlist):.2f}%) not yet submitted")
print(f"{running}/{len(runlist)} ({100*running/len(runlist):.2f}%) still running")
print(f"{worked}/{len(runlist)} ({100*worked/len(runlist):.2f}%) finished and worked fine")
print(f"{broken_time}/{len(runlist)} ({100*broken_time/len(runlist):.2f}%) broke due to time limit")
print(f"{broken_other}/{len(runlist)} ({100*broken_other/len(runlist):.2f}%) broke due to other reasons, please investigate!")
print(f"{broken_ROOT}/{len(runlist)} ({100*broken_ROOT/len(runlist):.2f}%) have good looking log files but broken ROOT files! Please investigate!")

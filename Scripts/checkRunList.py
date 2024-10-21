import numpy as np
import os,sys
import uproot

runlist = np.genfromtxt(sys.argv[1]).T[0].astype(int)

for run in runlist:

    if not os.path.exists(f"Cleaned/Offrun_{run}.root"):
        print(f"{run}: Non existing file! Check log: logs/Offrun_{run}_parameter_cleaningDST.log")
        continue

    try:
        file = uproot.open(f"Cleaned/Offrun_{run}.root")
    except:
        print(f"{run}: Problem opening file Cleaned/Offrun_{run}.root, even though it exists!")
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TailcutsSTD_5/fKnown']
    except:
        print(f"{run}: Tailcuts non present in file!")
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningPerformance3D_5/fKnown']
    except:
        print(f"{run}: Time cleaning performance not present in file!")
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningDetection3D_5/fKnown']
    except:
        print(f"{run}: Time cleaning detection 3D not present in file!")
        continue

    try:
        tailcuts = file['DST_tree']['HillasParameters_TimeCleaningDetection4D_5/fKnown']
    except:
        print(f"{run}: Time cleaning detection 4D not present in file!")
        continue

    try:
        tailcuts = file['DST_tree']['IntensityData_Extended0407_5']
    except:
        print(f"{run}: ImPACT intensity data not present in file!")
        continue


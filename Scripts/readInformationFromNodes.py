import numpy as np
from subprocess import check_output

runlist = np.array(np.genfromtxt('Lists/testCluster.lis').T[0]).astype(int)

cases = ['lfl00','lfl01','lfl02','lfl03','lfl04','lfl05','lfl06','lfl07','lfl08','lfl09',
                 'lfl10','lfl11','lfl12','lfl13','lfl14','lfl15','lfl16','lfl17','lfl18','lfl19']
ratio = {}
for case in cases:
    times = []
    times_CPU = []
    for run in runlist:
        try:
            time = float(check_output(f"grep 'Time to make' logs-testNodes/*{run}-{case}*", shell=True).decode('utf-8')[18:].split(" s")[0])
            time_CPU = float(check_output(f"grep 'Time to make' logs-testNodes/*{run}-{case}*", shell=True).decode('utf-8').split('(')[1].split(' s')[0])
            times.append(time)
            times_CPU.append(time_CPU)
        except:
            print(f"Error with file logs-testNodes/*{run}-{case}*")
    print(case, np.mean( np.array(times) / np.array(times_CPU) ) )

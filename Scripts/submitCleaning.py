import numpy as np
import os, sys
import hap_queue_tools as hap_queue
import random
import time

def getEnviromentalVariables():
    environmentVariables = {}
    environmentVariables['HESSROOT'] = os.getenv("HESSROOT")
    environmentVariables['HESSDST'] = os.getenv("HESSDST")
    environmentVariables['HESSDATA'] = os.getenv("HESSDATA")
    environmentVariables['PATH'] = os.getenv("PATH")
    environmentVariables['ROOTSYS']= os.getenv("ROOTSYS")
    environmentVariables['LIBRARY'] = os.getenv("LD_LIBRARY_PATH")

    return environmentVariables

def CheckFiles(PROJECTDIRECTORY):
    if (not os.path.exists(PROJECTDIRECTORY + '/temp_scripts/')):
        os.mkdir(PROJECTDIRECTORY + '/temp_scripts/')

    if (not os.path.exists(PROJECTDIRECTORY + '/logs/')):
        os.mkdir(PROJECTDIRECTORY + '/logs/')

def submit_job_ws(command, prefix, log_file, script_dir):
    import random
    import string
    import subprocess
    import os

    temp_script = script_dir + prefix + '.sh'
    with open(temp_script, 'w') as rsh:
        rsh.write('''\
#! /bin/bash
{}'''.format(command))

    job_id=submitSlurmJob(temp_script, log_file)

    return job_id

def submitSlurmJob(temp_script, log_file):
    import random
    import string
    import subprocess

    qsub_command = 'sbatch  '

    qsub_command += '-o {} {}'.format(log_file, temp_script)
    #os.system('chmod +x ' + temp_script)
    print('Running: ' + qsub_command)
    submit_output = subprocess.check_output(qsub_command, shell=True).decode("utf-8").split('\n')
    job_id = submit_output[-2].split(' ')[2]

    return job_id


def submitCleaning(run_list, outputpath, PROJECTDIRECTORY, nodes, mc_true=True):
    environmentVariables = getEnviromentalVariables()

    CheckFiles(PROJECTDIRECTORY)

    Script=PROJECTDIRECTORY+"CleanDSTs.C"

    jobs=[]

    nodes = nodes.split(",")

    for i in range(len(run_list)):
        command = "export PATH={}; export ROOTSYS={}; export LD_LIBRARY_PATH={};export HESSROOT={};".format(environmentVariables['PATH'], environmentVariables['ROOTSYS'], environmentVariables['LIBRARY'], environmentVariables['HESSROOT'])
        command +="export HESSDATA={}; export HESSDST={};".format(environmentVariables['HESSDATA'],environmentVariables['HESSDST'])

        command +="cd {};".format(PROJECTDIRECTORY)

        node = random.choice(nodes)

        if(mc_true=="True"):
            path_simfile, sim_file = os.path.split(run_list[i])
            sim_file = sim_file[:-10]

            outputname = outputpath+'/'+sim_file+'.root'
            command += """root -l -b -q 'CleanDSTs.C+("{}","{}",-1,-1)';""".format(run_list[i][:-1],outputname)
            logfile= sim_file+"_parameter_cleaningDST.log"
            ScriptName = sim_file+"_parameter_cleaning_submitScript"
        else:
            outputname = outputpath+'/Offrun_'+str(run_list[i][0])+ '.root'
            command += """root -l -b -q 'CleanDSTs.C+({},"{}",-1,-1)';""".format(run_list[i][0],outputname)
            logfile= 'Offrun_'+str(run_list[i][0])+ "_parameter_cleaningDST.log"
            ScriptName = 'Offrun_'+str(run_list[i][0])+ "_parameter_cleaning_submitScript"

        job_id = hap_queue.submit_to_queue_when_possible([command], f'{run_list[i][0]}-CleanDSTs', logfile="logs/"+logfile, max_queue_jobs=200, node=node)

        jobs.append(job_id)

        time.sleep(3)

    return (jobs)

##Main method happening here

PROJECTDIRECTORY = os.getcwd()

filelist=sys.argv[1]
outputpath = sys.argv[2]
mc_true = sys.argv[3]
node = sys.argv[4]
if(mc_true=="True"):
    print("Hello Jelena")
    with open(filelist,"r") as file:
        run_list = file.readlines()
    file.close()
else:
    run_list = []

    with open(filelist, 'r') as file:
        for line in file:
            # Split each line into values
            values = line.strip().split(' ')
            value1, value2 = map(int, values)

            # Append the values to the 2D list
            run_list.append([value1, value2])

jobs=submitCleaning(run_list, outputpath, PROJECTDIRECTORY, node, mc_true=mc_true)
hap_queue.wait_for_jobs_to_finish(jobs)


# cleanDSTproduction

Before starting:

- Compile HAP with the rodrigo-temporary-time-cleaning branch (make sure to do a clean before).
- Run `root -l -b -q 'CleanDSTs.C+(182099,"test.root",-1,10)'` to have the scripts compiled

For each list:

- Run `rm logs-testNodes/* | ./testNodes.sh` to check the best nodes. While it is running, do a quick squeue, if some nodes are shown as unavailable, don't even wait for them to be available, just `scancel` these jobs.
- Choose some 4-7 nodes (smaller numbers = better) and submit the jobs with: `nohup python Scripts/submitCleaning.py Lists/list-0.lis <your folder>/Cleaned false lfl02,lfl03,lfl04,lfl05,lfl10,lfl12,lfl13 &` (PUT YOUR CHOSEN NODES HERE!!)
- Once the list is finished, check the DSTs with: `python Scripts/checkRunList.py Lists/list-0.lis`

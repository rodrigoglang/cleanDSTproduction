#!/bin/bash
dir=$(pwd)
python Scripts/submitTestOfCluster.py Lists/testCluster.lis ${dir}/Cleaned-testNodes/ false lfl
python Scripts/readInformationFromNodes.py

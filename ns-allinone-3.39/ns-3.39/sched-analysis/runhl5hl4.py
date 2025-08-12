from traffic_conf import * 


import os
import time 


from dataclasses import dataclass
import numpy as np
import logging
import subprocess
import json




ns3Path = '/ns-allinone-3.39/ns-3.39'
ns3Scenario = 'p2p_scenarioReusable_taps'
ns3Scenarioconf = 'scratch/scen.json'
ns3Path = '/ns-allinone-3.39/ns-3.39'

band = 100
scs = 20

import numpy as np

# Type_of_combis = ["CU-plane", "C-plane"]
Type_of_combis = ["CU-plane"]

Weights = {0:"73 9 18"}
# Weights = {0:"23 72 5",1:"25 70 5"} # Mod weights

# Weights = {0:"23 73 4",1:"17 77 6"} # Original Weights - not sim with that _DONE IT_
Marking_Port = {0:"8080 46 9090 46 10800 8 10900 16 11000 24"}

# Marking_Port = {0:"8080 46 9090 46 10800 8 10900 16 11000 24", 1:"8080 8 9090 46 10800 8 10900 16 11000 24"}

# Link Capacity Swept Configuration
Mode = "PruebaTime"
Start = 5
Stop = 4.5
Step = -0.1
Cap_swept = np.arange(Start, Stop+Step, Step, dtype=float)  

SiteLimit  = 70e9
with open(f"./scratch/hl4hl5.json", "r") as jsonFileReceiver:
    data = json.load(jsonFileReceiver)
def main():
    for node in range(0,data["HL5"]):
       
        filename = f"{Mode}"
        # Change number of streams (json file)
        cmd = f"mkdir -p ./sim_results/{filename}"  # Use -p to avoid 'File exists' error
        os.system(cmd)

        data["FolderName"] = f"{filename}"
 
        user_pkt_size, user_rb, control_pkt_size, control_rb = traffic_def(100, 30, 1, 14, "BFP9", 16, 32)
        if ((user_rb+control_rb)*3)*1e9 > SiteLimit:
            cap = (SiteLimit/3)/(user_rb/control_rb + 1) 
        for sites in range(0,2):    
            for cell in range(0, data["Hl5Agreggration"][node]["Sites"][sites]["Cells"]):

                
                data["Hl5Agreggration"][node]["Sites"][sites]["CellFeatures"][cell]["URatenum"] = cap*(user_rb/control_rb)
                data["Hl5Agreggration"][node]["Sites"][sites]["CellFeatures"][cell]["UPacketSize"] = user_pkt_size
                data["Hl5Agreggration"][node]["Sites"][sites]["CellFeatures"][cell]["CRatenum"] = cap
                data["Hl5Agreggration"][node]["Sites"][sites]["CellFeatures"][cell]["CPacketSize"] = control_pkt_size

    with open("./scratch/hl4hl5ex.json", "w") as jsonFileReceiver:
        json.dump(data, jsonFileReceiver)

    cmd = "./ns3 run scratch/hl5hl4-setup.cc" 
    os.system(cmd)    
    
    # Save the json conf in the results folder
    os.system(f"cp ./scratch/hl4hl5ex.json ./sim_results/{filename}/")

    # os.system(f"tar -C ./sim_results -zcvf ./sim_results/{filename}.tar {filename}")
    # os.system(f"rm -r sim_results/{filename}/")

if __name__ == '__main__':
    main()

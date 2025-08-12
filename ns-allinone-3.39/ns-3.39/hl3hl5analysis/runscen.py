"""
This script allows the NS3 simulation to be run for different topologies where where the link length 
is varied and the propagation delay is taken into account to measure the one-way delay.

"""






import os
import time 


from dataclasses import dataclass
import numpy as np
import logging
import subprocess
import json




ns3Path = '/ns-allinone-3.39/ns-3.39'

ns3Scenarioconf = 'scratch/scen.json'
ns3Path = '/ns-allinone-3.39/ns-3.39'


import numpy as np


Seconds_sim = 0.5
NSIM = 1
DU_loc = ["hl3", "hl4"]
mtu = [8000, 1500]
prop = ["p"]
# Labels for the scenario when there is prop. delay
mod = ["avg", "worst"]

Prop_del = 5 # Propagation delay in us/km

# Avg distance and worst distance for the links
hl3hl4link = [27.5, 35] 
hl4hl5link = [17.5, 25]



Mode = "hl5-"
nsims = np.arange(1, NSIM+1, 1)

enabletracingtimestamps = False
enableSwitching = True
enable_model = False # Enable the model to be used in the simulation as a MM1 or MG1 model

def main():
    for du in DU_loc:
        for mtusize in mtu:
            for propflag in prop:       
                filename = f"{du}hl5-{mtusize}{propflag}"
                print(f"Running for  {du} {mtusize} {propflag}")
              
                if du == "hl3":
                    nnodes = 23
                else:
                    nnodes = 9
                
                for sim in nsims:
                    # if sim == 1:
                    #     enabletracingtimestamps = True
                    # else:
                    #     enabletracingtimestamps = False
                    print(f"NumSim {sim}")
                    if propflag == "p":
                        with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                            data = json.load(jsonFileReceiver)
                        for i , mod_i in enumerate(mod): 
                

                            cmd = f"mkdir -p ../sim_results/{filename}{mod_i}{sim}"  # Use -p to avoid 'File exists' error
                            os.system(cmd)
                            # data["Poisson"] = True
                            # data["Model"] = "mm1"
                            data["del-hl4hl5"] = Prop_del*hl4hl5link[i]
                            data["del-hl3hl4"] = Prop_del*hl3hl4link[i]
                            data["EnableTraceTimeStamps"] = enabletracingtimestamps
                            data["Seconds_sim"] = Seconds_sim
                            data["EnableSwitching"] = enableSwitching
                            data["netmtu"] = mtusize
                            data["FolderName"] = f"{filename}{mod_i}{sim}"
                            with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                                json.dump(data, jsonFileReceiver)

                            cmd = "../ns3 run scratch/hl3-hl5.cc" 
                            os.system(cmd)    
                            
                            # Compress folder
                            # os.system(f"tar -czvf ../sim_results/{filename}{mod_i}{sim}.tar.gz ../sim_results/{filename}{mod_i}{sim}")
                            # os.system(f"rm -rf ../sim_results/{filename}{mod_i}{sim}")
                    else:
                        print("Running without prop delay")
                        cmd = f"mkdir -p ../sim_results/{filename}{sim}"  # Use -p to avoid 'File exists' error
                        os.system(cmd)
                        with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                            data = json.load(jsonFileReceiver)
                        # data["Poisson"] = True
                        data["del-hl4hl5"] = 0
                        data["del-hl3hl4"] = 0
                        data["EnableTraceTimeStamps"] = enabletracingtimestamps
                        data["EnableSwitching"] = enableSwitching
                        data["netmtu"] = mtusize
                        data["Seconds_sim"] = Seconds_sim
                        data["FolderName"] = f"{filename}{sim}"
                        with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                            json.dump(data, jsonFileReceiver)

                        cmd = "../ns3 run scratch/hl3-hl5.cc" 
                        os.system(cmd)    
              
                        # Compress folder
                        # os.system(f"tar -czvf ../sim_results/{filename}{sim}.tar.gz ../sim_results/{filename}{sim}")
                        # os.system(f"rm -rf ../sim_results/{filename}{sim}")
          
if __name__ == '__main__':
    main()

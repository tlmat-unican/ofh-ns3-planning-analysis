"""
This script allows to run the NS3 simulation for the different scenarios where the bottleneck 
link is swept around a capacity for the HL3-HL5 scenario.

"""



import os
import time 


from dataclasses import dataclass
import numpy as np
import logging
import subprocess
import json
import numpy as np



ns3Path = '/ns-allinone-3.39/ns-3.39'

ns3Scenarioconf = 'scratch/scen.json'
ns3Path = '/ns-allinone-3.39/ns-3.39'




Seconds_sim = 0.5
model = "mm1"
Cswept = np.arange(3220, 3270, 50)
nnodes = 23
NSIM = 1
DU_loc = ["hl3"]
mtu = [64000]
prop = ["sp"]
mod = ["avg", "worst"]

Prop_del = 5  # Propagation delay in us/km

hl3hl4link = [27.5, 35]
hl4hl5link = [17.5, 25]


# Link Capacity Swept Configuration
Mode = "hl5-"
nsims = np.arange(1, NSIM+1, 1)


SiteLimit  = 70e9


Rho = 99




def main():
    for du in DU_loc:
        for mtusize in mtu:
            for C in Cswept:
                mainfolder = f"{C}"
                os.system(f"mkdir -p ../sim_results/{Rho}")
                C = C * 1e9
                c = C * (Rho/100) - nnodes*SiteLimit
                
                for propflag in prop:       
                    filename = f"{du}hl5-{mtusize}{propflag}"
                    
                    for sim in nsims:
                        
                        print(f"Running for {filename}{c}")
                        cmd = f"mkdir -p ../sim_results/{Rho}/{filename}{mainfolder}"  # Use -p to avoid 'File exists' error
                        os.system(cmd)
                        with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                            data = json.load(jsonFileReceiver)
                        data["LinkCap"] = mainfolder + "Gbps"
                        data["del-hl4hl5"] = 0
                        data["del-hl3hl4"] = 0
                        data["EnableHQoS"] = True
                        data["netmtu"] = mtusize
                        data["Seconds_sim"] = Seconds_sim
                        data["FolderName"] = f"{Rho}/{filename}{mainfolder}"
                        data["Poisson"] = True
                        data["Model"] = model
                        data["Backhaulenable"] = True
                        data["BHFeatures"][0]["Rate"] = f"{(c*0.23)/(1e9)}Gbps"
                        data["BHFeatures"][1]["Rate"] = f"{(c*0.26)/(1e9)}Gbps"
                        data["BHFeatures"][2]["Rate"] = f"{(c*0.51)/(1e9)}Gbps"
                        
                        with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                            json.dump(data, jsonFileReceiver)

                        cmd = "../ns3 run scratch/hl3-hl5.cc" 
                        os.system(cmd)    
                        # os.system(f"tar -czvf ../sim_results/{filename}{c}.tar.gz ../sim_results/{filename}{c}")
                        # os.system(f"rm -rf ../sim_results/{filename}{c}")

                  

          
if __name__ == '__main__':
    main()

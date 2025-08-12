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



ns3Path = '/ns-allinone-3.39/ns-3.39'

ns3Scenarioconf = 'scratch/scen.json'
ns3Path = '/ns-allinone-3.39/ns-3.39'


import numpy as np


C = 80e9
nnodes = 1
NSIM = 1
DU_loc = ["hl3"]
mtu = [8000]
prop = ["sp"]

Prop_del = 5

hl3hl4link = [27.5, 35]
hl4hl5link = [17.5, 25]

mod = ["avg", "worst"]
# Link Capacity Swept Configuration
Mode = "hl5-"
nsims = np.arange(1, NSIM+1, 1)
Seconds_sim = 1
SiteLimit  = 70e9
Budget = C - nnodes*SiteLimit

Cswept = np.arange(1, (Budget/1e9)-1, 1)


def main():
    for du in DU_loc:
        for mtusize in mtu:
            for c in Cswept:
                for propflag in prop:       
                    filename = f"{du}hl5-{mtusize}{propflag}"

                   
                    
                    for sim in nsims:
                        if propflag == "p":
                            with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                                data = json.load(jsonFileReceiver)
                            for i , mod_i in enumerate(mod): 
                    
                                
                                cmd = f"mkdir -p ../sim_results/{filename}{mod_i}{sim}"  # Use -p to avoid 'File exists' error
                                os.system(cmd)
                                data["del-hl4hl5"] = Prop_del*hl4hl5link[i]
                                data["del-hl3hl4"] = Prop_del*hl3hl4link[i]
                                data["Seconds_sim"] = Seconds_sim
                                data["netmtu"] = mtusize
                                data["FolderName"] = f"{filename}{mod_i}{sim}"
                                with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                                    json.dump(data, jsonFileReceiver)

                                cmd = "../ns3 run scratch/hl3-hl5.cc" 
                                os.system(cmd)    
                                
                                # Save the json conf in the results folder
                                os.system(f"cp ./scratch/hl3-hl5ex.json ./sim_results/{filename}/")
                        else:
                            print(f"Running for {filename}{c}")
                            cmd = f"mkdir -p ../sim_results/{filename}{c}"  # Use -p to avoid 'File exists' error
                            os.system(cmd)
                            with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                                data = json.load(jsonFileReceiver)
                            data["del-hl4hl5"] = 0
                            data["del-hl3hl4"] = 0
                            data["netmtu"] = mtusize
                            data["EnableHQoS"] = True
                            data["Seconds_sim"] = Seconds_sim
                            data["FolderName"] = f"{filename}{c}"
                            data["Backhaulenable"] = True
                            data["EnableModel"] = True
                            data["Enableswitching"] = False
                            data["Slices"] = 3
                            data["Poisson"] = True
                            data["BHFeatures"][0]["Rate"] = f"{(c*0.23)}Gbps"
                            data["BHFeatures"][1]["Rate"] = f"{(c*0.26)}Gbps"
                            data["BHFeatures"][2]["Rate"] = f"{(c*0.51)}Gbps"
                            
                            with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                                json.dump(data, jsonFileReceiver)

                            cmd = "../ns3 run scratch/hl3-hl5theo.cc" 
                            os.system(cmd)    
                            # os.system(f"tar -czvf ../sim_results/{filename}{c}.tar.gz ../sim_results/{filename}{c}")

          
if __name__ == '__main__':
    main()

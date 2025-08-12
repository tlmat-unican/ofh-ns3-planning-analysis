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




Seconds_sim = 0.001
Poisson  = True
model = "mg1"
C = 1700e9
nnodes = 23
DU_loc = ["hl3"]
mtu = [8000]
prop = ["sp"]



Name = ["FHWRR2", "FHWRR3"]
MapQueue = ["8 0 16 1", "8 0 16 1 24 2"]
Weights = ["1000 1", "1000 500 1"]
Marking_Port = [ "8080 46 11000 8 12000 8 13000 16", "8080 46 11000 8 12000 16 13000 24"]



MapQueue = ["8 0 16 1"]
Weights = ["1000 1"]
Marking_Port = ["8080 46 11000 46 12000 8 13000 16"]
# Link Capacity Swept Configuration
Mode = "hl5-"


SiteLimit  = 70e9
Budget = C - nnodes*SiteLimit

BHswept = np.arange(5, (Budget/1e9)-5, 2.5)


def main():
    for du in DU_loc:
        for mtusize in mtu:
            for name, mapqueue, weight, marking_port in zip(Name, MapQueue, Weights, Marking_Port):
                # if mapqueue == MapQueue[0] and weight == Weights[0] and marking_port == Marking_Port[0]:
                #     continue
                print(f"mapqueue: {mapqueue}, weight: {weight}, marking_port: {marking_port}")
                # break
                for c in BHswept:
                    for propflag in prop:       
                        filename = f"{du}hl5-{mtusize}{propflag}{name}"

                        if du == "hl3":
                            nnodes = 23
                        else:
                            nnodes = 9
                        
                    
                        print(f"Running for {filename}{c}")
                        cmd = f"mkdir -p ../sim_results/{filename}{c}"  # Use -p to avoid 'File exists' error
                        os.system(cmd)
                        with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                            data = json.load(jsonFileReceiver)
                        data["del-hl4hl5"] = 0
                        data["del-hl3hl4"] = 0
                        data["EnableModel"] = True 
                        data["MapQueue"] = mapqueue
                        data["Weights"] = weight
                        data["Marking_Port"] = marking_port
                        data["EnableHQoS"] = True
                        data["netmtu"] = mtusize
                        data["Seconds_sim"] = Seconds_sim
                        data["FolderName"] = f"{filename}{c}"
                        data["Poisson"] = Poisson
                        data["Model"] = model
                        data["Backhaulenable"] = True
                        data["BHFeatures"][0]["Rate"] = f"{(c*1e9*0.23)/(1e6)}Mbps"
                        data["BHFeatures"][1]["Rate"] = f"{(c*1e9*0.26)/(1e6)}Mbps"
                        data["BHFeatures"][2]["Rate"] = f"{(c*1e9*0.51)/(1e6)}Mbps"
                        
                        with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                            json.dump(data, jsonFileReceiver)

                        cmd = "../ns3 run scratch/hl3-hl5theo.cc" 
                        os.system(cmd)    
                      
                
       
                    # os.system(f"tar -czvf ../sim_results/{filename}{c}.tar.gz ../sim_results/{filename}{c}")
                    # os.system(f"rm -rf ../sim_results/{filename}{c}")


          
if __name__ == '__main__':
    main()

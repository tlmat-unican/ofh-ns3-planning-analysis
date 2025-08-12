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


Seconds_sim = 0.1
SiteLimit  = 70e9
SatLevels = [98, 98.5, 99, 99.5]
Poisson  = True
model = "mg1"
C = 1700e9
numsites = [23]
DU_loc = ["hl3"]
mtu = [8000]
prop = ["sp"]
Enableswitching = False

# Labels for the scenario when there is prop. delay
# mod = ["avg", "worst"]

# Prop_del = 5 # Propagation delay in us/km

# # Avg distance and worst distance for the links
# hl3hl4link = [27.5, 35] 
# hl4hl5link = [17.5, 25]



# Labels for the scenario when there is prop. delay
mod = ["worst"]

Prop_del = 5 # Propagation delay in us/km

# Avg distance and worst distance for the links
hl3hl4link = [0] 
hl4hl5link = [0]




# Name = ["FH", "FHWRR3"]
# MapQueue = ["8 0 16 1", "8 0 16 1 24 2"]
# Weights = ["1000 1", "1000 300 1"]
# Marking_Port = ["8080 46 11000 46 12000 8 13000 16", "8080 46 11000 8 12000 16 13000 24"]

Name = ["FHWRR3"]
MapQueue = ["8 0 16 1 24 2"]
Weights = ["1000 300 1"]
Marking_Port = ["8080 46 11000 8 12000 16 13000 24"]


# Name = ["FHWRR3"]
# MapQueue = ["8 0 16 1 24 2"]
# Weights = ["1000 300 1"]
# Marking_Port = ["8080 46 11000 8 12000 16 13000 24"]
Mode = "hl5-"






def main():
    for du in DU_loc:
        for nnodes in numsites:
            for mtusize in mtu:
                for name, mapqueue, weight, marking_port in zip(Name, MapQueue, Weights, Marking_Port):
                    # if mapqueue == MapQueue[0] and weight == Weights[0] and marking_port == Marking_Port[0]:
                    #     continue
                    print(f"mapqueue: {mapqueue}, weight: {weight}, marking_port: {marking_port}")
                    # break
                    BHswept = C*1e-9*(np.array(SatLevels)/100) - nnodes*SiteLimit*(1e-9)
                    for c in BHswept:
                        print(f"C: {c}")
                        for propflag in prop:       
                            filename = f"{du}hl5-{mtusize}{propflag}{name}N{nnodes}C"
                            if propflag == "p":
                                for modflag, hl3hl4, hl4hl5 in zip(mod,hl3hl4link, hl4hl5link):
                                    filename += modflag 
                                    print(f"Running for {filename}{c}")
                                    cmd = f"mkdir -p ../sim_results/{filename}{c}"  # Use -p to avoid 'File exists' error
                                    os.system(cmd)
                                    with open(f"../scratch/hl3-hl5-{nnodes}.json", "r") as jsonFileReceiver:
                                        data = json.load(jsonFileReceiver)
                    
                            #         data["del-hl4hl5"] = Prop_del*hl4hl5
                            #         data["del-hl3hl4"] = Prop_del*hl3hl4
                            #         data["EnableSwitching"] = Enableswitching
                            #         data["EnableModel"] = True 
                            #         data["MapQueue"] = mapqueue
                            #         data["Weights"] = weight
                            #         data["Marking_Port"] = marking_port
                            #         data["EnableHQoS"] = True
                            #         data["netmtu"] = mtusize
                            #         data["Seconds_sim"] = Seconds_sim
                            #         data["FolderName"] = f"{filename}{c}"
                            #         data["Poisson"] = Poisson
                            #         data["Model"] = model
                            #         data["Backhaulenable"] = True
                            #         data["BHFeatures"][0]["Rate"] = f"{(c*1e9*0.23)/(1e6)}Mbps"
                            #         data["BHFeatures"][1]["Rate"] = f"{(c*1e9*0.26)/(1e6)}Mbps"
                            #         data["BHFeatures"][2]["Rate"] = f"{(c*1e9*0.51)/(1e6)}Mbps"
                                    
                            #         with open("../scratch/hl3-hl5ex.json", "w") as jsonFileReceiver:
                            #             json.dump(data, jsonFileReceiver)

                            #         cmd = "../ns3 run scratch/hl3-hl5theo.cc" 
                            #         os.system(cmd)    
                                    
                            #         # os.system(f"tar -czvf ../sim_results/{filename}{c}.tar.gz ../sim_results/{filename}{c}")
                            #         # os.system(f"rm -rf ../sim_results/{filename}{c}")
                            else:
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
                                data["Slices"] = 3
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

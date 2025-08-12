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







Seconds_sim = 1
Poisson  = True
model = "mg1"
C = 1700e9
nnodes = 23
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
mod = [ "worst"]

Prop_del = 5 # Propagation delay in us/km

# Avg distance and worst distance for the links
hl3hl4link = [ 35] 
hl4hl5link = [ 25]




Name = ["FHcbr"]
MapQueue = ["8 0"]
Weights = ["1"]
Marking_Port = ["8080 46 11000 8 12000 8 13000 8"]


# MapQueue = ["8 0"]
# Weights = ["1"]
# Marking_Port = ["8080 46 11000 8 12000 8 13000 8"]

# Link Capacity Swept Configuration
Mode = "hl5-"


SiteLimit  = 70e9
Budget = C - nnodes*SiteLimit

BHswept = np.arange(25, (Budget/1e9)-5, 2.5)


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
                            with open(f"../scratch/hl3-hl5-{nnodes}test.json", "r") as jsonFileReceiver:
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

                            cmd = "../ns3 run scratch/hl3-hl5theocombi.cc" 
                            os.system(cmd) 
                 
                            os.system(f"tar -czvf ../sim_results/{filename}{c}.tar.gz ../sim_results/{filename}{c}")
                            os.system(f"rm -rf ../sim_results/{filename}{c}")

                  

          
if __name__ == '__main__':
    main()

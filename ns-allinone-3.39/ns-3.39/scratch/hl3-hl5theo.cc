
    /*
    * This is a basic example that compares CoDel and PfifoFast queues using a simple, single-flow
    * topology:
    *
    * source_{1-N} --------------------------Router_marking--------------------------QoS_router------------------------DU_sink
    *                   inf Mb/s, 0 ms                          inf Mb/s, 0 ms                       5 Mb/s, 5ms
    *                                                                                                 bottleneck
    *
    * The source generates traffic across the network using BulkSendApplication.
    * The default TCP version in ns-3, TcpNewReno, is used as the transport-layer protocol.
    * Packets transmitted during a simulation run are captured into a .pcap file, and
    * congestion window values are also traced.
    */

  
    #include "hl3-hl5.h"


    using namespace ns3;
    using json = nlohmann::json;
    
    NS_LOG_COMPONENT_DEFINE("hl3-hl5");

   
    int
    main(int argc, char* argv[])
    {
        RngSeedManager::SetSeed(time(NULL));
        Time::SetResolution(Time::PS);
        std::string JSONpath = "./scratch/hl3-hl5ex.json";
        std::ifstream f(JSONpath);
        json data = json::parse(f);
        uint32_t netMTU = data["netmtu"]; 
        bool enabletracing = true, enablepcap = false, enablehqos = data.at("EnableHQoS"), enableswitching = data.at("Enableswitching");
        bool enabletraceTimeStamps = data.at("EnableTraceTimeStamps");
        bool enablemodel = data.at("EnableModel");
        std::cout << GREEN << "Reading conf. file" << RESET << std::endl;
        std::string simFolder = data.at("FolderName");
        std::string resultsPathname = "./sim_results/" + simFolder + "/";
        // File.open("./sim_results/"+simFolder+"/switching.log",  std::fstream::out);

        int dedicatedlink = 100000000000;

        int hl3nodes = 2;
        int hl4nodes = 2;
        int hl5nodes = data["Hl5Agreggration"].size();

        std::cout << "N hl5nodes: " << hl5nodes << std::endl;   
        std::vector<bool> links = {false, false, false, false, false, false, false,  false, false, false, false, 
                                    false, false,  false, false, false,  false, false, false, false, false,  false, false, false,  
                                    false, false, false, false, false, false, false, false, false,  false};

       
      
        int DU = 1, bh = 2; 
        int sitesperhl5node = 1 , bhnodes = 0;

        int Routers = hl3nodes + hl4nodes + hl5nodes + DU + bh + sitesperhl5node* hl5nodes;
   
        
        std::vector<std::unique_ptr<RxTracerHelper>> rxTracersRUSite1;
        std::vector<std::unique_ptr<TxTracerHelper>> txTracersRUSite1;

    
        std::vector<std::unique_ptr<RxTracerHelper>> rxTracers1;
        std::vector<std::unique_ptr<TxTracerHelper>> txTracers1;
        




        int n_user_call_site1 = 0;
        int n_user_call_site2 = 0;
        int bhtraffic = 0;
        int initnode =  hl4nodes + hl5nodes + DU;
        for (int i = 0; i < hl5nodes; i++){
            for (int j = 0; j < data["Hl5Agreggration"][i]["Sites"].size(); j++) {
                    for (int z = 0; z < data["Hl5Agreggration"][i]["Sites"][j]["Cells"]; z++){
                        if(j == 0){
                            // std::cout << "HL5 node: " << i <<  " Site: " << j << " Cell: " << z <<  " tracefileindex: " << n_user_call_site1<< std::endl;  
                            bool CPlaneflag = data["Hl5Agreggration"][i]["Sites"][j]["CellFeatures"][z]["CUPlane"];
                           
                            txTracersRUSite1.push_back(std::make_unique<TxTracerHelper>("RU1", n_user_call_site1, resultsPathname, CPlaneflag)); // the split of the traffic is done by the port number 
                            rxTracersRUSite1.push_back(std::make_unique<RxTracerHelper>("Site1User" + std::to_string(n_user_call_site1), resultsPathname));

                            if (data["Hl5Agreggration"][i]["Sites"][j]["CellFeatures"][z]["CUPlane"]){
                                rxTracersRUSite1.push_back(std::make_unique<RxTracerHelper>("Site1Control" + std::to_string(n_user_call_site1), resultsPathname)); 
                            }
            
                            n_user_call_site1++; 

                        }
                    }
            }
          
        }


        if (data.contains("Backhaulenable") && data["Backhaulenable"]){
            for (int j = 0; j < data["BHFeatures"].size(); j++) {
                    for (int z = 0; z < data["FlowsPerSlice"]; z++){
                        // std::cout << "HL5 node: " << i << " SLices: " << j << " Flows: " << z << std::endl;
                        txTracers1.push_back(std::make_unique<TxTracerHelper>("BH1", bhtraffic, resultsPathname));
                        rxTracers1.push_back(std::make_unique<RxTracerHelper>("BH1_"+std::to_string(bhtraffic), resultsPathname));
                        
                        bhtraffic++;
                    }
            }

        }

        /*****************    Time - Callbacks         *****************/
        std::vector<std::unique_ptr<SnifferHelper>> TprocServhl5nodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> TprocWaithl5nodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> Twaithl5nodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> Tshl5nodesCallbacks;

        std::vector<std::unique_ptr<SnifferHelper>> TprocServhlxnodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> TprocWaithlxnodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> TwaithlxnodesCallbacks;
        std::vector<std::unique_ptr<SnifferHelper>> TshlxnodesCallbacks;
        if (enabletraceTimeStamps){
            for (int i = 0; i < hl5nodes; i++){
                TprocServhl5nodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl5node" + std::to_string(i) + "TprocServ") );
                TprocWaithl5nodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl5node" + std::to_string(i) + "TprocWait") );
                Twaithl5nodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl5node" + std::to_string(i) + "Twait") );
                Tshl5nodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl5node" + std::to_string(i) + "Ts") );
            }



       

            if (data["hl4level"]){
                std::cout << "HL4 - HL5 level" << std::endl;

                
                TwaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "DUnode" + std::to_string(0) + "Twait") );
                TshlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "DUnode" + std::to_string(0) + "Ts") );



                TprocServhlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "TprocServ") );
                TprocWaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "TprocWait") );
                TwaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "Twait") );
                TshlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "Ts") );


            }else{

                std::cout << "HL3 - HL5 level" << std::endl;

                TwaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "DUnode" + std::to_string(0) + "Twait") );
                TshlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "DUnode" + std::to_string(0) + "Ts") );

                TprocServhlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl3node" + std::to_string(0) + "TprocServ") );
                TprocWaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl3node" + std::to_string(0) + "TprocWait") );
                TwaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl3node" + std::to_string(0) + "Twait") );
                TshlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl3node" + std::to_string(0) + "Ts") );

                TprocServhlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "TprocServ") );
                TprocWaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "TprocWait") );
                TwaithlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "Twait") );
                TshlxnodesCallbacks.push_back(std::make_unique<SnifferHelper>(resultsPathname + "hl4node" + std::to_string(0) + "Ts") );

            }
        }
      

        


   
        
        NodeContainer nodes;
        nodes.Create(Routers);

    
        /************************************************
        ************ Creating links features ************
        *************************************************/
        std::cout << YELLOW << "Creating topology with " << Routers << " nodes" << RESET << std::endl;
        PointToPointHelper hl3hl4p2p;
        hl3hl4p2p.SetDeviceAttribute("DataRate", StringValue(data["LinkCap"]));
        hl3hl4p2p.SetChannelAttribute("Delay", StringValue(to_string(data["del-hl3hl4"]) + "us"));
        hl3hl4p2p.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        hl3hl4p2p.SetDeviceAttribute("EnableSwithcingTime", BooleanValue(enableswitching));
        hl3hl4p2p.SetDeviceAttribute("SwitchingCapacity", StringValue("2000Gbps"));
        hl3hl4p2p.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));
        

        PointToPointHelper hl3hl4p2p_tc;
        hl3hl4p2p_tc.SetDeviceAttribute("DataRate", StringValue(data["LinkCap"]));
        hl3hl4p2p_tc.SetChannelAttribute("Delay", StringValue(to_string(data["del-hl3hl4"]) + "us"));
        hl3hl4p2p_tc.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        hl3hl4p2p_tc.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));
        // hl3hl4p2p_tc.SetDeviceAttribute("EnableSwithcingTime", BooleanValue(false));
        // hl3hl4p2p_tc.SetDeviceAttribute("SwitchingCapacity", StringValue("1700Gbps"));
        hl3hl4p2p_tc.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p")); // 1 packet queue size in case of traffic control is used

        PointToPointHelper hl4hl5p2p;
        hl4hl5p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(dedicatedlink)+ "Gbps"));
        hl4hl5p2p.SetChannelAttribute("Delay", StringValue(to_string(data["del-hl4hl5"]) + "us"));
        hl4hl5p2p.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        hl4hl5p2p.SetDeviceAttribute("EnableSwithcingTime", BooleanValue(enableswitching));
        hl4hl5p2p.SetDeviceAttribute("SwitchingCapacity", StringValue("100Gbps"));
        hl4hl5p2p.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));

        PointToPointHelper  p2p;  // to connect the access sites as well as the BH nodes
        p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(dedicatedlink)+ "Gbps"));
        p2p.SetChannelAttribute("Delay", StringValue("0ms"));
        p2p.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        p2p.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));
     

        PointToPointHelper DUp2p;
        DUp2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(dedicatedlink)+ "Gbps"));
        DUp2p.SetChannelAttribute("Delay", StringValue("0ms"));
        DUp2p.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        DUp2p.SetDeviceAttribute("EnableSwithcingTime", BooleanValue(enableswitching));
        DUp2p.SetDeviceAttribute("SwitchingCapacity", StringValue("2000Gbps"));
        DUp2p.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));


        PointToPointHelper BHp2p;
        BHp2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(dedicatedlink)+ "Gbps"));
        BHp2p.SetChannelAttribute("Delay", StringValue("0ms"));
        BHp2p.SetDeviceAttribute("Mtu", UintegerValue(netMTU));
        BHp2p.SetDeviceAttribute("EnableModel", BooleanValue(enablemodel));


        // HL3-Hl4 level
        NetDeviceContainer HL3_01 = hl3hl4p2p.Install(NodeContainer{nodes.Get(0), nodes.Get(1)});
        NetDeviceContainer HL3_02;
        if (enablehqos){
            std::cout << "Traffic control enabled" << std::endl;
            HL3_02 = hl3hl4p2p_tc.Install(NodeContainer{nodes.Get(0), nodes.Get(2)});
        }else{
            HL3_02 = hl3hl4p2p.Install(NodeContainer{nodes.Get(0), nodes.Get(2)});
        }


        NetDeviceContainer HL3_13 = hl3hl4p2p.Install(NodeContainer{nodes.Get(1), nodes.Get(3)});
        // NetDeviceContainer HL3_12 = hl3hl4p2p.Install(NodeContainer{nodes.Get(1), nodes.Get(2)});



        // HL4-HL5 level 
        std::vector<NetDeviceContainer> HL4_2;
        for (int i = 0; i < hl5nodes; i++){
            if (links[i]){
                HL4_2.push_back(hl4hl5p2p.Install(NodeContainer{nodes.Get(3), nodes.Get(4+i)}));
            }else{
                HL4_2.push_back(hl4hl5p2p.Install(NodeContainer{nodes.Get(2), nodes.Get(4+i)})); // Path for link up
            }
            
        }

      
    
        std::vector<NetDeviceContainer> BHNodes;


        int FHnodes = hl3nodes + hl4nodes + hl5nodes, i = 0;
      

        std::vector<NetDeviceContainer> AccessSites;
        // Sites for FH traffic
        for (int node = hl3nodes + hl4nodes; node < FHnodes; node++) {
            for (int nsite = 0; nsite < sitesperhl5node; nsite++){ // In case of multiple sites per node
   
                // std::cout << "HL5 node: " << node << " " << " || Site: " << nsite << " NodeperSite: " << FHnodes+i << std::endl;
                AccessSites.push_back(p2p.Install(NodeContainer{nodes.Get(node), nodes.Get(FHnodes+i)}));
                i++;
            } 
        } 
        
        // Sites for BH traffic
        int lastFHnode = FHnodes  + hl5nodes*sitesperhl5node;
        i=0;
        for (int node = hl3nodes + hl4nodes; node < FHnodes; node++) {
            for (int nsite = 0; nsite < bhnodes; nsite++){
                i++;
                std::cout << "HL5 node: " << node << " " << " || Site: " << nsite << " NodeperBH: " << lastFHnode+i << std::endl;
                AccessSites.push_back(p2p.Install(NodeContainer{nodes.Get(node), nodes.Get(lastFHnode+i)}));
            } 
        } 

        int totnodes = lastFHnode;
        std::cout << "Total nodes: " << totnodes << std::endl;
        NetDeviceContainer DU0, BH0, BH1;
        if (data["hl4level"]){
            std::cout << "DU at level HL4" << std::endl;
            DU0 = DUp2p.Install(NodeContainer{nodes.Get(totnodes), nodes.Get(2)});
            
            BH0 = BHp2p.Install(NodeContainer{nodes.Get(totnodes+1), nodes.Get(0)});
            BH1 = BHp2p.Install(NodeContainer{nodes.Get(totnodes+2), nodes.Get(2)});
        }else{
            std::cout << "DU at level HL3" << std::endl;
            std::cout << totnodes  <<  " " << 0 << std::endl;
            DU0 = DUp2p.Install(NodeContainer{nodes.Get(totnodes), nodes.Get(0)});
            
            BH0 = BHp2p.Install(NodeContainer{nodes.Get(totnodes+1), nodes.Get(0)});
            BH1 = BHp2p.Install(NodeContainer{nodes.Get(totnodes+2), nodes.Get(2)});
        }
       

        /************************************************
        ******************* IP stack ********************
        *************************************************/
        std::cout << YELLOW << "Installing IP stack" << RESET << std::endl;
        InternetStackHelper stack;
        for (int i = 0; i < Routers; i++)
        {
            stack.Install(nodes.Get(i));
        }


        /************************************************
        *************** TCL - policies ******************
        *************************************************/

        if(enablehqos){

            //// MARKING 
            TrafficControlHelper tch1;
            tch1.SetRootQueueDisc("ns3::MarkerQueueDisc", "MarkingQueue", StringValue(data.at("Marking_Port")));
            // tch1.Install(R0R4.Get(0));
            // for (uint32_t accesssites = 0; accesssites < AccessSites.size(); accesssites++){
            //     tch1.Install(AccessSites.at(accesssites));
            // }
            tch1.Install(DU0.Get(0));
            // tch1.Install(DU0.Get(1));
            tch1.Install(BH0.Get(0));
    

            //// Policies
            TrafficControlHelper tch2;
            // Set up the root queue disc with PrioQueueDisc
            uint16_t rootHandle = tch2.SetRootQueueDisc("ns3::PrioQueueDscpDisc");
            // Get ClassIdList for the second-level queues
            TrafficControlHelper::ClassIdList cid = tch2.AddQueueDiscClasses(rootHandle, 2, "ns3::QueueDiscClass");
            // tch2.AddChildQueueDisc(rootHandle, cid[0], "ns3::FifoQueueDisc", "MaxSize", StringValue("100p"));
            tch2.AddChildQueueDisc(rootHandle, cid[0], "ns3::FifoQueueDisc");
            // tch2.AddChildQueueDisc(rootHandle, cid[1], "ns3::FifoQueueDisc");
            tch2.AddChildQueueDisc(rootHandle, cid[1], "ns3::WrrQueueDisc", "Quantum", StringValue(data.at("Weights")), "MapQueue", StringValue(data.at("MapQueue")));





            // tch2.Install(BH0.Get(0));
            tch2.Install(HL3_02.Get(0));
            // tch2.Install(HL3_02.Get(1));
            // tch2.Install(BH1.Get(0));
            tch2.Install(BH1.Get(1));


       
   
        }
        


        /*******************************************************************
        ************** Creating networks from spine leaf nodes *************
        ********************************************************************/
        Ipv4InterfaceContainer HL3;
        Ipv4AddressHelper HL3_address;
   
        HL3_address.SetBase("10.0.1.0", "255.255.255.252");
        HL3.Add(HL3_address.Assign(HL3_01)); 
        HL3_address.SetBase("10.0.2.0", "255.255.255.252");
        HL3.Add(HL3_address.Assign(HL3_02));
        HL3_address.SetBase("10.0.3.0", "255.255.255.252");
        // HL3.Add(HL3_address.Assign(HL3_03));

        HL3_address.SetBase("10.0.13.0", "255.255.255.252");
        HL3.Add(HL3_address.Assign(HL3_13));
        HL3_address.SetBase("10.0.12.0", "255.255.255.252");
        // HL3.Add(HL3_address.Assign(HL3_12));
    
  



        std::cout << YELLOW << "Creating networks" << RESET << std::endl;
        Ipv4InterfaceContainer HL4HL5_0, HL4HL5_1;
        Ipv4AddressHelper HL4HL5_address;

        for (int i = 0; i < HL4_2.size(); i++){
            std::string str = "10.0.2" + std::to_string(4+i) + ".0";
            const char* adr = str.c_str();
            HL4HL5_address.SetBase(adr, "255.255.255.252");
            HL4HL5_0.Add(HL4HL5_address.Assign(HL4_2.at(i)));
        }


        /*******************************************************************
        ********************** Sites node configuration ********************
        ********************************************************************/
        Ipv4InterfaceContainer RUsAccess;
        Ipv4AddressHelper Ruaccess;
        for (uint32_t sites = 0; sites < AccessSites.size(); sites++){
            std::string str = "128.192."+ std::to_string(sites+1) +".0";
            // std::cout << str << std::endl;
            const char* adr = str.c_str();
            Ruaccess.SetBase(adr,"255.255.255.0");
            RUsAccess.Add(Ruaccess.Assign(AccessSites.at(sites)));
        }

        /************************************************************
        ********************** DU configuration ********************
        *************************************************************/
        Ipv4InterfaceContainer DUAccess;
        Ipv4AddressHelper Duaccess;
        Duaccess.SetBase("10.0.77.0","255.255.255.252");
        DUAccess.Add(Duaccess.Assign(DU0));

        Ipv4InterfaceContainer BHAccess;
        Ipv4AddressHelper bhaccess;
        bhaccess.SetBase("10.0.88.0", "255.255.255.252");
        BHAccess.Add(bhaccess.Assign(BH0));
        bhaccess.SetBase("10.0.99.0", "255.255.255.252");   
        BHAccess.Add(bhaccess.Assign(BH1)); 
        /*********************************************************************
        ********************** Populate Routing Tables ***********************
        **********************************************************************/
        std::cout << YELLOW << "Populating routing tables" << RESET << std::endl;
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();


       /*******************************************************************
        ********************** RU node configuration ***********************
        ********************************************************************/
       ApplicationContainer Client_appRU, Server_appRU, Client_appBH, Server_appBH;
       if (data.at("Poisson")){
            std::cout << "Poisson traffic" << std::endl;    
            configureAppPoisson(nodes, FHnodes, lastFHnode, data, DUAccess, RUsAccess,BHAccess, totnodes,
            Client_appRU, Server_appRU, Client_appBH, Server_appBH, links );
    
       }else{
            configureApplications(nodes, FHnodes, lastFHnode, data, DUAccess, RUsAccess,BHAccess, totnodes,
            Client_appRU, Server_appRU, Client_appBH, Server_appBH, links );
       }


   
        Server_appRU.Start(Seconds(0));
        Client_appRU.Start(Seconds(0));  

        Server_appBH.Start(Seconds(0));
        Client_appBH.Start(Seconds(0));  
        std::cout << GREEN << "Config. has finished" << RESET << std::endl;

        if (enabletracing){
            if (data.at("Poisson")){
                tracerloggingPoissonApps(hl5nodes, rxTracersRUSite1, txTracersRUSite1, 
                txTracers1,  rxTracers1, data,  FHnodes,  totnodes,  lastFHnode, links);
            }else{
                tracerlogging(hl5nodes, rxTracersRUSite1, txTracersRUSite1, 
                txTracers1,  rxTracers1, data,  FHnodes,  totnodes,  lastFHnode, links);
            }
           
        }

        
        std::cout << GREEN << "Tracers enable" << RESET << std::endl;
        if (enablepcap){
            DUp2p.EnablePcap("./sim_results/sched.pcap", nodes, true);
        }
        
        if (enabletraceTimeStamps){
            traceTimeStamps(hl5nodes, 
                        TprocWaithl5nodesCallbacks, TprocWaithlxnodesCallbacks, 
                        TprocServhl5nodesCallbacks, TprocServhlxnodesCallbacks,
                        Twaithl5nodesCallbacks, TwaithlxnodesCallbacks, 
                        Tshl5nodesCallbacks, TshlxnodesCallbacks, data, totnodes);
        }
        
        


        Ptr<FlowMonitor> flowMonitor;
        FlowMonitorHelper flowHelper;
        flowMonitor = flowHelper.InstallAll();


        Ptr<PacketSink> Server_trace1 = StaticCast<PacketSink>(Server_appRU.Get(0));

        // Config::ConnectWithoutContext("/NodeList/3/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs", MakeCallback(&Sniffer));
        // Config::ConnectWithoutContext("/NodeList/2/DeviceList/0/$ns3::PointToPointNetDevice/TxSnifferAction", MakeCallback(&Sniffer));
        
        Simulator::Stop(Seconds(data.at("Seconds_sim")));
        Simulator::Schedule(Seconds(0), &PrintTotalRx, Server_trace1);
        Simulator::Run();
        Simulator::Destroy();
        
        std::cout << GREEN << "Simulation has finished" << RESET << std::endl;
        flowMonitor->SerializeToXmlFile("./sim_results/DelayXml.xml", false, true);
       
        // std::cout << MAGENTA << "INFO: " << RESET << "Sink: Total RX - " << Server_trace->GetTotalRx() << " bytes" << std::endl;
        return 0;
    }


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

    #include "ns3/applications-module.h"
    #include "ns3/core-module.h"
    #include "ns3/enum.h"
    #include "ns3/error-model.h"
    #include "ns3/event-id.h"
    #include "ns3/internet-module.h"
    #include "ns3/ipv4-global-routing-helper.h"
    #include "ns3/network-module.h"
    #include "ns3/point-to-point-module.h"
    #include "ns3/tcp-header.h"
    #include "ns3/udp-header.h"
    #include "ns3/traffic-control-module.h"
    #include "ns3/flow-monitor-helper.h"
    #include "json.hpp"
    #include "logs.h"


    #include <fstream>
    #include <iostream>
    #include <string>
    #include <vector>


    using namespace ns3;
    using json = nlohmann::json;
    std::ofstream File;
    
    void Sniffer(const std::string& tag){
        File  << Simulator::Now().GetFemtoSeconds() <<  " " << tag << std::endl;
    }



    class SnifferHelper {
    

        public:
            SnifferHelper(const std::string& filename) {
                TracingFile.open(filename + "Sniffer.log", std::fstream::out);
            }

            ~SnifferHelper() {
                TracingFile.close(); // Close the file in the destructor
            }

            void Sniffer(Ptr<const Packet> pkt, const std::string& tag) {
                TracingFile << Simulator::Now().GetFemtoSeconds() << " " << pkt->GetSize() <<  " " << tag << std::endl;
                // Don't close the file here; it will be automatically closed in the destructor
            }

            void SnifferStatus(const std::string& tag) {
                TracingFile << Simulator::Now().GetFemtoSeconds() <<  " " << tag << std::endl;
                // Don't close the file here; it will be automatically closed in the destructor
            }

        private:
            std::ofstream TracingFile;
            int num;
            std::string filename;

    };


    
    double parseMbpsTo1e9(const std::string& mbpsString) {
        double mbps = std::stod(mbpsString);
        double gbps = mbps * 1e6;
        return gbps;
    }

    class RxTracerHelper {
    public:
        RxTracerHelper(const std::string& name, const std::string& filename) {
            RxFile.open(filename + "RxFile" + name + ".log", std::fstream::out);
        }

        ~RxTracerHelper() {
            RxFile.close(); // Close the file in the destructor
        }

        void RxTracerWithAdresses(Ptr<const Packet> pkt, const Address & from) {
            RxFile << pkt->GetUid() << " " << Simulator::Now().GetFemtoSeconds() << " " << pkt->GetSize() << std::endl;
           
            // Don't close the file here; it will be automatically closed in the destructor
        }


    private:
        std::ofstream RxFile;
        int num;
        std::string filename;
      
    };


    class TxTracerHelper {
    public:
        TxTracerHelper(const std::string& typetx, int num, const std::string& filename, bool CPlane=true) {
            if (typetx == "RU1"){
                TxFileUser.open(filename + "Site1TxFileUser" + std::to_string(num) + ".log", std::fstream::out);
                if (CPlane){
                    TxFileControl.open(filename + "Site1TxFileControl" + std::to_string(num) + ".log", std::fstream::out);
                }
                
            }else if(typetx == "RU2"){
                TxFileUser.open(filename + "Site2TxFileUser" + std::to_string(num) + ".log", std::fstream::out);
                if (CPlane){
                    TxFileControl.open(filename + "Site2TxFileControl" + std::to_string(num) + ".log", std::fstream::out);
                }
               
            }else if(typetx == "BH1"){
                TxFile.open(filename + "BH1TxFile" + std::to_string(num) + ".log", std::fstream::out);
            }else if (typetx == "BH2"){
                TxFile.open(filename + "BH2TxFile" + std::to_string(num) + ".log", std::fstream::out);
            }else{
                std::cout << MAGENTA << "Tracer: Not recognized flag" << RESET << std::endl;
            }
            

       
        }


        ~TxTracerHelper() {
            TxFileUser.close(); // Close the file in the destructor
            TxFileControl.close();
            TxFile.close();
        }

        void TxTracer(Ptr<const Packet> pkt, const Address & from, const Address & to) {

            if( (InetSocketAddress::ConvertFrom(to).GetPort()/1000)%10 == 8 ){
                TxFileUser << pkt->GetUid() << " " << Simulator::Now().GetFemtoSeconds() << " " << pkt->GetSize() <<std::endl;
            }else if((InetSocketAddress::ConvertFrom(to).GetPort()/1000)%10 == 9 ){
                TxFileControl << pkt->GetUid() << " " << Simulator::Now().GetFemtoSeconds() << " " << pkt->GetSize() <<std::endl;
            }else{
                TxFile << pkt->GetUid() << " " << Simulator::Now().GetFemtoSeconds() << " " << pkt->GetSize() <<std::endl;
            }
            
            
            // Don't close the file here; it will be automatically closed in the destructor
        }


    private:
        std::ofstream TxFileUser;
        std::ofstream TxFileControl;
        std::ofstream TxFile;
        int num;
        std::string filename;
        std::string typetx;
       
    };

    // Function to print total received bytes
void PrintTotalRx(Ptr<PacketSink> serverSink) {
        std::cout << MAGENTA << "INFO: " << RESET << "Sink: Total RX - " << serverSink->GetTotalRx() << " bytes in t = " << Simulator::Now().GetSeconds() << std::endl;
        Simulator::Schedule(Seconds(Simulator::Now().GetSeconds()+0.01), &PrintTotalRx, serverSink);
    }



void configureApplications(NodeContainer nodes, int FHnodes, int lastFHnode, const json& data, Ipv4InterfaceContainer DUAccess, Ipv4InterfaceContainer RUsAccess, Ipv4InterfaceContainer BHAccess ,int totnodes,
    ApplicationContainer &Client_appRU, ApplicationContainer &Server_appRU, ApplicationContainer &Client_appBH, ApplicationContainer &Server_appBH, std::vector<bool> links){
       int port1 = 8080; 
       int port2 = 9090;
       int portBH = 11000;
       int napp = 0 ;
       int auxPort = portBH;
       int app = 0;
       int site_inode = 1;
       int bh_inode = 1; 
       int hl5node = 0;
       int totFlows = 0;
        for (const auto& nodeagreggation : data["Hl5Agreggration"]) {
            // Iterate over the Sites array within each Hl5Agreggration object

            int DU_i = 0; 
            for (const auto& siteInfo : nodeagreggation["Sites"]) {
                int cells = siteInfo["Cells"].get<int>();
                // Iterate over the CellFeatures array within each Sites object
                for (const auto& cellFeature : siteInfo["CellFeatures"]) {
                    double uRatenum = cellFeature["URatenum"].get<double>();
                    double uPacketSize = cellFeature["UPacketSize"].get<double>();
                    double cRatenum = cellFeature["CRatenum"].get<double>();
                    double cPacketSize = cellFeature["CPacketSize"].get<double>();
                    // if (links[hl5node]){
                    //     DU_i = 1;
                    // }
                    int nRU  =  (site_inode-1)*2 + 1;

                    InetSocketAddress serverAddress(RUsAccess.GetAddress(nRU, 0), port1 + napp);
                    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", Address(serverAddress));
                    Server_appRU.Add(packetSinkHelper.Install(nodes.Get(FHnodes + site_inode-1)));


                    // std::cout << "RU Destination node: " << FHnodes + site_inode -1 << " con @IP: " << RUsAccess.GetAddress(nRU, 0)  << " " << port1 + napp<<  std::endl;
                    // Create client helper

                    if (cellFeature["Poisson"].get<bool>()){
                        PoissonHelper clientHelper("ns3::UdpSocketFactory",  Address(serverAddress));
                    
                        // Calculate UInterval and CInterval
                        double UInterval = 1.0 / (uRatenum / (8 * uPacketSize));


                        // std::cout << "UInterval: " << UInterval << " CInterval: " << CInterval << std::endl;
            
                        double tid = double(1)/(double(uRatenum)/(double(8)*double(uPacketSize)));
                    
                        clientHelper.SetAttribute("Interval", DoubleValue(tid));
                        clientHelper.SetAttribute("MaxBytes", UintegerValue(cellFeature["UMaxBytes"].get<uint32_t>()));
                        clientHelper.SetAttribute("PacketSize", UintegerValue(cellFeature["UPacketSize"].get<uint32_t>()));
                        clientHelper.SetAttribute("PacketGen", StringValue(data.at("Model").get<std::string>()));
                        Client_appRU.Add(clientHelper.Install(NodeContainer{nodes.Get(totnodes+DU_i)}));
                    }else{
                        
                        Ofhv2Helper clientHelper("ns3::UdpSocketFactory");
                        clientHelper.SetAttribute("U-Plane", AddressValue(serverAddress));
                        // Calculate UInterval and CInterval
                        double UInterval = 1.0 / (uRatenum / (8 * uPacketSize));


                        // std::cout << "UInterval: " << UInterval << " CInterval: " << CInterval << std::endl;
                        // Set attributes
                        clientHelper.SetAttribute("CUPlane", BooleanValue( cellFeature["CUPlane"].get<bool>()));
                        // clientHelper.SetAttribute("Trafficpattern", StringValue("Exponential"));
                        clientHelper.SetAttribute("U-Interval", DoubleValue(UInterval));
                        clientHelper.SetAttribute("U-MaxBytes", UintegerValue(cellFeature["UMaxBytes"].get<uint32_t>()));
                        // clientHelper.SetAttribute("PacketDistribution", StringValue("Exponential"));
                        clientHelper.SetAttribute("U-PacketSize", UintegerValue(cellFeature["UPacketSize"].get<uint32_t>()));

                        if(cellFeature["CUPlane"].get<bool>() ){
                            InetSocketAddress serverAddress2(RUsAccess.GetAddress(nRU, 0), port2 + napp);
                            PacketSinkHelper packetSinkHelper2("ns3::UdpSocketFactory", Address(serverAddress2));
                            Server_appRU.Add(packetSinkHelper2.Install(nodes.Get(FHnodes + site_inode-1)));
                            double CInterval = 1.0 / (cRatenum / (8 * cPacketSize));
                            clientHelper.SetAttribute("C-Interval", DoubleValue(CInterval));
                            clientHelper.SetAttribute("C-Plane", AddressValue(serverAddress2));
                            clientHelper.SetAttribute("C-PacketSize", UintegerValue(cellFeature["CPacketSize"].get<uint32_t>()));
                            clientHelper.SetAttribute("C-MaxBytes", UintegerValue(cellFeature["CMaxBytes"].get<uint32_t>()));

                        }
                        Client_appRU.Add(clientHelper.Install(NodeContainer{nodes.Get(totnodes+DU_i)}));

                    }

                   
                    

                    // Increment app counter
                    napp++;
                }
                    site_inode++; // sitenodes assuming that they are consecuence nodes
                    DU_i++; // DUnodes
            }
      
           
            hl5node++;
        }

        if(data.contains("Backhaulenable") && data["Backhaulenable"].is_boolean()){
                // Check if backhaul is enabled

                if (data["Backhaulenable"].get<bool>() ) {
                    std::cout << "BH is enabled" << std::endl;
                    // Iterate over BHFeatures array
                    for (int bh = 0; bh < 1; bh++){ //double nodes per aggregation node, each one for each spine-leaf 
                        //  std::cout << "BH node: " << bh << std::endl;
                        int slice_index = 0;
                        for (const auto& slice : data["BHFeatures"]) {
                            for (int flow = 0; flow < data["FlowsPerSlice"]; flow++) {
                                int bhnode = 0;
                                auxPort = portBH + slice_index * 1000 + totFlows + flow;
                                
                   
                            if(data["Poisson"].get<bool>()){
                                InetSocketAddress serverAddress(BHAccess.GetAddress(2, 0), auxPort);
                                PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", Address(serverAddress));
                                Server_appBH.Add(packetSinkHelper.Install(nodes.Get(lastFHnode + 2)));
                                PoissonHelper clientHelper("ns3::UdpSocketFactory", serverAddress);
                                double Interval = double(1.0) / (double(parseMbpsTo1e9(slice["Rate"])) / (double(8) * double(slice["PacketSize"])));
                                clientHelper.SetAttribute("Interval", DoubleValue(Interval));
                                clientHelper.SetAttribute("MaxBytes", UintegerValue(slice["MaxBytes"]));
                                clientHelper.SetAttribute("PacketSize", UintegerValue(slice["PacketSize"]));
                                clientHelper.SetAttribute("PacketGen", StringValue(data.at("Model").get<std::string>()));
                                Client_appBH.Add(clientHelper.Install(NodeContainer{nodes.Get(lastFHnode + 1)}));
                            }else{
                                InetSocketAddress serverAddress(BHAccess.GetAddress(2, 0), auxPort);
                                PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", Address(serverAddress));
                                Server_appBH.Add(packetSinkHelper.Install(nodes.Get(lastFHnode + 2)));
                                // std::cout << "BH Destination node: " << lastsite + bh_inode << " con @IP: " << RUsAccess.GetAddress(nBH, 0) << " Port: " << auxPort <<  std::endl;
                                OnOffHelper clientHelper("ns3::UdpSocketFactory", serverAddress);
                                clientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant="+ to_string(slice["OnTime"])+"]"));
                                clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant="+ to_string(slice["OffTime"])+"]"));
                                clientHelper.SetAttribute("DataRate", StringValue((slice["Rate"])));
                                clientHelper.SetAttribute("MaxBytes", UintegerValue(slice["MaxBytes"]));
                                clientHelper.SetAttribute("PacketSize", UintegerValue(slice["PacketSize"]));
                                Client_appBH.Add(clientHelper.Install(NodeContainer{nodes.Get(lastFHnode + 1)}));
                            }



                                // std::cout << "BH Source node: " <<lastFHnode + 2<< std::endl;
                                app++;   
                                // std::cout << app << std::endl;
                            }
                            slice_index++;
                        }
                        
                        bh_inode++;
                    }
                    totFlows += data["FlowsPerSlice"].get<int>();
                }
             


            }

    }





void tracerlogging(int hl5nodes, const std::vector<std::unique_ptr<RxTracerHelper>>& rxTracersRUSite1, 
                    const std::vector<std::unique_ptr<TxTracerHelper>>& txTracersRUSite1,
                    const std::vector<std::unique_ptr<TxTracerHelper>>& txTracers1, 
                    const std::vector<std::unique_ptr<RxTracerHelper>>& rxTracers1,
                    const json& data, int FHnodes, int totnodes, int lastFHnode, std::vector<bool> links ){



            int n_controll_call_site1 = 0;
            int n_user_call_site1 = 0;
            int nuser = 0, ncontrol = 0;
            int txnode6 = 0, txnode7 = 0;
            int txnode8 = 0, txnode9 = 0;    

            for (int i = 0; i < hl5nodes; i++) {  // TO ADJUST TO the size of hl5aggregation
                int sites = data["Hl5Agreggration"][i]["Sites"].size();
                for (int j = 0; j < data["Hl5Agreggration"][i]["Sites"].size(); j++) {
                    // Construct the callback paths
                    for (int z = 0; z < data["Hl5Agreggration"][i]["Sites"][j]["Cells"]; z++){
                        if(data["Hl5Agreggration"][i]["Sites"][j]["CellFeatures"][z]["Logging"].get<bool>()){
                            if(j == 0){
                            
                                int Planes = 2;
                                // if (!links[i]){
                                std::string txCallbackPath;
                                if (data["Hl5Agreggration"][i]["Sites"][j]["CellFeatures"][z]["Poisson"].get<bool>()){
                                    txCallbackPath = "/NodeList/" + std::to_string(totnodes) + "/ApplicationList/" + std::to_string(txnode6) + "/$ns3::Poissonapp/TxWithAddresses";
                                }else{
                                    txCallbackPath = "/NodeList/" + std::to_string(totnodes) + "/ApplicationList/" + std::to_string(txnode6) + "/$ns3::ofhapplication/TxWithAddresses";
                                }

                                
                                    // std::cout << txCallbackPath << std::endl;
                                    // std::cout << n_user_call_site1 << std::endl;
                                    Config::ConnectWithoutContext(txCallbackPath, MakeCallback(&TxTracerHelper::TxTracer, txTracersRUSite1[n_user_call_site1].get()));
                                    bool CPlaneflag = data["Hl5Agreggration"][i]["Sites"][j]["CellFeatures"][z]["CUPlane"];
                                    if (!CPlaneflag) { Planes = 1; }
                            
                                    for (int planes = 0; planes < Planes; planes++) {
                                        std::string rxCallbackPath = "/NodeList/" + std::to_string((FHnodes) + sites*i +j) + "/ApplicationList/" + std::to_string(z*2 + planes) + "/$ns3::PacketSink/Rx";
                                        // std::cout << rxCallbackPath << std::endl;
                                        // Connect the RxTracerHelper callback
                                        // std::cout << "Site1: Instance control: " << n_controll_call_site1 << std::endl;
                                        Config::ConnectWithoutContext(rxCallbackPath, MakeCallback(&RxTracerHelper::RxTracerWithAdresses, rxTracersRUSite1[n_controll_call_site1].get()));
                                        n_controll_call_site1++;
                                    }
                            }
                                txnode6++;
                                n_user_call_site1++;    
                        }
                           nuser++;
                           ncontrol++;
                    }
                }

        
            }

            if(data["Backhaulenable"].get<bool>()){
                int slices = data["Slices"];
                std::cout << "Backhaul logging is enabled" << std::endl;
                for (const auto& slice : data["BHFeatures"]) {
                    if (slices == 0 ){
                        break;
                    }
                    for (int flow = 0; flow < data["FlowsPerSlice"]; flow++) {
                        if (slice["Logging"]){
                            // std::cout << "Checking logging" << std::endl;
                            std::string txCallbackPath;
                            if(data.at("Poisson")){
                                txCallbackPath = "/NodeList/" + std::to_string(totnodes + 1) + "/ApplicationList/" + std::to_string(txnode8) + "/$ns3::Poissonapp/TxWithAddresses";
                            }else{
                                txCallbackPath = "/NodeList/" + std::to_string(totnodes + 1) + "/ApplicationList/" + std::to_string(txnode8) + "/$ns3::OnOffApplication/TxWithAddresses";;
                            }
                            // std::cout << txCallbackPath << std::endl;
                            // std::cout << txCallbackPath << std::endl;
                            Config::ConnectWithoutContext(txCallbackPath, MakeCallback(&TxTracerHelper::TxTracer, txTracers1[txnode8].get()));
                            std::string rxCallbackPath = "/NodeList/" + std::to_string(totnodes + 2) + "/ApplicationList/" + std::to_string(txnode9) + "/$ns3::PacketSink/Rx";
                            // std::cout << rxCallbackPath << std::endl;
                            Config::ConnectWithoutContext(rxCallbackPath, MakeCallback(&RxTracerHelper::RxTracerWithAdresses, rxTracers1[txnode9].get()));
                        }
                       
                        txnode8++;
                        txnode9++;
                    }
                    slices--;
                }

            }
                    
                  

                        
}                  
                    

void traceTimeStamps(int hl5nodes, 
                    const std::vector<std::unique_ptr<SnifferHelper>>& TprocWaithl5nodesCallbacks, const std::vector<std::unique_ptr<SnifferHelper>>& TprocWaithlxnodesCallbacks,
                    const std::vector<std::unique_ptr<SnifferHelper>>& TprocServhl5nodesCallbacks, const std::vector<std::unique_ptr<SnifferHelper>>& TprocServhlxnodesCallbacks,
                    const std::vector<std::unique_ptr<SnifferHelper>>& Twaithl5nodesCallbacks,  const std::vector<std::unique_ptr<SnifferHelper>>& TwaithlxnodesCallbacks, 
                    const std::vector<std::unique_ptr<SnifferHelper>>& Tshl5nodesCallbacks,  const std::vector<std::unique_ptr<SnifferHelper>>& TshlxnodesCallbacks,
                    const json& data, int totnodes){

        std::string CallbackPath;                    
        if (data["hl4level"]){
            std::cout << YELLOW << "HL4-HL5 level" << RESET << std::endl;

            CallbackPath = "/NodeList/" + std::to_string(totnodes) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TwaithlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/" + std::to_string(totnodes) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TshlxnodesCallbacks[0].get()));


            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocServ";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TprocServhlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocWait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TprocWaithlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TwaithlxnodesCallbacks[1].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TshlxnodesCallbacks[1].get()));
        }else{
            std::cout << YELLOW << "HL3-HL5 level" << RESET << std::endl;

            CallbackPath = "/NodeList/" + std::to_string(totnodes) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TwaithlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/" + std::to_string(totnodes) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TshlxnodesCallbacks[0].get()));


            CallbackPath = "/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocServ";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TprocServhlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocWait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TprocWaithlxnodesCallbacks[0].get()));
            CallbackPath = "/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TwaithlxnodesCallbacks[1].get()));
            CallbackPath = "/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TshlxnodesCallbacks[1].get()));



            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocServ";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TprocServhlxnodesCallbacks[1].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocWait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TprocWaithlxnodesCallbacks[1].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TwaithlxnodesCallbacks[2].get()));
            CallbackPath = "/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TshlxnodesCallbacks[2].get()));

        }

        
    
        for (int i = 0;  i < hl5nodes; i++){
            CallbackPath = "/NodeList/" + std::to_string(4 + i) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocServ";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, TprocServhl5nodesCallbacks[i].get()));
            CallbackPath = "/NodeList/" + std::to_string(4 + i) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTprocWait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, TprocWaithl5nodesCallbacks[i].get()));
            CallbackPath = "/NodeList/" + std::to_string(4 + i) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTwait";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::Sniffer, Twaithl5nodesCallbacks[i].get()));
            CallbackPath = "/NodeList/" + std::to_string(4 + i) + "/DeviceList/*/$ns3::PointToPointNetDevice/SnifferTs";
            Config::ConnectWithoutContext(CallbackPath, MakeCallback(&SnifferHelper::SnifferStatus, Tshl5nodesCallbacks[i].get()));

        }


}
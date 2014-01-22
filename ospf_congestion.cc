
//#include "ns3/output-stream-wrapper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/nstime.h"

#include "ns3/ipv4-ospf-routing-helper.h"
#include "ns3/conf-loader.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/queue.h"

#include <list>
#include <string>
#include <iostream>
#include <sstream>
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("OspfExample");

int downNode1 = 1;
int downInterface1 = 2;

void downAction(){
    cout << "action down" << endl;
    int downNode2 = ConfLoader::Instance()->getCoreNum()+downInterface1-1;
    
    ConfLoader::Instance()->getNodeContainer().Get (downNode1)->GetObject<Ipv4OSPFRouting>()->removeNeighbor(downNode2);
    ConfLoader::Instance()->getNodeContainer().Get (downNode2)->GetObject<Ipv4OSPFRouting>()->removeNeighbor(downNode1);
}

void upAction(){
    cout << "action up" << endl;
    int downNode2 = ConfLoader::Instance()->getCoreNum()+downInterface1-1;
    
    ConfLoader::Instance()->getNodeContainer().Get (downNode1)->GetObject<Ipv4OSPFRouting>()->addNeighbor(downNode2);
    ConfLoader::Instance()->getNodeContainer().Get (downNode2)->GetObject<Ipv4OSPFRouting>()->addNeighbor(downNode1);
}

void initLSAs(){
    map<int, vector<uint16_t> > LSAs;
    for(int i=0; i<ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum(); i++){
        std::vector<uint16_t> lsa;
        if(i<ConfLoader::Instance()->getCoreNum()){
            for(int j=ConfLoader::Instance()->getCoreNum(); j<ConfLoader::Instance()->getTotalNum(); j++){
                lsa.push_back((uint16_t)j);
            }
        }else if(i<ConfLoader::Instance()->getTotalNum()){
            for(int j=0; j<ConfLoader::Instance()->getCoreNum();j++){
                lsa.push_back((uint16_t)j);
            }
            if(i<(ConfLoader::Instance()->getCoreNum()+ConfLoader::Instance()->getToRNum())){
                lsa.push_back((uint16_t)(i+ConfLoader::Instance()->getBorderNum()+ConfLoader::Instance()->getToRNum()));
            }
        }else{
            lsa.push_back((uint16_t)(i-(ConfLoader::Instance()->getBorderNum()+ConfLoader::Instance()->getToRNum())));
        }
        LSAs[i] = lsa;
        for(std::vector<uint16_t>::iterator it = lsa.begin(); it!=lsa.end(); ++it){
            ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->addToNeighbors((int)(*it), Simulator::Now());
        }
    }
    for(int i=0; i<ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum(); i++){
        ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->setLSA(LSAs);
    }
}

void update(){
  for(int i=0; i<ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum(); i++){
        ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->update();
  }
}

void printTxInfo(int node, int index){
    PointerValue ptr;
    Ptr<Queue> txQueue;
    ConfLoader::Instance()->getNodeContainer().Get(node)->GetObject<Ipv4OSPFRouting>()->getIpv4()->GetNetDevice (index)->GetAttribute("TxQueue", ptr);
    txQueue = ptr.Get<Queue> ();
    cout << node << ":" << index << " totalDropped Packets: "<< txQueue->GetTotalDroppedPackets()<< endl;
    cout << node << ":" << index << " totalReveived Packets: "<< txQueue->GetTotalReceivedPackets()<< endl;
}

void statistics(){

    printTxInfo(8,1);
    printTxInfo(2,1);
    printTxInfo(2,2);

    printTxInfo(9,1);
    printTxInfo(3,1);
    printTxInfo(3,2);

    printTxInfo(10,1);
    printTxInfo(4,1);
    printTxInfo(4,2);

    printTxInfo(11,1);
    printTxInfo(5,1);
    printTxInfo(5,2);

    printTxInfo(0,1);
    printTxInfo(0,2);
    printTxInfo(0,3);
    printTxInfo(0,4);

    printTxInfo(1,1);
    printTxInfo(1,2);
    printTxInfo(1,3);
    printTxInfo(1,4);

    printTxInfo(2,3);
    printTxInfo(3,3);
    printTxInfo(4,3);
    printTxInfo(5,3);
}

void Hello(){
  cout << Simulator::Now() << "----------------hello---------"<<endl;
    for(int i=0; i<ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum(); i++){
        ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->sendHelloMessage();
    }
}

void CheckNeighbor(){
  cout << Simulator::Now() << "----------------checkNeighbor---------"<<endl;
    for(int i=0; i<ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum(); i++){
        ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->checkNeighbors();
    }
}

void createApplication(int source, int dest, int port, string rate, int size, float start, float stop){
  cout << "Create Applications " << source << " : " << dest << " : "<< port << " : "<< rate << " : "<< size << " : "<< start << " : "<< stop << endl;

  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (ConfLoader::Instance()->getNodeContainer().Get(dest)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port)));
                     //Address (InetSocketAddress ("255.255.255.255", port)));
  onoff.SetConstantRate (DataRate (rate), size);
  ApplicationContainer apps = onoff.Install (ConfLoader::Instance()->getNodeContainer().Get (source));
  apps.Start (Seconds (start));
  apps.Stop (Seconds (stop));
}

int main (int argc, char *argv[])
{
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("OSPFRoutingHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("Ipv4OSPFRouting", LOG_LEVEL_ALL);
  //LogComponentEnable ("Queue", LOG_LEVEL_ALL);
  //LogComponentEnableAll(LOG_LEVEL_ALL);


  //int UnavailableInterval = 3;
  //int HelloInterval = 2;
  //float CheckNeighborInterval = 0.1;
    int CORE_NUM = 4;
  int TOR_NUM = 8;
  int BORDER_NUM = 2;
  int SUBNET_MASK = 24;
  uint32_t ADDRESS_START = 0x0a000000; // 10.0.0.1
  int nNodes = CORE_NUM + TOR_NUM + BORDER_NUM;
  int total = nNodes + TOR_NUM;
  float app_start_time = 1.0;
  float app_stop_time = 10.0;
  uint32_t stopTime = 60;
  string dataRate = "1Gbps";//"1Gbps";
  string delay = "1ms";
  string sendRate = "10Mb/s";//"100Mb/s";
  string dequeGap = "0.3ms";
  uint32_t packetReceiveDelay = 1000; //ns
  int maxPackets = 1000;
  uint16_t port = 9;   // Discard port (RFC 863)
  /*int sendNode = nNodes+2;
  int destNode = nNodes+1;
  int sendNode2 = nNodes+0;
  int destNode2 = nNodes+1;*/
  uint32_t packetSize = 512;
  float CONGESTION_WARNING_LIMIT = 0.98;
  float CALCULATE_COST = 0.06; //s
  float simulateTime = app_stop_time;
  float simulateInterval = 0.5;
  
  //float downTime = 2;
  //float upTime  = 8;

  //float findDelay = 0.1; //s
  float lspDelay = 0.01;
  //ConfLoader::Instance()->setUnavailableInterval(UnavailableInterval);
  ConfLoader::Instance()->setCoreNum(CORE_NUM);
  ConfLoader::Instance()->setToRNum(TOR_NUM);
  ConfLoader::Instance()->setBorderNum(BORDER_NUM);
  ConfLoader::Instance()->setSubnetMask(SUBNET_MASK);
  ConfLoader::Instance()->setAddressStart(ADDRESS_START);
  ConfLoader::Instance()->setPacketReceiveDelay(packetReceiveDelay);
  ConfLoader::Instance()->setCongestionWaningLimit(CONGESTION_WARNING_LIMIT);
  ConfLoader::Instance()->setCalculateCost(CALCULATE_COST);
  ConfLoader::Instance()->setLSPDelay(lspDelay);

  CommandLine cmd;
  bool enableFlowMonitor = true;
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  //cmd.AddValue ("nNodes", "Number of Router nodes", nNodes);
  cmd.Parse (argc, argv);

  //
  //  Step 1
  //  Node Basic Configuration
  //
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (total);
  ConfLoader::Instance()->setNodeContainer(c);

  list<NodeContainer> nodeContainers;

  for(int i=0; i<CORE_NUM; i++){
      for(int j=CORE_NUM; j<CORE_NUM+TOR_NUM; j++){
          NodeContainer ninj = NodeContainer (c.Get(i), c.Get(j));
          nodeContainers.push_back(ninj);
      }
  }

  for(int i=0; i<CORE_NUM; i++){
      for(int j=CORE_NUM+TOR_NUM; j< nNodes; j++){
          NodeContainer ninj = NodeContainer (c.Get(i), c.Get(j));
          nodeContainers.push_back(ninj);
      }
  }

  list<NodeContainer> nodeContainers_2;
  for(int i=CORE_NUM; i<CORE_NUM+TOR_NUM; i++){
      NodeContainer ninj = NodeContainer (c.Get(i), c.Get(i+TOR_NUM+BORDER_NUM));
      nodeContainers_2.push_back(ninj);
  }

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (delay));
  pointToPoint.SetDeviceAttribute ("InterframeGap", StringValue(dequeGap));

  list<NetDeviceContainer> netDeviceContainers;
  for(list<NodeContainer>::iterator it= nodeContainers.begin(); it!=nodeContainers.end(); ++it){
      NetDeviceContainer didj = pointToPoint.Install (*it);
      netDeviceContainers.push_back(didj);
  }

    /*PointerValue ptr;
    netDeviceContainers.front().Get(0)->GetAttribute ("TxQueue", ptr);
    Ptr<Queue> txQueue = ptr.Get<Queue> ();

    UintegerValue limit;
    txQueue->GetAttribute ("MaxPackets", limit);
    cout << "MaxPackets: " << limit.Get () << " packets" << endl;

    txQueue->GetAttribute ("MaxBytes", limit);
    cout << "MaxBytes: " << limit.Get () << " bytes" << endl;*/

    //Config::Set ("/NodeList/*/DeviceList/*/TxQueue/MaxBytes", UintegerValue (0));


  list<NetDeviceContainer> netDeviceContainers_2;
  for(list<NodeContainer>::iterator it= nodeContainers_2.begin(); it!=nodeContainers_2.end(); ++it){
      NetDeviceContainer didj = pointToPoint.Install (*it);
      netDeviceContainers_2.push_back(didj);
  }

  //NetDeviceContainer devices;
  //devices = pointToPoint.Install (c);
  //
  //
  // Address Configuration
  //
  //
      int i = 0;
      for(list<NetDeviceContainer>::iterator it= netDeviceContainers.begin(); it!=netDeviceContainers.end(); ++it){
          stringstream ss;
          ss << "192." << i/255 << "." << i%255 << ".0";
          string b = ss.str();
          Subnet subnet(b,SUBNET_MASK);
          int index1 = i/(TOR_NUM+BORDER_NUM);
          int index2 = i%(TOR_NUM+BORDER_NUM)+CORE_NUM;
          ConfLoader::Instance()->addItem2LinkSubnetMap(index1, index2, subnet);
          ConfLoader::Instance()->addSubnet(subnet);
          i++;
      }
      i = 0;
      for(list<NetDeviceContainer>::iterator it= netDeviceContainers_2.begin(); it!=netDeviceContainers_2.end(); ++it){
          stringstream ss;
          ss << "10." << i/255 << "." << i%255 << ".0";
          string b = ss.str();
          Subnet subnet(b,SUBNET_MASK);
          int index1 = i+CORE_NUM;
          int index2 = i+CORE_NUM+TOR_NUM+BORDER_NUM;
          ConfLoader::Instance()->addItem2LinkSubnetMap(index1, index2, subnet);
          ConfLoader::Instance()->addSubnet(subnet);
          ConfLoader::Instance()->addToNodeSubnet(index2, subnet);
          i++;
      }


      // Internet stack install
      InternetStackHelper internet;
      
      Ipv4ListRoutingHelper listRouting;
      //Ipv4StaticRoutingHelper staticRouting;
      Ipv4OSPFRoutingHelper ipv4OSPFRoutingHelper;
      //listRouting.Add (staticRouting, 0);
      listRouting.Add (ipv4OSPFRoutingHelper, 10);

      internet.SetRoutingHelper (listRouting);
      internet.Install (c);

      i = 0;
      list<Ipv4InterfaceContainer> ipv4InterfaceContainers;
      for(list<NetDeviceContainer>::iterator it= netDeviceContainers.begin(); it!=netDeviceContainers.end(); ++it){
          stringstream ss;
          ss << "192." << i/255 << "." << i%255 << ".0";
          string b = ss.str();
          //cout << b << endl;
          Ipv4AddressHelper ipv4AddrHelper;
          ipv4AddrHelper.SetBase (b.c_str(), "255.255.255.0");
          /*Subnet subnet(b,SUBNET_MASK);
          int index1 = i/(TOR_NUM+BORDER_NUM);
          int index2 = i%(TOR_NUM+BORDER_NUM)+CORE_NUM;
          ConfLoader::Instance()->addItem2LinkSubnetMap(index1, index2, subnet);
          ConfLoader::Instance()->addSubnet(subnet);*/
          Ipv4InterfaceContainer ii = ipv4AddrHelper.Assign (*it);
          ipv4InterfaceContainers.push_back(ii);
          i++;
      }
      i = 0;
      list<Ipv4InterfaceContainer> ipv4InterfaceContainers_2;
      for(list<NetDeviceContainer>::iterator it= netDeviceContainers_2.begin(); it!=netDeviceContainers_2.end(); ++it){
          stringstream ss;
          ss << "10." << i/255 << "." << i%255 << ".0";
          string b = ss.str();
          //cout << b << endl;
          Ipv4AddressHelper ipv4;
          ipv4.SetBase (b.c_str(), "255.255.255.0");
          /*Subnet subnet(b,SUBNET_MASK);
          int index1 = i+CORE_NUM;
          int index2 = i+CORE_NUM+TOR_NUM+BORDER_NUM;
          ConfLoader::Instance()->addItem2LinkSubnetMap(index1, index2, subnet);
          ConfLoader::Instance()->addSubnet(subnet);*/
          Ipv4InterfaceContainer ii = ipv4.Assign (*it);
          ipv4InterfaceContainers.push_back(ii);
          i++;
      }

    map<pair<int,int>, Subnet> mm = ConfLoader::Instance()->getLinkSubnetMap();
    cout << mm.size() << endl;
    for(map<pair<int,int>, Subnet>::iterator it = mm.begin(); it!=mm.end(); ++it){
        cout << it->first.first << " " << it->first.second << " " << it->second.toString()<<endl;
    }
      //Ipv4InterfaceContainer interfaces = ipv4AddrHelper.Assign (devices);
      //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  NS_LOG_INFO ("Create Applications.");
  
  Config::Set ("/NodeList/*/DeviceList/*/TxQueue/MaxPackets", UintegerValue (maxPackets));
  //FlowMonitorHelper flowmon;
  //Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  //pointToPoint.EnablePcapAll ("dce-quagga-ospfd");
  //Ptr<OutputStreamWrapper> stream = traceHelper.CreateFileStream ("ospf-example.tr");
  NodeContainer cc;
  cc.Add(c.Get(0));
  pointToPoint.EnableAscii("TxRxPointToPoint", cc);
  pointToPoint.EnableAscii("MacTxDrop", cc);

  //cout << destNode << endl;
  
  //createApplication(sendNode, destNode, port, sendRate, packetSize, app_start_time, app_stop_time);
  //createApplication(sendNode2, destNode2, port, sendRate, packetSize, app_start_time, app_stop_time);
  
  createApplication(nNodes+0, nNodes+4, port, sendRate, packetSize, app_start_time, app_stop_time);
  createApplication(nNodes+1, nNodes+4, port, sendRate, packetSize, app_start_time, app_stop_time);
  createApplication(nNodes+6, nNodes+4, port, sendRate, packetSize, app_start_time, app_stop_time);
  createApplication(nNodes+3, nNodes+4, port, sendRate, packetSize, app_start_time, app_stop_time);
  ApplicationContainer apps;

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  for(int i=CORE_NUM; i< CORE_NUM+TOR_NUM;i++){  
    apps = sink.Install (c.Get (i));
  }
  apps.Start (Seconds (0));
  apps.Stop (Seconds (stopTime));

  /*Ptr<OutputStreamWrapper> routintable = Create<OutputStreamWrapper>("routingtable",std::ios::out);
  for(int i=0;i<total;i++){
    //ipv4RoutingHelper.PrintRoutingTableAt(Seconds(0), c.Get(i), routintable);
    //ipv4RoutingHelper.PrintRoutingTableAt(Seconds(10), c.Get(i), routintable);
    //ipv4RoutingHelper.PrintRoutingTableAt(Seconds(app_stop_time), c.Get(i), routintable);
    //ipv4RoutingHelper.PrintRoutingTableAt(Seconds(stopTime), c.Get(i), routintable);
  }*/

  for(int i = 0; i< total; i++){
      cout << c.Get(i)->GetObject<Ipv4OSPFRouting>()->toString() << endl;
  }

  //Simulator::Schedule(Seconds(10), &action, c.Get(0));
  
  cout << "Run Simulation." << endl;
  //Ptr<Node> n1 = c.Get (downNode1);
  //Ptr<Ipv4> ipv4 = n1->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  //uint32_t ipv4ifIndex = downInterface1;
  Simulator::Schedule (Seconds (0), &initLSAs);
  //Simulator::Schedule (Seconds (stopTime), &statistics);
  //Simulator::Schedule (Seconds (downTime),&Ipv4::SetDown, ipv4, ipv4ifIndex);
  //Simulator::Schedule (Seconds (downTime+ findDelay ),&downAction);
  //Simulator::Schedule (Seconds (upTime),&Ipv4::SetUp, ipv4, ipv4ifIndex);
  //Simulator::Schedule (Seconds (upTime+ findDelay ),&upAction);
  int N =(int)(simulateTime/simulateInterval);
  for(int i=1; i<N;i++){
    Time onInterval = Seconds (i*simulateInterval);
    Simulator::Schedule (onInterval, &update);
  }
/*
  int N = stopTime/HelloInterval;
  for(int i=0; i< N; i++){
    Time onInterval = Seconds(i*HelloInterval);
    Simulator::Schedule (onInterval, &Hello);
  }

  N = (int)(stopTime/CheckNeighborInterval);
  for(int i=0; i< N; i++){
    Time onInterval = Seconds(i*CheckNeighborInterval);
    Simulator::Schedule (onInterval, &CheckNeighbor);
  }*/

  if (stopTime != 0)
    {
      Simulator::Stop (Seconds (stopTime));
    }
  Simulator::Run ();
  cout << "Done." << endl;
  cout << "Lost packets: " << endl << ConfLoader::Instance()->PrintMap(ConfLoader::Instance()->getLossPacketCounter()) << endl;
  cout << "Send: " << endl << ConfLoader::Instance()->PrintMap(ConfLoader::Instance()->getSendPacket()) << endl;
  cout << "Receive: " << endl << ConfLoader::Instance()->PrintMap(ConfLoader::Instance()->getRecvPacket()) << endl;
  cout << "Success: " << endl << ConfLoader::Instance()->PrintMap(ConfLoader::Instance()->getSuccessPacket()) << endl;
  return 0;
}

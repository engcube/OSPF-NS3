
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


#include <list>
#include <string>
#include <iostream>
#include <sstream>
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("OspfExample");

int action_time = 0;

void action(){
}

void update(){
  //cout << "----------------update---------"<<endl;
  action_time ++;
}


int main (int argc, char *argv[])
{
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("DceManager", LOG_LEVEL_INFO);
  //LogComponentEnable ("DceApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("Ipv4DceRoutingHelper", LOG_LEVEL_INFO);
  //LogComponentEnable ("Ipv4DceRouting", LOG_LEVEL_INFO);

  //LogComponentEnable ("QuaggaHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("OSPFRoutingHelper", LOG_LEVEL_INFO);
  //LogComponentEnable ("Ipv4OSPFRouting", LOG_LEVEL_INFO);



  int CORE_NUM = 2;
  int TOR_NUM = 4;
  int BORDER_NUM = 2;
  
  int SUBNET_MASK = 24;
  uint32_t ADDRESS_START = 0x0a000000; // 10.0.0.1

  int nNodes = CORE_NUM + TOR_NUM + BORDER_NUM;
  int total = nNodes + TOR_NUM;
  
  float app_start_time = 1.0;
  float app_stop_time = 2.0;

  uint32_t stopTime = 600;

  string dataRate = "100Mbps";//"1Gbps";
  string delay = "0ms";
  string dest_ip = "10.0.1.2";
  string sendRate = "0.1Mb/s";//"100Mb/s";
  uint16_t port = 9;   // Discard port (RFC 863)
  int sendNode = nNodes+3;
  int destNode = nNodes+2;
  int simulateTime = (int)app_stop_time;
  int simulateInterval = 3;
  uint32_t packetSize = 512;

  
  ConfLoader::Instance()->setCoreNum(CORE_NUM);
  ConfLoader::Instance()->setToRNum(TOR_NUM);
  ConfLoader::Instance()->setBorderNum(BORDER_NUM);
  ConfLoader::Instance()->setSubnetMask(SUBNET_MASK);
  ConfLoader::Instance()->setAddressStart(ADDRESS_START);

  CommandLine cmd;
  bool enableFlowMonitor = false;
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

  list<NetDeviceContainer> netDeviceContainers;
  for(list<NodeContainer>::iterator it= nodeContainers.begin(); it!=nodeContainers.end(); ++it){
      NetDeviceContainer didj = pointToPoint.Install (*it);
      netDeviceContainers.push_back(didj);
  }

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

  //pointToPoint.EnablePcapAll ("dce-quagga-ospfd");

  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (c.Get(destNode)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port)));
     
  onoff.SetConstantRate (DataRate (sendRate), packetSize);
  ApplicationContainer apps = onoff.Install (c.Get (sendNode));

  apps.Start (Seconds (app_start_time));
  apps.Stop (Seconds (app_stop_time));
  
  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  for(int i=CORE_NUM; i< CORE_NUM+TOR_NUM;i++){  
    apps = sink.Install (c.Get (i));
  }
  apps.Start (Seconds (app_start_time));
  apps.Stop (Seconds (app_stop_time));

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
  
  Ptr<Node> n1 = c.Get (sendNode);
  Ptr<Ipv4> ipv4 = n1->GetObject<Ipv4> ();
  // The first ifIndex is 0 for loopback, then the first p2p is numbered 1,
  // then the next p2p is numbered 2
  uint32_t ipv4ifIndex = 1;

  Simulator::Schedule (Seconds (2),&Ipv4::SetDown,ipv4, ipv4ifIndex);
  Simulator::Schedule (Seconds (10),&Ipv4::SetUp,ipv4, ipv4ifIndex);

  for(int i=1; i<simulateTime/simulateInterval;i++){
    Time onInterval = Seconds (i*simulateInterval);
    Simulator::Schedule (onInterval, &update);
  }

  if (stopTime != 0)
    {
      Simulator::Stop (Seconds (stopTime));
    }
  Simulator::Run ();
  cout << "Done." << endl;
  Simulator::Destroy ();

  return 0;
}

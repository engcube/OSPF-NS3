// -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-
//
// Copyright (c) 2008 University of Washington
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <vector>
#include <iomanip>
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/boolean.h"
#include "ipv4-ospf-routing.h"
#include "ns3/ospf-tag.h"

#include <sstream>
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("Ipv4OSPFRouting");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Ipv4OSPFRouting);

TypeId 
Ipv4OSPFRouting::GetTypeId (void)
{ 
  static TypeId tid = TypeId ("ns3::Ipv4OSPFRouting")
    .SetParent<Object> ()
    .AddAttribute ("RandomEcmpRouting",
                   "Set to true if packets are randomly routed among ECMP; set to false for using only one route consistently",
                   BooleanValue (false),
                   MakeBooleanAccessor (&Ipv4OSPFRouting::m_randomEcmpRouting),
                   MakeBooleanChecker ())
    .AddAttribute ("RespondToInterfaceEvents",
                   "Set to true if you want to dynamically recompute the OSPF routes upon Interface notification events (up/down, or add/remove address)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&Ipv4OSPFRouting::m_respondToInterfaceEvents),
                   MakeBooleanChecker ())
  ;
  return tid;
}

//Ipv4OSPFRouting::Ipv4OSPFRouting (){
//}

Ipv4OSPFRouting::Ipv4OSPFRouting () 
  : m_randomEcmpRouting (false),
    m_respondToInterfaceEvents (false)
{
  NS_LOG_FUNCTION (this);
  m_update_state = false;
  m_rand = CreateObject<UniformRandomVariable> ();
  //m_OSPFRoutingTable = map<Subnet, int>();
}

Ipv4OSPFRouting::~Ipv4OSPFRouting ()
{
  NS_LOG_FUNCTION (this);
}

void Ipv4OSPFRouting::SetOSPFRoutingTable(map<Subnet, int>& grid){
    this->m_OSPFRoutingTable = grid;
}

map<Subnet, int>& Ipv4OSPFRouting::GetOSPFRoutingTable (void){
    return this->m_OSPFRoutingTable;
}

void
Ipv4OSPFRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Ipv4RoutingProtocol::DoDispose ();
}

// Formatted like output of "route -n" command
void
Ipv4OSPFRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION (this << stream);
}

/*Ptr<OSPFRouter> Ipv4OSPFRouting::getRouter() const{
  return m_router;
}

void Ipv4OSPFRouting::setRouter(Ptr<OSPFRouter> router){
  m_router = router;
}
*/

bool Ipv4OSPFRouting::update(){
  //cout << "<<<<in update  " << m_id << endl;
  NS_LOG_DEBUG(Simulator::Now() << "update  " << m_id );
  return true;
}

void Ipv4OSPFRouting::addToNeighbors(int neighbor, Time time){
    m_CurNeighbors[neighbor] = time;
}
  
map<int, Time>& Ipv4OSPFRouting::getNeighbors(){
    return m_CurNeighbors;
}

void Ipv4OSPFRouting::removeFromNeighbors(int neighbor){
    m_CurNeighbors.erase(neighbor);
}

void Ipv4OSPFRouting::addToLinkStateDatabase(int node, int cost){
    m_LinkStateDatabase[node] = cost;
}

map<int, int>& Ipv4OSPFRouting::getLinkStateDatabase(){
    return m_LinkStateDatabase;
}
  
void Ipv4OSPFRouting::removeFromLinkStateDatabase(int node){
    m_LinkStateDatabase.erase(node);
}


string Ipv4OSPFRouting::toString(){
    stringstream result;
    result << m_id << "\tRoutingTable:" << endl;
    for(map<Subnet, int>::iterator it = m_OSPFRoutingTable.begin(); it != m_OSPFRoutingTable.end(); ++it){
        result << it->first.toString() << "\t" << it->second << "\n";
    }
    return result.str();
}

void Ipv4OSPFRouting::sendHelloMessage(){
    Ptr<Packet> packet = Create<Packet>(1);
    OSPFTag tag;
    tag.setType(1);
    tag.setNode(m_id);
    packet->AddPacketTag(tag);

    Ptr<Socket> m_socket = Socket::CreateSocket (ConfLoader::Instance()->getNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
    cout << m_id << " send a hello message" << endl;
}

void Ipv4OSPFRouting::sendLSAMessage(int node, int index){
    m_LSAs[node] = ConfLoader::Instance()->getLSA(index);
    Ptr<Packet> packet = Create<Packet>(1);
    OSPFTag tag;
    tag.setType(2);
    tag.setNode(m_id);
    //tag.setLSA(node, lsa);
    tag.setLSAIndex((uint32_t)index);
    packet->AddPacketTag(tag);

    Ptr<Socket> m_socket = Socket::CreateSocket (ConfLoader::Instance()->getNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind ();
    m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
    m_socket->Send (packet);
    cout << m_id << " send LSA of " << node << endl;
    Dijkstra();
}

void Ipv4OSPFRouting::Dijkstra(){
    int source = m_id;
    int total = ConfLoader::Instance()->getTotalNum()+ConfLoader::Instance()->getToRNum();
    int dist[total];
    bool visited[total];
    int previous[total];
    for(int i=0; i<total; i++){
        dist[i] = 1000000;
        visited[i] = false;
        previous[i] = source;
    }
    dist[source] = 0;
    vector<int> Q;
    Q.push_back(source);
    while(Q.size()!=0){
        int u = 0;
        int min = 1000000;
        for(int i=0;i<total;i++){
            if(!visited[i]){
                if(dist[i]<min){
                    u = i;
                    min = dist[i];
                }
            }
        }
        Q.erase(std::remove(Q.begin(), Q.end(), u), Q.end());
        visited[u] = true;
        vector<uint16_t> neighbors =m_LSAs[u];
        int alt = dist[u]+1;
        for(vector<uint16_t>::iterator it = neighbors.begin(); it!=neighbors.end(); ++it){
            int v = (int)*it;
            if(alt<dist[v]&&!visited[v]){
                dist[v] = alt;
                previous[v] = u;
                Q.push_back(v);
            }
        }  
    }

    m_LinkStateDatabase.clear();
    for(int i=0;i<total;i++){
        if(previous[i] == source){
            m_LinkStateDatabase[i] = previous[i];
            continue;
        }
        while(find(m_LSAs[m_id].begin(), m_LSAs[m_id].end(), previous[i])==m_LSAs[m_id].end()){
            previous[i] = previous[previous[i]];
        }
        m_LinkStateDatabase[i] = previous[i];
    }


    //cout << "Link State of "<<m_id << endl;
    m_OSPFRoutingTable.clear();
    /*for(map<int,int>::iterator it = m_LinkStateDatabase.begin(); it!=m_LinkStateDatabase.end(); ++it){
        //Subnet subnet;
        //m_OSPFRoutingTable[subnet] = ConfLoader::Instance()->calcSourceInterfaceByNode(source, it->second);
        cout << it->first << " " << it->second << endl;
    }*/

    int tor = ConfLoader::Instance()->getToRNum();
    for(int i=0; i<tor; i++){
        int node = i+ConfLoader::Instance()->getTotalNum();
        int previous = m_LinkStateDatabase[node];
        if(previous == source){
            previous = node;
        }
        m_OSPFRoutingTable[ConfLoader::Instance()->getSubnetByNode(node)]
          = ConfLoader::Instance()->calcSourceInterfaceByNode(source, previous);
          // = ConfLoader::Instance()->calcSourceInterfaceByNode(source, node);
    }

    cout << "OSPFRoutingTable of "<<m_id << endl;
    for(map<Subnet, int>::iterator it=m_OSPFRoutingTable.begin(); it!=m_OSPFRoutingTable.end();++it){
        cout << it->first.toString() << " " <<it->second << endl;
    }
}

void Ipv4OSPFRouting::toString(vector<uint16_t>& v){
    for(int i=0; i< (int)v.size(); i++){
      cout << (int)v[i] << " ";
    }
    cout << endl;
}
void Ipv4OSPFRouting::handleMessage(Ptr<const Packet> packet){
          OSPFTag tag;
          uint8_t type;
          uint16_t from;
          bool found = packet->PeekPacketTag(tag);
          if (found){
            type = tag.getType();
            from = tag.getNode();
            //cout << from;
            if(type == 1){
                cout << "receive hello message" << endl;
                addToNeighbors(from, Simulator::Now());
            }else if(type == 2){
                cout << "receive update message from " << (int)from << endl;
                uint16_t lsa_node = tag.getLSANode();
                if((int)lsa_node != m_id){
                    uint32_t index = tag.getLSAIndex();
                    vector<uint16_t> lsa = ConfLoader::Instance()->getLSA(index);
                    if(m_LSAs.find((int)lsa_node)==m_LSAs.end()){
                        //cout << "not found index" << endl;
                        return sendLSAMessage(lsa_node, index);
                    }else{
                        vector<uint16_t> my = m_LSAs[(int)lsa_node];
                        if(my.size()!=lsa.size()){                        
                            //cout << "size not equal" << endl;
                            return sendLSAMessage(lsa_node, index);
                        }

                        for(vector<uint16_t>::iterator it = my.begin(); it != my.end(); ++it){
                          if(find(lsa.begin(), lsa.end(), *it)==lsa.end()){
                              //cout << "my not found in lsa" << endl;
                              //toString(my);
                              //toString(lsa);
                              return sendLSAMessage(lsa_node, index);
                          }
                        }

                        for(vector<uint16_t>::iterator it = lsa.begin(); it != lsa.end(); ++it){
                          if(find(my.begin(), my.end(), *it)==my.end()){
                              //cout << "lsa not found in my" << endl;
                              //toString(my);
                              //toString(lsa);
                              return sendLSAMessage(lsa_node, index);
                          }
                        }
                    }
                }
                //cout << (int)lsa_node << " has existed in " << m_id << " now has " << m_LSAs.size() << endl;
            }else{
                cout << "receive not-hello message" << endl;
            }
          }else{
              cout << "receive normal message" << endl;
              ConfLoader::Instance()->prepareLinkDown();
          }
}

void Ipv4OSPFRouting::checkNeighbors(){
    //cout << m_id << " now has " << m_LSAs.size() << endl;

    Time now = Simulator::Now();
    for(map<int, Time>::iterator it = m_CurNeighbors.begin(); it != m_CurNeighbors.end(); ++it){
        if(now - it->second > Seconds(ConfLoader::Instance()->getUnavailableInterval())){
            m_CurNeighbors.erase(it);
        }
    }
    bool toNotify = false;
    for(map<int, Time>::iterator it = m_CurNeighbors.begin(); it != m_CurNeighbors.end(); ++it){
        if(m_LastNeighbors.find(it->first) == m_LastNeighbors.end()){
            toNotify = true;
        }
    }

    for(map<int, Time>::iterator it = m_LastNeighbors.begin(); it != m_LastNeighbors.end(); ++it){
        if(m_CurNeighbors.find(it->first) == m_CurNeighbors.end()){
            toNotify = true;
        }
    }
    if(toNotify){
        vector<uint16_t> lsa;
        for(map<int, Time>::iterator it = m_CurNeighbors.begin(); it != m_CurNeighbors.end(); ++it){
            lsa.push_back((uint16_t)it->first);
        }
        cout << "neighbors change" << endl;
        int index = ConfLoader::Instance()->getLSANum();
        ConfLoader::Instance()->addLSA(index, lsa);
        sendLSAMessage(m_id, index);
    }
    m_LastNeighbors = m_CurNeighbors;
}

void Ipv4OSPFRouting::send2Peer(Ptr<Packet> packet){
}

void Ipv4OSPFRouting::sendMessage(Ipv4Address ip, Ptr<Packet> packet){
  /*Ptr<Socket> m_socket = Socket::CreateSocket (ConfLoader::Instance()->getNodeContainer().Get(m_id), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  m_socket->Bind ();
  m_socket->Connect (Address (InetSocketAddress (ip, 9)));
  m_socket->Send (packet);*/
}


Ptr<Ipv4Route> Ipv4OSPFRouting::LookupOSPFRoutingTable (Ipv4Address dest)
{
  NS_LOG_LOGIC ("Looking for route for destination " << dest);
  int out_interface = -1;
  for(map<Subnet, int>::iterator it = m_OSPFRoutingTable.begin(); it != m_OSPFRoutingTable.end(); ++it){
      if(it->first.contains(dest)){
          out_interface = it->second;
      }
  }
  if(out_interface == -1){
      cout << "No route found!" << endl;
      return 0;
  }
  int destNode = ConfLoader::Instance()->calcDestNodeBySource(m_id, out_interface);
  int destInterface = ConfLoader::Instance()->calcDestInterfaceBySource(m_id, out_interface);
  Ptr<Ipv4> to_ipv4 = ConfLoader::Instance()->getNodeContainer().Get(destNode)->GetObject<Ipv4OSPFRouting>()->getIpv4();
  cout << "Route from this node "<<m_id <<" on interface " << out_interface <<" to Node " << destNode << " on interface " << destInterface << endl;
  Ptr<Ipv4Route> rtentry = Create<Ipv4Route> ();
  rtentry->SetDestination (to_ipv4->GetAddress (destInterface, 0).GetLocal ());
  rtentry->SetSource (m_ipv4->GetAddress (out_interface, 0).GetLocal ());
  rtentry->SetGateway (Ipv4Address("0.0.0.0"));
  rtentry->SetOutputDevice (m_ipv4->GetNetDevice (out_interface));
  return rtentry;
}


Ptr<Ipv4Route>
Ipv4OSPFRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_FUNCTION (this << p << &header << oif << &sockerr);
  NS_LOG_DEBUG( Simulator::Now() << " " << m_id <<" send a packet\t"<< p << "\t" << header.GetSource() << "\t"<<header.GetDestination());
  cout << Simulator::Now() << " " << m_id <<" send a packet\t"<< p << "\t" << header.GetSource() << "\t"<<header.GetDestination() << endl;
  NS_LOG_LOGIC ("Unicast destination- looking up");
  Ptr<Ipv4Route> rtentry = LookupOSPFRoutingTable (header.GetDestination ());
  if (rtentry)
    {
      sockerr = Socket::ERROR_NOTERROR;
    }
  else
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
    }
  return rtentry;
}

Ptr<Ipv4> Ipv4OSPFRouting::getIpv4(){
  return m_ipv4;
}

bool 
Ipv4OSPFRouting::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,                             
                UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                LocalDeliverCallback lcb, ErrorCallback ecb)
{ 
  NS_LOG_FUNCTION (this << p << header << header.GetSource () << header.GetDestination () << idev << &lcb << &ecb);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  NS_LOG_DEBUG( Simulator::Now() << " " << m_id <<" receive a packet\t"<< p << "\t" << header.GetSource() << "\t"<<header.GetDestination() );
  cout << Simulator::Now() << " " << m_id <<" receive a packet\t"<< p << "\t" << header.GetSource() << "\t"<<header.GetDestination() << endl;
  for (uint32_t j = 0; j < m_ipv4->GetNInterfaces (); j++)
    {
      for (uint32_t i = 0; i < m_ipv4->GetNAddresses (j); i++)
        {
          Ipv4InterfaceAddress iaddr = m_ipv4->GetAddress (j, i);
          Ipv4Address addr = iaddr.GetLocal ();
          if (addr.IsEqual (header.GetDestination ()))
            {
              if (j == iif)
                {
                  NS_LOG_LOGIC ("For me (destination " << addr << " match)");
                }
              else
                {
                  NS_LOG_LOGIC ("For me (destination " << addr << " match) on another interface " << header.GetDestination ());
                }
              lcb (p, header, iif);
              cout << "destination match!" <<endl;
              return true;
            }
          if (header.GetDestination ().IsEqual (iaddr.GetBroadcast ()))
            {
              NS_LOG_LOGIC ("For me (interface broadcast address)");
              lcb (p, header, iif);
              return true;
            }
          NS_LOG_LOGIC ("Address "<< addr << " not a match");
        }
    }
    if (m_ipv4->IsForwarding (iif) == false)
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      return false;
    }
  // Next, try to find a route
  NS_LOG_LOGIC ("Unicast destination- looking up global route");
  Ptr<Ipv4Route> rtentry = LookupOSPFRoutingTable (header.GetDestination ());
  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Found unicast destination- calling unicast callback");
      ucb (rtentry, p, header);
      return true;
    }
  else
    {
      NS_LOG_LOGIC ("Did not find unicast destination- returning false");
      return false; // Let other routing protocols try to handle this
                    // route request.
    }
}

void 
Ipv4OSPFRouting::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
    }
}

void 
Ipv4OSPFRouting::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << i);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
    }
}

void 
Ipv4OSPFRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
    }
}

void 
Ipv4OSPFRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  if (m_respondToInterfaceEvents && Simulator::Now ().GetSeconds () > 0)  // avoid startup events
    {
    }
}

void 
Ipv4OSPFRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;
}


} // namespace ns3

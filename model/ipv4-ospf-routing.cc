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

#include <sstream>

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

string Ipv4OSPFRouting::toString(){
    stringstream result;
    result << m_id << "\tRoutingTable:" << endl;
    for(map<Subnet, int>::iterator it = m_OSPFRoutingTable.begin(); it != m_OSPFRoutingTable.end(); ++it){
        result << it->first.toString() << "\t" << it->second << "\n";
    }
    return result.str();
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
      return 0;
  }
  int destNode = ConfLoader::Instance()->calcDestNodeBySource(m_id, out_interface);
  int destInterface = ConfLoader::Instance()->calcDestInterfaceBySource(m_id, out_interface);
  Ptr<Ipv4> to_ipv4 = ConfLoader::Instance()->getNodeContainer().Get(destNode)->GetObject<Ipv4OSPFRouting>()->getIpv4();

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

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ipv4-ospf-routing-helper.h"
#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE ("OSPFRoutingHelper");

namespace ns3 {

Ipv4OSPFRoutingHelper::Ipv4OSPFRoutingHelper ()
{
}

Ipv4OSPFRoutingHelper::Ipv4OSPFRoutingHelper (const Ipv4OSPFRoutingHelper &o)
{
}

Ipv4OSPFRoutingHelper*
Ipv4OSPFRoutingHelper::Copy (void) const
{
  return new Ipv4OSPFRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
Ipv4OSPFRoutingHelper::Create (Ptr<Node> node) const
{
  int id = node->GetId();

  //NS_LOG_LOGIC ("Adding OSPFRouter interface to node " << id);

  //Ptr<OSPFRouter> router = CreateObject<OSPFRouter> ();
  //node->AggregateObject (router);
  
  //mOSPFGlobalInfo.
  NS_LOG_LOGIC ("Adding OSPFRouting Protocol to node " << id);
  Ptr<Ipv4OSPFRouting> routing = CreateObject<Ipv4OSPFRouting> ();
  routing->setID(id);
  //router->setID(id);
  //router->SetRoutingProtocol (routing);
  map<Subnet, int> mOSPFRoutingTable;
  Subnet subnet(0x0a000000, 24);
  //mOSPFRoutingTable.insert(pair<Subnet, int>(subnet, id));
  //mOSPFRoutingTable[subnet] = id;
  routing->SetOSPFRoutingTable(mOSPFRoutingTable);
  //routing->GetOSPFRoutingTable()[subnet] = id;
  //ConfLoader::Instance()->UpdateOSPFGrid(node->GetId(), mOSPFRoutingTable);
  //node->GetObject<OSPFRouter>()->GetRoutingProtocol()->SetOSPFRoutingTable (mOSPFRoutingTable);
  node->AggregateObject(routing);
  return routing;
}

void 
Ipv4OSPFRoutingHelper::PopulateRoutingTables (void)
{
}
void 
Ipv4OSPFRoutingHelper::RecomputeRoutingTables (void)
{
}

uint32_t
Ipv4OSPFRoutingHelper::AllocateRouterId (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static uint32_t routerId = 0;
  return routerId++;
}

} // namespace ns3

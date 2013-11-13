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
#ifndef IPV4_OSPF_ROUTING_HELPER_H
#define IPV4_OSPF_ROUTING_HELPER_H

#include <map>
#include <iostream>

#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

#include "ns3/ipv4-ospf-routing.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/conf-loader.h"

using namespace std;

namespace ns3 {

/**
 * \brief Helper class that adds ns3::Ipv4OSPFRouting objects
 */
class Ipv4OSPFRoutingHelper  : public Ipv4RoutingHelper
{
public:
  /**
   * \brief Construct a OSPFRoutingHelper to make life easier for managing
   * OSPF routing tasks.
   */
  Ipv4OSPFRoutingHelper ();
  /**
   * \brief Construct a OSPFRoutingHelper from another previously initialized
   * instance (Copy Constructor).
   */
  Ipv4OSPFRoutingHelper (const Ipv4OSPFRoutingHelper &);

  /**
   * \internal
   * \returns pointer to clone of this Ipv4OSPFRoutingHelper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  Ipv4OSPFRoutingHelper* Copy (void) const;

  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  /**
   * \brief Build a routing database and initialize the routing tables of
   * the nodes in the simulation.  Makes all nodes in the simulation into
   * routers.
   *
   * All this function does is call the functions
   * BuildOSPFRoutingDatabase () and  InitializeRoutes ().
   *
   */
  static void PopulateRoutingTables (void);
  /**
   * \brief Remove all routes that were previously installed in a prior call
   * to either PopulateRoutingTables() or RecomputeRoutingTables(), and 
   * add a new set of routes.
   * 
   * This method does not change the set of nodes
   * over which OSPFRouting is being used, but it will dynamically update
   * its representation of the OSPF topology before recomputing routes.
   * Users must first call PopulateRoutingTables() and then may subsequently
   * call RecomputeRoutingTables() at any later time in the simulation.
   *
   */
  static void RecomputeRoutingTables (void);

  static uint32_t AllocateRouterId ();
  //void UpdateOSPFGrid(int id, Ptr<OSPFGrid> mOSPFGrid){

  //void CreateOSPFGrid(Ptr<Node> node) const;
private:
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  Ipv4OSPFRoutingHelper &operator = (const Ipv4OSPFRoutingHelper &o);



};

} // namespace ns3

#endif /* IPV4_OSPF_ROUTING_HELPER_H */

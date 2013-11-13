#ifndef CONF_LOADER_H
#define CONF_LOADER_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <map>

#include "ns3/node-container.h"
#include "ns3/subnet.h"
#include "ns3/srp-router-interface.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

using namespace std;

namespace ns3{

class NodeContainer;
class SRPGrid;
class Ipv4Address;

class ConfLoader
{
public:
  static ConfLoader* Instance();
  void setCoreNum(int num);
  void setToRNum(int num);
  void setBorderNum(int num);
  void setSubnetMask(int mask);
  void setAddressStart(uint32_t address);
  int getCoreNum() const;
  int getToRNum() const;
  int getBorderNum() const;
  int getSubnetMask() const;
  int getTotalNum() const;

  uint32_t getAddressStart() const;


  map<int, Subnet>& getIndexSubnetMap();
  void addItem2IndexSubnetMap(int index, Subnet& subnet);
  Ipv4Address getIpv4ByIndex(int index);

  void setNodeContainer(NodeContainer& nc);
  NodeContainer& getNodeContainer();
  int getInterfaceIndex(int my, int to);

  map<int, bool>& getNodeStates();
  void setNodeStates(map<int, bool>& states);
  void setNodeState(int i, bool state);
  bool getNodeState(int i);
  bool getLinkState(int i, int j);

  map<pair<int,int>,bool>& getLinkStates();
  void setLinkStates(map<pair<int,int>,bool>& states);
  void setLinkState(int i, int j, bool state);

  /*map<int, bool> getNodeActions();
  void setNodeActions(map<int, bool> actions);
  bool getNodeAction(int i);

  map<pair<int,int>,bool> getLinkActions();
  void setLinkActions(map<pair<int,int>,bool> actions);
  bool getLinkAction(int i, int j);

  vector<int> getLinkAction(int i);

  void clearNodeActions();
  void clearLinkActions();*/

  void UpdateSRPGrid(int id, Ptr<SRPGrid> mSRPGrid);
  int getIndexBySubnet(Subnet& subnet);
  Subnet& getSubnetByID(int id);

  map<Ipv4Address, int>& getIpv4IndexMap();
  void setIpv4IndexMap(map<Ipv4Address, int>& m_map);
  void addItem2Ipv4IndexMap(Ipv4Address& ip, int index);
  int getIndexByIpv4(Ipv4Address& ip);

  string getUpdateMsgString(){return UPDATE_MSG;};

  void incrementLossPacketCounter(){ this->m_lossPacketCounter++;};
  int getLossPacketCounter(){ return this->m_lossPacketCounter;};

  void setCurrentTime(Time time){
    m_stopTime = time;
    if(m_startTime.IsZero()){
        m_startTime = time;
    }
  };

  Time getDiffTime(){
      return m_stopTime - m_startTime;
  }

private:

  string UPDATE_MSG;

	ConfLoader(){
    UPDATE_MSG = "update!";
    m_lossPacketCounter = 0;
  };
	ConfLoader(ConfLoader const&){};
	//ConfLoader& operator=(ConfLoader const&){};
	static ConfLoader* m_pInstance;

  enum NodeType{
      CORE = 0,
      TOR,
      BORDER,
  };

  map<int, Subnet> index_subnet_map;

  map<Ipv4Address, int> m_ipv4_index_map;
  
  map<int, bool> nodeStates;
  map<pair<int,int>,bool> linkStates;

  //map<int, bool> nodeActions;
  //map<pair<int,int>,bool> linkActions;

  int m_CoreNum;
  int m_ToRNum;
  int m_BorderNum;
  int m_SubnetMask;
  uint32_t m_AddressStart;

  int m_lossPacketCounter;
  Time m_startTime;
  Time m_stopTime;

  NodeContainer m_nodes;

};

}

#endif /* CONF_LOADER_H */
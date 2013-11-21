#ifndef CONF_LOADER_H
#define CONF_LOADER_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <map>

#include "ns3/node-container.h"
#include "ns3/subnet.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

using namespace std;

namespace ns3{

class NodeContainer;
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


  map<pair<int,int>, Subnet>& getLinkSubnetMap();
  void addItem2LinkSubnetMap(int index1, int index2, Subnet& subnet);
  pair<int,int> getLinkBySubnet(Subnet& subnet);
  Subnet& getSubnetByID(int index1, int index2);

  void setNodeContainer(NodeContainer& nc);
  NodeContainer& getNodeContainer();

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

  int calcDestNodeBySource(int id, int interface);
  int calcDestInterfaceBySource(int id, int interface);

  vector<Subnet>& getSubnets(){
      return m_Subnets;
  }

  void addSubnet(Subnet& subnet){
      m_Subnets.push_back(subnet);
  }

  void update(int id, map<Subnet, int>& table);

  string getHelloMsgString(){
      return string("hello");
  }

  void setUnavailableInterval(int UnavailableInterval){
      this->m_UnavailableInterval = UnavailableInterval;
  };

  int getUnavailableInterval(){
      return this->m_UnavailableInterval;
  };
private:

	ConfLoader(){
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

  map<pair<int,int>, Subnet> m_LinkSubnet;
  std::vector<Subnet> m_Subnets;

  int m_UnavailableInterval;
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
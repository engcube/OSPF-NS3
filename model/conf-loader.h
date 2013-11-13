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


  map<int, Subnet>& getIndexSubnetMap();
  void addItem2IndexSubnetMap(int index, Subnet& subnet);


  void setNodeContainer(NodeContainer& nc);
  NodeContainer& getNodeContainer();

  int getIndexBySubnet(Subnet& subnet);
  Subnet& getSubnetByID(int id);


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

  int calcDestNodeBySource(int id, int interface);
  int calcDestInterfaceBySource(int id, int interface);

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
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

  void incrementLossPacketCounter(){ 
    if(!isDown){
        return;
    }
    this->m_lossPacketCounter++;
  };

  void prepareLinkDown(){
      if(isDown){
          return;
      }
      isDown = true;
      cout << "Lost packets: " << m_lossPacketCounter << endl;
      cout << "Duration: " <<  m_startTime << " to " << m_stopTime << endl;
      this->m_lossPacketCounter=0;
  }
  int getLossPacketCounter(){ return this->m_lossPacketCounter;};

  void setCurrentTime(Time time){
    if(!isDown){ return;}
    m_stopTime = time;
    if(m_startTime.IsZero()){
        m_startTime = time;
    }
  };

  Time getDiffTime(){
      return m_stopTime - m_startTime;
  };

  Time& getStartTime(){
    return m_startTime;
  };

  Time& getStopTime(){ return m_stopTime;};

  int calcDestNodeBySource(int id, int interface);
  int calcDestInterfaceBySource(int id, int interface);
  int calcSourceInterfaceByNode(int id, int node);
  Subnet& calcSubnetByNode(int node);

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

  void addToNodeSubnet(int id, Subnet& subnet){
      m_NodeSubnets[id] = subnet;
  };

  Subnet& getSubnetByNode(int id){
      return m_NodeSubnets[id];
  };

  void addLSA(int index, vector<uint16_t>& lsa){ 
    m_lsas[index] = lsa;
    m_lsaNum ++;
  };

  int getLSANum(){return m_lsaNum;};
  
  vector<uint16_t>& getLSA(int index){ return m_lsas[index];};

  void incrementSuccessPacket(){m_SuccessPacket++;};
  int getSuccessPacket(){return m_SuccessPacket;};
  void incrementSendPacket(){m_SendPacket++;};
  int getSendPacket(){return m_SendPacket;};

  int getNodeByInterface(int id, int interface){
      if(id < m_CoreNum){
          return m_CoreNum+interface-1;
      }else if(id<m_CoreNum+m_ToRNum+m_BorderNum){
          if(interface==m_CoreNum+1) return id+m_ToRNum+m_BorderNum;
          else return interface-1;
      }else{
        return id - (m_ToRNum+m_BorderNum);
      }
      return -1;
  };
  
private:

	ConfLoader(){
    m_lossPacketCounter = 0;
    isDown = false;
    m_lsaNum = 0;
    m_SuccessPacket = 0;
    m_SendPacket = 0;
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
  map<int, Subnet> m_NodeSubnets;

  int m_UnavailableInterval;
  int m_CoreNum;
  int m_ToRNum;
  int m_BorderNum;
  int m_SubnetMask;
  uint32_t m_AddressStart;

  bool isDown;
  int m_lossPacketCounter;
  Time m_startTime;
  Time m_stopTime;

  int m_SuccessPacket;
  int m_SendPacket;

  NodeContainer m_nodes;

  map<int, vector<uint16_t> > m_lsas;
  int m_lsaNum;
};

}

#endif /* CONF_LOADER_H */
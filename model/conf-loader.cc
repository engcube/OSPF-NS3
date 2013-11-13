#include <fstream>
#include <iostream>

#include "conf-loader.h"
//#include "ns3/log.h"


//NS_LOG_COMPONENT_DEFINE ("ConfLoader");

namespace ns3{

using namespace std;

ConfLoader* ConfLoader::m_pInstance = NULL;

ConfLoader* ConfLoader::Instance(){
	if(!m_pInstance){
		m_pInstance = new ConfLoader;
	}
	return m_pInstance;
}

void ConfLoader::setCoreNum(int num){
    m_CoreNum = num;
}
void ConfLoader::setToRNum(int num){
    m_ToRNum = num;
}
void ConfLoader::setBorderNum(int num){
    m_BorderNum = num;
}
void ConfLoader::setSubnetMask(int mask){
    m_SubnetMask = mask;
}
void ConfLoader::setAddressStart(uint32_t address){
    m_AddressStart = address;
}
int ConfLoader::getCoreNum() const{
    return m_CoreNum;
}
int ConfLoader::getToRNum() const{
    return m_ToRNum;
}
int ConfLoader::getBorderNum() const{
    return m_BorderNum;
}
int ConfLoader::getSubnetMask() const{
    return m_SubnetMask;
}
uint32_t ConfLoader::getAddressStart() const{
    return m_AddressStart;
}

int ConfLoader::getTotalNum() const{
    return m_CoreNum+m_ToRNum+m_BorderNum;
}


map<int, Subnet>& ConfLoader::getIndexSubnetMap(){
  return index_subnet_map;
}

void ConfLoader::addItem2IndexSubnetMap(int index, Subnet& subnet){
  index_subnet_map[index] = subnet;
}

void ConfLoader::setNodeContainer(NodeContainer& nc){
    m_nodes = nc;
}

NodeContainer& ConfLoader::getNodeContainer(){
    return m_nodes;
}


int ConfLoader::getIndexBySubnet(Subnet& subnet){
    for(map<int, Subnet>::iterator it = index_subnet_map.begin(); it!=index_subnet_map.end(); ++it){
        if (it->second.equals(subnet)){
            return it->first;
        }
    }
    return -1;
}



Subnet& ConfLoader::getSubnetByID(int id){
    return index_subnet_map[id];
}


//----------------------

int ConfLoader::calcDestNodeBySource(int id, int interface){
    if(id < m_CoreNum){
        return m_CoreNum+interface-1;
    }else if(id < getTotalNum()){
        if(interface<=m_CoreNum){
            return interface-1;
        }else{
            return id+m_ToRNum+m_BorderNum;
        }
    }else{
        return id - (m_ToRNum+m_BorderNum);
    }
}

int ConfLoader::calcDestInterfaceBySource(int id, int interface){
    if(id < m_CoreNum){
        return id+1;
    }else if(id < getTotalNum()){
        if(interface<=m_CoreNum){
            return id-m_CoreNum+1;
        }else{
            return 1;
        }
    }else{
        return m_CoreNum+1;
    }
}

}

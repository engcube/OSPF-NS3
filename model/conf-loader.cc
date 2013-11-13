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
    cout << "Core " << m_CoreNum << endl;
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

int ConfLoader::getInterfaceIndex(int my, int to){
    if(my<m_CoreNum){
        if(to<m_CoreNum){
            return 0;
        }else{
            return to-m_CoreNum+1;
        }
    }else{
        if(to>=m_CoreNum){
            return 0;
        }else{
            return to+1;
        }
    }
    return 0;
}

int ConfLoader::getIndexBySubnet(Subnet& subnet){
    for(map<int, Subnet>::iterator it = index_subnet_map.begin(); it!=index_subnet_map.end(); ++it){
        if (it->second.equals(subnet)){
            return it->first;
        }
    }
    return -1;
}

map<int, bool>& ConfLoader::getNodeStates(){
    return this->nodeStates;
}

void ConfLoader::setNodeStates(map<int, bool>& states){
    this->nodeStates = states;
}

map<pair<int,int>,bool>& ConfLoader::getLinkStates(){
    return this->linkStates;
}

void ConfLoader::setLinkStates(map<pair<int,int>,bool>& states){
    this->linkStates = states;
}

void ConfLoader::setLinkState(int i, int j, bool state){
    if(i>j){
        this->linkStates[make_pair(j,i)] = state;
    }
    else{
        this->linkStates[make_pair(i,j)] = state;
    }
}


/*map<int, bool> ConfLoader::getNodeActions(){
    return this->nodeActions;
}

bool ConfLoader::getNodeAction(int i){
    return this->nodeActions[i];
}*/

void ConfLoader::setNodeState(int i, bool state){
    this->nodeStates[i] = state;
}

bool ConfLoader::getNodeState(int i){
    return this->nodeStates[i];
}

bool ConfLoader::getLinkState(int i, int j){
    return this->linkStates[make_pair(i,j)];
}

/*bool ConfLoader::getLinkAction(int i, int j){
    return this->linkActions[make_pair(i,j)];
}

vector<int> ConfLoader::getLinkAction(int i){
    vector<int> result;
    for(int k=0; k<getTotalNum(); k++){
        pair<int,int> tmp(i,k);
        //if (this->linkActions.find(tmp)){
            result.push_back(k);
            //this->linkActions.erase(tmp);
        //}
    }
    return result;
}


void ConfLoader::setNodeActions(map<int, bool> actions){
    this->nodeActions = actions;
}

map<pair<int,int>,bool> ConfLoader::getLinkActions(){
    return this->linkActions;
}

void ConfLoader::setLinkActions(map<pair<int,int>,bool> actions){
    this->linkActions = actions;
}

void ConfLoader::clearNodeActions(){
    this->nodeActions.clear();
}

void ConfLoader::clearLinkActions(){
    this->linkActions.clear();
}*/

map<Ipv4Address, int>& ConfLoader::getIpv4IndexMap(){
    return m_ipv4_index_map;
}

void ConfLoader::setIpv4IndexMap(map<Ipv4Address, int>& m_map){
    m_ipv4_index_map = m_map;
}

void ConfLoader::addItem2Ipv4IndexMap(Ipv4Address& ip, int index){
    m_ipv4_index_map[ip] = index;
}

Subnet& ConfLoader::getSubnetByID(int id){
    return index_subnet_map[id];
}


int ConfLoader::getIndexByIpv4(Ipv4Address& ip){
    return m_ipv4_index_map[ip];
}

Ipv4Address ConfLoader::getIpv4ByIndex(int index){
    for(map<Ipv4Address, int>::iterator it = m_ipv4_index_map.begin(); it!=m_ipv4_index_map.end(); ++it){
        if (it->second == index){
            return it->first;
        }
    }
    return NULL;
}

void ConfLoader::UpdateSRPGrid(int id, Ptr<SRPGrid> mSRPGrid){
  //Ptr<SRPGrid> mSRPGrid = node->GetObject<SRPRouter>()->GetRoutingProtocol()->GetSRPGrid();
  //cout << id << ":status:" << this->nodeStates[id] << endl;
  if(this->nodeStates[id]){
    
    if( id < m_CoreNum){  //Core
        for(int i = m_CoreNum; i< m_CoreNum+m_ToRNum; i++){
            map<int, int> mmap;
            if(this->nodeStates[i] && getLinkState(id,i)){
                mmap[i] = 1;
            }else{
                mmap[i] = 0;
            }
            SRPRoutingEntry entry(index_subnet_map[i], mmap);
            mSRPGrid->addSRPGridEntry(entry);
        }
        map<int, int> mmap;
        for(int i = m_CoreNum+m_ToRNum; i<getTotalNum(); i++){
            if(this->nodeStates[i] && getLinkState(id,i)){
              mmap[i] = 1;
            }else{
                mmap[i]=0;
            }
        }
        Subnet subnet(0,0);
        SRPRoutingEntry entry(subnet, mmap);
        entry.setDescription("B_exit");
        mSRPGrid->addSRPGridEntry(entry);
/*
        for(map<Ipv4Address, int>::iterator it = m_ipv4_index_map.begin(); it!=m_ipv4_index_map.end();++it){
            map<int, int> mmap;
            if(it->second == id){
                mmap[id] = 1;
            }else{
                for(int i = m_CoreNum; i< m_CoreNum+m_ToRNum; i++){
                    if(this->nodeStates[i] && getLinkState(id,i)){
                        mmap[i] = 1;
                    }else{
                        mmap[i] = 0;
                    }
                }
                for(int i = m_CoreNum+m_ToRNum; i< getTotalNum(); i++){
                    if(this->nodeStates[i] && getLinkState(id,i)){
                        mmap[i] = 3;
                    }else{
                        mmap[i] = 2;
                    }
                }
            }
            SRPRoutingEntry entry(it->first, mmap);
            mSRPGrid->addSRPGridEntry(entry);
        }
*/
    }
    else if( id < m_CoreNum+m_ToRNum){ //ToR

        for(int i= m_CoreNum; i< m_CoreNum+m_ToRNum; i++){
           if(i==id){
              continue;
           }
           map<int, int> mmap;
           for(int j=0; j < m_CoreNum; j++){
            if(this->nodeStates[j] && getLinkState(j,id) && this->nodeStates[i] && getLinkState(j,i)){
                mmap[j] = 1;
            }else{
                mmap[j]=0;
            }
           }

           SRPRoutingEntry entry(index_subnet_map[i], mmap);
           mSRPGrid->addSRPGridEntry(entry);
        }
        map<int, int> mmap;
        for(int j=0; j < m_CoreNum; j++){
          if(this->nodeStates[j] && getLinkState(j,id)){
              mmap[j] = 1;
            }
            else{
                mmap[j]=0;
            }
        }
        Subnet subnet(0,0);
        SRPRoutingEntry entry(subnet, mmap);
        entry.setDescription("B_exit");
        mSRPGrid->addSRPGridEntry(entry);

    }else { //NodeType.BORDER
        for(int i= m_CoreNum; i< m_CoreNum+m_ToRNum; i++){
           map<int, int> mmap;
           for(int j=0; j < m_CoreNum; j++){
              if(this->nodeStates[j]&& getLinkState(j,id) && this->nodeStates[i]&& getLinkState(j,i)){
                mmap[j] = 1;
              }else{
                mmap[j]=0;
              }
           }
           
           for(int j=m_CoreNum+m_ToRNum; j< getTotalNum(); j++){
                if(j==id){
                    continue;
                }
                mmap[j] = 2;
                for(int k=0; k < m_CoreNum; k++){
                    if(this->nodeStates[k]&& getLinkState(k,id)){
                        mmap[j] = 3;
                    }else{
                        mmap[j] = 2;
                    }
                }
                if(mmap[j]==3){
                    if(j<id && this->nodeStates[j]&& getLinkState(j,id)){
                        mmap[j] = 3;
                    }else if(j<id){
                        mmap[j] = 2;
                    }else if(j>id && this->nodeStates[j]&& getLinkState(id,j)){
                        mmap[j] = 3;
                    } else if(j>id){
                        mmap[j] = 2;
                    }
                }           
                //cout << mmap[j] << endl;
           }
           SRPRoutingEntry entry(index_subnet_map[i], mmap);
           mSRPGrid->addSRPGridEntry(entry);
        }
    }
  }
}

}

#ifndef OSPF_TAG_H
#define OSPF_TAG_H

#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

namespace ns3{

class OSPFTag : public Tag
{
public:
   OSPFTag(){this->type = 0;};
   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual uint32_t GetSerializedSize (void) const;
   virtual void Serialize (TagBuffer i) const;
   virtual void Deserialize (TagBuffer i);
   virtual void Print (std::ostream &os) const;

   // these are our accessors to our tag structure

   void setType(uint8_t type){this->type = type;};
   uint8_t getType(){return this->type;};

   void setNode(uint16_t node){this->node = node;};
   uint16_t getNode(){return this->node;};  

   void setLSA(int node, vector<int>& lsa){
      this->lsa_node = node;
      this->lsa = lsa;
   }
private:
   //1 -- hello
   uint8_t type;
   uint16_t node;
   uint16_t lsa_node;
   vector<int> lsa;
};

}
#endif

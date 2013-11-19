#include "ospf-tag.h"
#include "ns3/log.h"

using namespace ns3; 

NS_LOG_COMPONENT_DEFINE ("OSPFTag");

NS_OBJECT_ENSURE_REGISTERED (OSPFTag);


TypeId OSPFTag::GetTypeId (void){
   static TypeId tid = TypeId ("ns3::OSPFTag")
                   .SetParent<Tag> ()
                   .AddConstructor<OSPFTag> ();
           return tid;
}

TypeId OSPFTag::GetInstanceTypeId (void) const{
    return GetTypeId ();
}

uint32_t OSPFTag::GetSerializedSize (void) const{
    return 1;
}

void OSPFTag::Serialize (TagBuffer i) const{    
  i.WriteU8(type);
}

void OSPFTag::Deserialize (TagBuffer i){
    type = i.ReadU8();
}

void OSPFTag::Print (std::ostream &os) const{
    os << endl;
}

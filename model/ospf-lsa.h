#ifndef OSPF_LSA_H
#define OSPF_LSA_H

#include <iostream>
#include <vector>

using namespace std;

namespace ns3{

class OSPFLSA : public Tag
{
public:
   OSPFLSA(){};
   
private:
   int m_id;
   vector<int> neighbors;
};

}
#endif

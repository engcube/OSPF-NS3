#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>
#include <algorithm>

using namespace std;

int CORE_NUM = 4;
int TOR_NUM = 512;
int BORDER_NUM = 2;

int total = CORE_NUM+TOR_NUM+BORDER_NUM+TOR_NUM;

int m_id = 6;

map<int, vector<int> > m_OSPFRoutingTable;

map<int, vector<int> > m_LinkStateDatabase;
map<int, vector<int> > m_LSAs;


int calcSourceInterfaceByNode(int id, int node){
    if (id==node) return 0;
    if(id < CORE_NUM){
        return node-CORE_NUM+1;
    }else if(id < CORE_NUM+TOR_NUM+BORDER_NUM){
        if(node-id == TOR_NUM+BORDER_NUM){
            return CORE_NUM+1;
        }else{
            return node+1;
        }
    }else{
        return 1;
    }
}

vector<int> addNode(int tmp, vector< vector<int> > data){
    std::vector<int> result;

    if(find(m_LSAs[m_id].begin(), m_LSAs[m_id].end(), tmp)==m_LSAs[m_id].end()){
        for(int l = 0; l < data[tmp].size(); l++){
            int tt = data[tmp][l];
            vector<int> r = addNode(tt, data);
            for(int i=0; i<r.size(); i++){
                result.push_back(r[i]);
            }
        }
    }else{
        result.push_back(tmp);
    }
    return result;
}

void Dijkstra(){
    int source = m_id;
    int dist[total];
    bool visited[total];
    vector< vector<int> > previous(total);
    for(int i=0; i<total; i++){
        dist[i] = 1000000;
        visited[i] = false;
        //previous[i].push_back(source);
    }
    dist[source] = 0;
    vector<int> Q;
    Q.push_back(source);
    while(Q.size()!=0){
        int u = 0;
        int min = 1000000;
        for(int i=0;i<total;i++){
            if(!visited[i]){
                if(dist[i]<min){
                    u = i;
                    min = dist[i];
                }
            }
        }
        Q.erase(std::remove(Q.begin(), Q.end(), u), Q.end());
        visited[u] = true;
        vector<int> neighbors = m_LSAs[u];
        int alt = dist[u]+1;
        for(vector<int>::iterator it = neighbors.begin(); it!=neighbors.end(); ++it){
            int v = *it;
            if(!visited[v]){
                //previous[v].clear();
                if(alt<dist[v]){
                    dist[v] = alt;
                    previous[v].push_back(u);
                    Q.push_back(v);
                }else if(alt==dist[v]){
                    previous[v].push_back(u);
                    if(find(Q.begin(), Q.end(), v)==Q.end()){
                      Q.push_back(v);
                    }
                }
            }
        }  
    }

    m_LinkStateDatabase.clear();
    for(int i=0;i<total;i++){
        std::vector<int> v;
        for(int j = 0; j < previous[i].size(); ++j){
            if(previous[i][j] == source){
                v.push_back(previous[i][j]);
                continue;
            }
            int tmp = previous[i][j];
            vector<int> result = addNode(tmp,previous);
            for(int l =0 ; l<result.size();++l){
                v.push_back(result[l]);
            }
        }
        m_LinkStateDatabase[i] = v;
    }

    m_OSPFRoutingTable.clear();

    int tor = TOR_NUM;
    for(int i=0; i<tor; i++){
        int node = i+CORE_NUM+TOR_NUM+BORDER_NUM;
        vector<int> previous = m_LinkStateDatabase[node];

        std::vector<int> v;
        for(int j=0; j<previous.size(); j++){ 
            if(previous[j] == source){
                previous[j] = node;
            }
            //cout << previous[i] << endl;
            v.push_back(calcSourceInterfaceByNode(source, previous[j]));
        }
        m_OSPFRoutingTable[node] = v;
    }
}


void initLSAs(){
    for(int i=0; i<total; i++){
        std::vector<int> lsa;
        if(i<CORE_NUM){
            for(int j=CORE_NUM; j<CORE_NUM+TOR_NUM+BORDER_NUM; j++){
                lsa.push_back(j);
            }
        }else if(i<CORE_NUM+TOR_NUM+BORDER_NUM){
            for(int j=0; j<CORE_NUM;j++){
                lsa.push_back(j);
            }
            if(i<CORE_NUM+TOR_NUM){
                lsa.push_back(i+BORDER_NUM+TOR_NUM);
            }
        }else{
            lsa.push_back(i-(BORDER_NUM+TOR_NUM));
        }
        m_LSAs[i] = lsa;
        /*for(std::vector<int>::iterator it = lsa.begin(); it!=lsa.end(); ++it){
            ConfLoader::Instance()->getNodeContainer().Get(i)->GetObject<Ipv4OSPFRouting>()->addToNeighbors((int)(*it), Simulator::Now());
        }*/
    }
}

/*string toGridString(){
    stringstream result;
    result << m_id << "\tGrid:" << endl;
    for(map<int, map<int,int> >::iterator it = m_Grid.begin(); it != m_Grid.end(); ++it){
        result << it->first << ":\t\t";
        for(map<int,int>::iterator it2 = it->second.begin(); it2!=it->second.end(); ++it2){
            result << it2->first << ":" << it2->second << "\t";
        }
        result << endl;
    }
    return result.str();
}*/


string toString(){
    stringstream result;
    result << m_id << "\tRoutingTable:" << endl;
    for(map<int, vector<int> >::iterator it = m_OSPFRoutingTable.begin(); it != m_OSPFRoutingTable.end(); ++it){
        std::vector<int> v = it->second;
        for(std::vector<int>::iterator it2 = v.begin(); it2!=v.end(); ++it2){
            result << it->first << "\t" << *it2 << "\n";
        }
    }
    return result.str();
}

int main(){
    clock_t start,stop;
	//time_t start,stop;
	initLSAs();
    cout << "LSA number :" << m_LSAs.size() << endl;
    //start = time(NULL);
	start = clock();
    Dijkstra();
    //cout << toString() << endl;

    stop = clock();
    //stop = time(NULL);
    cout << "Ticks:" <<stop-start << endl;
    cout << CLOCKS_PER_SEC << endl;
    cout << (stop-start)*1.0/CLOCKS_PER_SEC << endl;
	//cout << toString() << endl;
}

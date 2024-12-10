//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    std::string path2 = "../simulations/inputs/coordinator.txt";

    auto res = readLine(path2);
    std::string node_id = res.first;
    std::string start_time = res.second;
    std::cout <<"nodeid: "<< node_id << " "<<"starttime: "<<start_time<<"\n";

    if (node_id=="0"){
        cMessage *msg=new cMessage("0");
        std::cout<<node_id<<"\n";
        send(msg,"ports$o",0);
    }else{
        cMessage *msg=new cMessage("1");
        std::cout<<node_id<<"\n";
        send(msg,"ports$o",1);
    }


}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}


std::pair<std::string, std::string>Coordinator:: readLine(const std::string& path) {
    std::ifstream file(path);
    std::string node_id;
    std::string start_time;
    std::string line;
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file.");
    }
    while (std::getline(file, line)) {
        node_id = line[0];
        start_time = line[2];
    }
    file.close();
    return std::make_pair(node_id, start_time);
}

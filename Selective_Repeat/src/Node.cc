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

#include "Node.h"

Define_Module(Node);
//Sender variable
int WS;
int sender_start_index=0;
int sender_end_index=WS-1;
int sender_index=0;
std::vector<Frame_Base*> sender_buffer(WS);
int ack_expected=0;
int next_frame_to_send=0;
bool no_nack=true;

//Receiver Variable
int receiver_start_index=0;
int receiver_end_index=WS-1;
int receiver_index=0;
std::vector<Frame_Base*> receiver_buffer(WS);
std::vector<bool> arrived(WS,false);
int frame_expected=0;
Frame_Base* frame;
bool is_sender=true;

enum event_type{frame_arrival,CRC_error,frame_timeout,ack_timeout};
event_type event;

void Node::initialize()
{
    // TODO - Generated method body
    WS=this->getParentModule()->par("WS").intValue();
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    //check if it is sender node or receiver based on coordinator
    if(is_sender) //should be modified
    {


    }else {

    }
}

std::vector<string> Node::readFile(string path)
{
    //TODO
}
std::string Node::framing(string line)
{
    //TODO
}
std::string Node::CRC(string frame)
{
    //TODO
}
void Node::sendMessage(string message)
{
    //TODO
}


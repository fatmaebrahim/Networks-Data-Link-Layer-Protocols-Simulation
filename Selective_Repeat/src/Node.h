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

#ifndef __SELECTIVE_REPEAT_NODE_H_
#define __SELECTIVE_REPEAT_NODE_H_

#include <omnetpp.h>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <bitset>
#include "Frame_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
public:

    void receiveFrame(cMessage *msg);
    void receiveNack(cMessage *msg);
    void sendFrame(cMessage *msg);
    void prepareMessages();
    void start();
    bool isAckLost();
    int inc(int seq_no);
    int incWS(int seq_no);
    bool isBetween(int frame_expected, int received_seq,int receiver_end_index);
    Frame_Base* applyError(Frame_Base* msg,std::string code_error);
    std::string calculateCRC(const std::string& input, const std::string& polynomial);
    bool verifyCRC(const std::string& receivedMessage, const std::string& polynomial);

    std::string framing(std::string line);
    std::string deframing(std::string frame);
    std::vector<std::string> readFile(const std::string& path);
    std::string path ;

    std::vector<Frame_Base*> receiver_buffer;
    std::vector<bool> arrived;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif

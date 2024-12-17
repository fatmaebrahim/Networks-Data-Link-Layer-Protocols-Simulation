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
int sender_end_index;
int sender_index=0;
std::vector<Frame_Base*> sender_buffer;
std::vector<std::string> sender_buffer_codes;
std::vector<Frame_Base*> timeouts;
int ack_expected=0;
int next_frame_to_send=0;
bool no_nack=true;

//Receiver Variable
int receiver_start_index=0;
int receiver_end_index;
int receiver_index=0;
std::vector<Frame_Base*> receiver_buffer;
std::vector<bool> arrived;
int frame_expected=0;
Frame_Base* frame;
int senderID;


enum event_type{frame_arrival,CRC_error,frame_timeout,ack_timeout};
event_type event;

//may be needed
bool flag_timout;
bool flag_nack;
std::string current_error;


void Node::initialize()
{
    // TODO - Generated method body
    WS=this->getParentModule()->par("WS").intValue();
    receiver_end_index=WS-1;
    sender_end_index=WS-1;

}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    double timeout = getParentModule()->par("TO").doubleValue();
    double process_time =getParentModule()->par("PT").doubleValue();
    double trans_delay = getParentModule()->par("TD").doubleValue();


    Frame_Base* received = dynamic_cast<Frame_Base*>(msg);
    if(strcmp(msg->getName(),"0")==0){
        EV << "Received a message: " << msg->getName() << endl;
        //message from coordinator
        path= "../simulations/inputs/input0.txt";
        prepareMessages();
        senderID=0;
        start();

    }
    else  if(strcmp(msg->getName(),"1")==0){
        EV << "Received a message: " << msg->getName() << endl;
        //message from coordinator
        path= "../simulations/inputs/input1.txt";
        prepareMessages();
        senderID=1;
        start();

    }

    else {

        if (received->getFrameType()==2){
            //receive
            EV<<"Node:"<<1-senderID<<" is receiving"<<"\n";
            EV << "recieved: " << received->getPayload() << endl;
            receiveFrame(received);
        }
        else  if (received->getFrameType()==1){
            //send
            EV << "ack: " << received->getAck_nack_number() << endl;
            sendFrame(received);
        }
        else  if (received->getFrameType()==0){
               //send
            EV << "notack: " << received->getName() << endl;

        }
    }

}
// Function to convert a string to binary representation
std::string stringToBinary(const std::string& input) {
    std::string binaryString;
    for (char c : input) {
        binaryString += std::bitset<8>(c).to_string(); // Convert each char to 8-bit binary
    }
    return binaryString;
}

// Function to convert binary back to string (optional for debugging)
std::string binaryToString(const std::string& binary) {
    std::string result;
    for (size_t i = 0; i < binary.size(); i += 8) {
        std::bitset<8> charBits(binary.substr(i, 8));
        result += static_cast<char>(charBits.to_ulong());
    }
    return result;
}
void Node::sendFrame(cMessage *msg){
    Frame_Base* received = dynamic_cast<Frame_Base*>(msg);
    EV << "start:" << sender_start_index<<"  end:"<<sender_end_index<<"  to_send:" <<next_frame_to_send<<"\n";
    int ackno= received->getAck_nack_number();
    //slide window
    while (sender_buffer[sender_start_index]->getHeader()!= received->getAck_nack_number()) {
        //mark other frames as sent to stop their timers --todo
        EV<<timeouts.size()<<"\n";
        cancelEvent(timeouts[sender_start_index]);
        sender_start_index++;
        if(sender_end_index<WS-1){
           sender_end_index++;
         }
    }

    //send frames in the new window
    EV<<"Node:"<<senderID<<" is sending"<<"\n";
    int index=0;

    for (int i = next_frame_to_send; i<=sender_end_index; i++){

        flag_timout=false;
        flag_nack=false;
        Frame_Base* frame_to_send = sender_buffer[i]->dup();
        frame_to_send=sender_buffer[i];
        frame_to_send->setFrameType(2);
        current_error=sender_buffer_codes[i];
        frame_to_send=applyError(frame_to_send,sender_buffer_codes[i]);
        //seqno
        //ackno
        next_frame_to_send++;
        //supposed to get handled in apply error , assume no errors
        double time=simTime().dbl()+getParentModule()->par("PT").doubleValue()*(i+1);
        EV<<i<<": "<<sender_buffer[i]<<"at time:"<<time<<"\n";
        sendDelayed(frame_to_send,time, "out");
        Frame_Base*  temp_frame = frame_to_send->dup();
        timeouts.push_back(temp_frame);
        //add TO to handle timeout where it will send a message to itself at timeouts
        scheduleAt(simTime()+getParentModule()->par("PT").doubleValue()*(index+1)+ getParentModule()->par("TO").doubleValue(),temp_frame);
        index++;
    }



}

void Node::receiveFrame(cMessage *msg){

}



void Node::start(){
    EV<<"Node:"<<senderID<<" is sending"<<"\n";

    for (int i = sender_start_index; i<=sender_end_index; i++){
        EV<<i<<": "<<sender_buffer[i]<<"\n";
        flag_timout=false;
        flag_nack=false;
        Frame_Base* frame_to_send = new Frame_Base;
        frame_to_send = sender_buffer[i]->dup();
        frame_to_send->setFrameType(2);
        current_error=sender_buffer_codes[i];
        frame_to_send=applyError(frame_to_send,sender_buffer_codes[i]);
        //seqno
        //ackno
        next_frame_to_send++;
        //supposed to get handled in apply error , assume no errors
        double time=simTime().dbl()+getParentModule()->par("PT").doubleValue()*(i+1);
        EV<<i<<": "<<sender_buffer[i]<<"at time:"<<time<<"\n";
//        EV<"At time ["<<time<<"], Node["<<senderID<<"] [sent] frame and payload=["<< frame_to_send->getPayload()<<"]"<<endl;

        sendDelayed(frame_to_send,time, "out");
        Frame_Base*  temp_frame = frame_to_send->dup();
        timeouts.push_back(temp_frame);
        //add TO to handle timeout where it will send a message to itself at timeouts
        scheduleAt(simTime()+getParentModule()->par("PT").doubleValue()*(i+1)+ getParentModule()->par("TO").doubleValue(),temp_frame);
    }


}
//[Modification, Loss, Duplication, Delay]

Frame_Base*Node:: applyError(Frame_Base *msg,std::string code_error){
    //apply error on msg using current_error
        /*
         * At time [.. starting sending time after processing….. ], Node[id] [sent] frame with
        seq_num=[..] and payload=[ ….. in characters after modification….. ] and trailer=[ …….in
        bits….. ] , Modified [-1 for no modification, otherwise the modified bit number] , Lost
        [Yes/No], Duplicate [0 for none, 1 for the first version, 2 for the second version], Delay [0
        for no delay , otherwise the error delay interval]. */
    if(code_error[0]=='1') // Modification
    {
            int modified_bit = int(uniform(0, 8 * std::string(msg->getPayload()).size())); // Assuming 8 bits per character
            std::string payload = msg->getPayload();
            std::string binaryPayload = stringToBinary(payload);

            binaryPayload[modified_bit] = (binaryPayload[modified_bit] == '0') ? '1' : '0';

            std::string modifiedPayload = binaryToString(binaryPayload);
            msg->setPayload(modifiedPayload.c_str());

            EV << "Modified bit: " << modified_bit
               << " | Original payload: " << payload
               << " | Modified payload: " << modifiedPayload << endl;

            // Set the modification log
            EV << "Modified [" << modified_bit << "] , Lost [No], Duplicate [0], Delay [0]." << endl;


    }else if (code_error[1]=='1') //Loss
    {
        /*"At time[" << simTime()+getParentModule()->par("PT").doubleValue()*(i+1) << "], Node["<<senderID<<"] sent frame with seq_num=["<< transmittedMsg->getSeqNumber() <<"] and payload=["<< transmittedMsg->getPayload()<<"]"<<
                " and trailer=["<< toBinary(transmittedMsg->getParity())<<"]" << "Modified [" << modified << "]" << "Lost [" << lost << "]" << ", Duplicate [" << duplicate << "], Delay [" << eD << "]"<< endl;*/
        EV<<", Modified [-1] , Lost [Yes], Duplicate [0], Delay [0]."<<endl;
        return nullptr;

    }else if (code_error[2]=='1')//Duplication
    {
        //NOT Complete
        double DD = getParentModule()->par("DD").doubleValue();
        Frame_Base*  duplicated_msg= msg->dup();

        scheduleAt(simTime()+DD,duplicated_msg);


    }else if (code_error[3]=='1')//Delay
    {
        double delayTime=getParentModule()->par("ED").doubleValue();
        scheduleAt(simTime() + delayTime, msg);
        EV << ", Modified [-1] , Lost [Yes], Duplicate [0], Delay ["<<delayTime<<"]." << endl;

    }
    //see if the codes
    return msg;
    //ToDo
}


std::vector<std::string> Node:: readFile(const std::string& path)
{
    std::vector<std::string> payloads;
    std::string code;
    std::string payload;
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file.");
    }

    std::string line;
    while (std::getline(file, line)) {
        code = line.substr(0, 4);
        payload = line.substr(5, line.size());
        payloads.push_back(payload);
        sender_buffer_codes.push_back(code);
    }

    file.close();
    return  payloads;
}

std::string Node::framing(std::string line)
{
    std::string frame;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '$') {
            frame += '/';
            frame += '$';

        }
        else if (line[i] == '/') {
            frame += '/';
            frame += '/';
        }
        else {
            frame += line[i];
        }
    }
    std::string flag="$";
    flag += frame;
    flag += '$';

    return flag;

}

std::string Node:: deframing(std::string frame)
{
    std::string line;
    int i = 1;
    while(i<frame.size()-1){
        if (frame[i] == '/' && frame[i+1] == '/') {
            line += '/';
            i += 2;
        }
        else if (frame[i] == '/' && frame[i + 1] == '$') {
            line += '$';
            i += 2;
        }
        else {
            line += frame[i];
            i++;
        }
    }
    return line;
}


// CRC calculation function
std::string Node::calculateCRC(const std::string& input, const std::string& polynomial) {
    std::string data = input;
    data.append(polynomial.size() - 1, '0'); // Append zeros for CRC calculation.

    std::string crc = data.substr(0, polynomial.size());
    for (size_t i = polynomial.size(); i <= data.size(); ++i) {
        if (crc[0] == '1') {
            for (size_t j = 0; j < polynomial.size(); ++j) {
                crc[j] = (crc[j] == polynomial[j]) ? '0' : '1'; // XOR operation
            }
        }
        crc.erase(0, 1); // Shift left
        if (i < data.size()) crc += data[i];
    }
    return input + crc; // Append CRC to the original input
}
// Function to check CRC
bool Node::verifyCRC(const std::string& receivedMessage, const std::string& polynomial) {
    size_t originalSize = receivedMessage.size() - (polynomial.size() - 1);
    std::string data = receivedMessage.substr(0, originalSize);
    std::string recalculatedMessage = calculateCRC(data, polynomial);

    return receivedMessage == recalculatedMessage;
}


void Node:: prepareMessages()
{
    //TODO

            std::vector<std::string> payloads = readFile(path);
            Frame_Base* frame = new Frame_Base;
            for (int i=0 ;i<payloads.size();i++) {
                std::string framed = framing(payloads[i]);
                std::string polynomial = "1101";
                std::string binaryMessage = stringToBinary(framed);
                std::string messageWithCRC = calculateCRC(binaryMessage, polynomial);
                frame->setPayload(messageWithCRC.c_str());
                sender_buffer.push_back(frame);
            }



}




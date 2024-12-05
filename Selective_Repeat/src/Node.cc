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
bool is_sender;

enum event_type{frame_arrival,CRC_error,frame_timeout,ack_timeout};
event_type event;

void Node::initialize()
{
    // TODO - Generated method body
    WS=this->getParentModule()->par("WS").intValue();
}

void Node::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(),"start")==0){
        is_sender=true;

    }else{
        is_sender=false;
    }

    // TODO - Generated method body
    //check if it is sender node or receiver based on coordinator
    if(is_sender) //should be modified
    {
        auto result = sendMessage();
        std::vector<std::string> codes = result.first;
        std::vector<std::string> frames = result.second;
        for(int i =0; i<codes.size();i++){
            std::cout << codes[i] << " : "<<frames[i]<<"\n";
        }

    }
    else {

    }
}




std::pair< std::vector<std::string>, std::vector<std::string> >Node:: readFile(const std::string& path)
{
    std::vector<std::string> codes;
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
        codes.push_back(code);
        payloads.push_back(payload);
    }

    file.close();
    return std::make_pair(codes, payloads);
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


std::pair< std::vector<std::string>, std::vector<std::string>> Node:: sendMessage()
{
    //TODO
    std::string path = "D:\\Fatma\\1stTerm_4thYear\\Networks\\Project\\2024_project\\input0.txt";

            auto result = readFile(path);
            std::vector<std::string> codes = result.first;
            std::vector<std::string> payloads = result.second;
            std::vector<std::string> frames ;
            for (const auto& payload : payloads) {
                std::string framed = framing(payload);
                std::string polynomial = "1101";
                std::string binaryMessage = stringToBinary(framed);
                std::string messageWithCRC = calculateCRC(binaryMessage, polynomial);
                frames.push_back(messageWithCRC);
            }
            return std::make_pair(codes, frames);

}




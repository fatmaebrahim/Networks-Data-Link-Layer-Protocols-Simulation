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
int SN;
int sender_start_index=0;
int sender_end_index;
int sender_index=0;
std::vector<Frame_Base*> sender_buffer;
std::vector<std::string> sender_buffer_codes;

int seq_no=0;
int ack_expected=0;
int next_frame_to_send=0;
bool no_nack=true;

//Receiver Variable
int receiver_start_index=0;
int receiver_end_index;
int receiver_index=0;

std::vector<Frame_Base*> ack_timeouts;

int frame_expected=0;
Frame_Base* frame;
int senderID;
std::vector<Frame_Base*> timeouts;

enum event_type{frame_arrival,CRC_error,frame_timeout,ack_timeout};
event_type event;

//may be needed
bool flag_timout;
bool flag_nack;
std::string current_error;
std::string polynomial = "1101";
std::ofstream output("output.txt");

void Node::initialize()
{
    // TODO - Generated method body
    WS=this->getParentModule()->par("WS").intValue();
    SN =this->getParentModule()->par("SN").intValue();
    receiver_end_index=WS-1;
    sender_end_index=WS-1;
    arrived.resize(SN+1, false);
    receiver_buffer.resize(WS);


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
        path= "../simulations/inputs/input2.txt";
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

//            EV << "recieved: " << received->getPayload() << endl;
            receiveFrame(received);
        }
        else  if (received->getFrameType()==1){
            //send
            EV<<"Node:"<<senderID<<" is sending"<<"\n";
//            EV << "ack: " << received->getAck_nack_number() << endl;
            sendFrame(received);
        }
        else  if (received->getFrameType()==0){
               //send
//            EV << "notack: " << received->getName() << endl;
            receiveNack(received);

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

//                                  4              0              7
bool Node:: isBetween(int frame_expected, int received_seq, int receiver_end_index){
    if (frame_expected<=received_seq && received_seq<= receiver_end_index)
        return true;
    if (receiver_end_index<frame_expected && frame_expected<=received_seq)
        return true;
    if (received_seq<= receiver_end_index && receiver_end_index<frame_expected)
        return true;
    return false;
}

int Node:: inc(int seq_no){
    return (seq_no+1)%(SN+1);
}

int Node:: dec(int seq_no){
    return (seq_no-1)%(SN+1);
}

int Node:: incWS(int seq_no){
    return (seq_no+1)%(WS);
}

bool Node:: isAckLost(){
    double loss_prob = getParentModule()->par("LP").doubleValue();
    double random_prob = (double)rand() / RAND_MAX;
    return false;
//    return random_prob < loss_prob;


}

void Node::receiveNack(cMessage *msg){
    Frame_Base* received = dynamic_cast<Frame_Base*>(msg);
    int received_seq= received->getAck_nack_number();
    int nack_index = sender_start_index;
    while(nack_index<sender_buffer.size()-1&&  sender_buffer[nack_index]->getHeader()!=received_seq && nack_index<sender_buffer.size()-1)
    {
        nack_index++;
    }


    EV<<"next frame to send:"<<next_frame_to_send<<" sender end index:"<<sender_end_index<<"\n";
    current_error ="0000";

     Frame_Base* frame_to_send = sender_buffer[nack_index]->dup();
     frame_to_send=applyError(frame_to_send,current_error);



//     next_frame_to_send++;
     double time=getParentModule()->par("PT").doubleValue();
     std::string messageWithCRC =frame_to_send->getPayload();
     std::string message_only=messageWithCRC.substr(0,(messageWithCRC.size()-polynomial.size()-1) );
     std::string crc_only=messageWithCRC.substr((messageWithCRC.size()-polynomial.size()),messageWithCRC.size() );
     output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"], Introducing channel error with code =["<<current_error<<"]"<<"\n";
     output<<"nackkkkkkkkkkkkkkkkkkkkkkkkk"<<"\n";
     output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";
     EV<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";

     EV<<"send"<<" seq_no: "<<frame_to_send->getHeader()<<" at time:"<<time+simTime().dbl()<<"\n";
     sendDelayed(frame_to_send,time, "out");



}




void Node::receiveFrame(cMessage *msg){

    Frame_Base* received = dynamic_cast<Frame_Base*>(msg);
    EV << "start:" << receiver_start_index<<"  end:"<< receiver_end_index<<"\n";

    EV<<"\n";
    //if receiver.seqno is inside the window of the receiver
    int received_seq=received->getHeader();
    EV<<"received seq_no: "<<received_seq<<" expected seq_no: "<<frame_expected<<"\n";
    EV<<arrived[received_seq%(SN+1)]<<" "<<frame_expected<<","<<received_seq<<","<<receiver_end_index<<"\n";
    EV<<"arrived: ";
    for (int i =0; i<arrived.size();i++){
        EV<<arrived[i]<<",";
    }
   EV<<"\n";
    if(arrived[received_seq%(SN+1)]==false &&(received_seq)!=(frame_expected) && no_nack &&isBetween(frame_expected , received_seq,receiver_end_index) )
    {
        EV<<"arrived: ";
        for (int i =0; i<arrived.size();i++){
            EV<<arrived[i]<<",";
        }
       EV<<"\n";
        double time;
        if (! isAckLost()){
        //send nack or ack or silent
        Frame_Base* nack_to_send = new Frame_Base;
        nack_to_send->setFrameType(0);
        nack_to_send->setAck_nack_number(frame_expected);
        time=getParentModule()->par("PT").doubleValue()+getParentModule()->par("TD").doubleValue();
        sendDelayed(nack_to_send,time, "out");
//        EV<<"sending nack:"<<nack_to_send->getAck_nack_number()<<" at time:"<<time<<"\n";

        EV<<"At time ["<<time+ simTime().dbl()<<"],Node["<<1-senderID<<"] Sending [NACK] with number ["<<nack_to_send->getAck_nack_number()<<"] ,loss [No]."<<"\n";

        }
        else{
        EV<<"At time ["<<time+ simTime().dbl()<<"],Node["<<1-senderID<<"] Sending [NACK] with number ["<<frame_expected<<"] ,loss [Yes]."<<"\n";

        }
        no_nack=false;

    }


    EV<<arrived[received_seq%(SN+1)]<<" "<<frame_expected<<","<<received_seq<<","<<receiver_end_index<<"\n";
    //                                                        7               7              1
    EV<<"arrived: ";
    for (int i =0; i<arrived.size();i++){
        EV<<arrived[i]<<",";
    }
   EV<<"\n";
   EV<<"arrived isbetween"<<arrived[received_seq%(SN+1)]<<" , "<<isBetween(frame_expected , received_seq,receiver_end_index)<<"\n";
    if(arrived[received_seq%(SN+1)]==false && isBetween(frame_expected , received_seq,receiver_end_index))
    {
        EV<<"arrived: ";
         for (int i =0; i<arrived.size();i++){
             EV<<arrived[i]<<",";
         }
        EV<<"\n";
        EV<<"Debug info "<<"received_seq%WS "<<received_seq%WS<<endl;
        EV<<"Debug info "<<"received_seq%WS "<<received_seq%WS<<endl;


        EV<<"in betweeeeeeeeeeeeeeeeeeen"<<"\n";

            if(verifyCRC(received->getPayload(),polynomial))
            {
                //                 7
                arrived[received_seq%(SN+1)]=true;
                receiver_buffer[frame_expected%WS]=received->dup();
                EV<<frame_expected<<": "<<receiver_buffer[frame_expected%WS]->getHeader()<<"\n";
                EV<<"in ifffffffffffffffffffff"<<"\n";

                EV<<(frame_expected%WS)<<" "<<receiver_buffer.size()<<"\n";
      //                             7  0                         3 0
                while(arrived[frame_expected%(SN+1)] && (frame_expected%WS)<receiver_buffer.size())
                {

                    no_nack=true;
                    arrived[frame_expected%(SN+1)]=false;
                   //    0
                    frame_expected=inc(frame_expected);
              //                                      2
                    receiver_end_index=inc(receiver_end_index);
              //
                    receiver_start_index=inc(receiver_start_index);
                    //reset ack timer                  0
                    EV<<"New frame expected "<< frame_expected<<" , "<<receiver_start_index<<" , "<<receiver_end_index<<"\n";
                    EV<<"arrived: ";
                    for (int i =0; i<arrived.size();i++){
                        EV<<arrived[i]<<",";
                    }
                   EV<<"\n";
                }


                EV<<"noooooooooonack"<<no_nack<<"\n";


                 if (no_nack && ! isAckLost()){

                     Frame_Base* ack_to_send = new Frame_Base;
                     ack_to_send->setFrameType(1);
                     ack_to_send->setAck_nack_number(frame_expected);
                     double time=getParentModule()->par("PT").doubleValue()+getParentModule()->par("TD").doubleValue();
                     sendDelayed(ack_to_send,time, "out");
     //                EV<<"sending ack:"<<ack_to_send->getAck_nack_number()<<" at time:"<<simTime().dbl()+time<<"\n";

                      output<<"At time ["<<time+ simTime().dbl()<<"],Node["<<1-senderID<<"] Sending [ACK] with number ["<<ack_to_send->getAck_nack_number()<<"] ,loss [No]."<<"\n";
                      EV<<"At time ["<<time+ simTime().dbl()<<"],Node["<<1-senderID<<"] Sending [ACK] with number ["<<ack_to_send->getAck_nack_number()<<"] ,loss [No]."<<"\n";

                 }


            }




    }

}




void Node::sendFrame(cMessage *msg){
    Frame_Base* received = dynamic_cast<Frame_Base*>(msg);
  EV << "send:" << sender_start_index<<"  end:"<<sender_end_index<<"  to_send:" <<next_frame_to_send<<"\n";
    int ackno= received->getAck_nack_number();

    int ack_dec;

    for (int i =0; i < timeouts.size();i++){
//        EV<<"timeoutssssss: "<<timeouts[i]->getHeader()<<"\n";
        ack_dec=dec(ackno);
        if (isBetween(sender_start_index,timeouts[i]->getHeader(),ack_dec)){

            EV<<"canceled event: "<<timeouts[i]->getHeader()<<"\n";
            cancelEvent(timeouts[i]);
        }
    }
    //slide window
    std::string canceled="";
//    EV<<"sender_start_index "<<sender_start_index<<endl;
    while (sender_start_index<sender_buffer.size()-1&& sender_buffer[sender_start_index]->getHeader() != received->getAck_nack_number()) {
        EV << "send:" << sender_start_index<<"  end:"<<sender_end_index<<"  ackno:" <<ackno<<"\n";


        sender_start_index=inc(sender_start_index);
//        sender_start_index++;
        if(sender_end_index<sender_buffer.size()-1){
          sender_end_index=inc(sender_end_index);
//            sender_end_index++;
         }
//        canceled+=std::to_string(sender_start_index);

    }
//    EV<<"the canceled frames are "<<canceled<<endl;

//    if (sender_start_index==sender_start_index && !isBetween(sender_start_index , next_frame_to_send,sender_start_index)){
//        cancelEvent(timeouts[sender_start_index%WS]);
//        EV<<"canceled event: "<<timeouts[sender_start_index%WS]->getHeader()<<"\n";
//    }

    //send frames in the new window

    int index=0;

    for (int i = next_frame_to_send; i<=sender_end_index && i<sender_buffer.size(); i++){
        EV<<"next frame to send:"<<next_frame_to_send<<" sender end index:"<<sender_end_index<<"nextframe: "<<i<<"\n";
        flag_timout=false;
        flag_nack=false;
        Frame_Base* frame_to_send = sender_buffer[i]->dup();
//        frame_to_send=sender_buffer[i];

        current_error=sender_buffer_codes[i];
        frame_to_send=applyError(frame_to_send,sender_buffer_codes[i]);
        if (frame_to_send ==nullptr) //the frame is lost
            continue;
        double delayTime=0;
               if (sender_buffer_codes[i][3]=='1')
               {
                    delayTime = frame_to_send->par("delay").doubleValue();

               }
             next_frame_to_send++;
//               next_frame_to_send=inc(next_frame_to_send);
        double time=getParentModule()->par("PT").doubleValue()*(index+1)+delayTime;
        std::string messageWithCRC =frame_to_send->getPayload();
              std::string message_only=messageWithCRC.substr(0,(messageWithCRC.size()-polynomial.size()-1) );
              std::string crc_only=messageWithCRC.substr((messageWithCRC.size()-polynomial.size()),messageWithCRC.size() );
             output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"], Introducing channel error with code =["<<current_error<<"]"<<"\n";
             output<<"hellooooooooooooooooooooooooo"<<"\n";
             output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";
             EV<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";
             EV<<"send"<<" seq_no: "<<frame_to_send->getHeader()<<" at time:"<<time+simTime().dbl()<<"\n";

        sendDelayed(frame_to_send,time, "out");
        Frame_Base*  temp_frame = frame_to_send->dup();
        EV<<"pushed to timeouts: "<<temp_frame->getHeader()<<"\n";
        timeouts.push_back(temp_frame);
//        timeouts[index%WS]=temp_frame;

        //add TO to handle timeout where it will send a message to itself at timeouts
        time=simTime().dbl()+getParentModule()->par("PT").doubleValue()*(index+1)+ getParentModule()->par("TO").doubleValue();
        EV<<"timeout: "<<" seq_no: "<<frame_to_send->getHeader()<<" at time:"<<time<<"\n";
        scheduleAt(time,temp_frame);
        index++;
    }



}


void Node::start(){
    EV<<"Node:"<<senderID<<" is sending"<<"\n";
    int index=0;
    for (int i = sender_start_index; i<=sender_end_index; i++){


        Frame_Base* frame_to_send = new Frame_Base;
        frame_to_send = sender_buffer[i]->dup();

        current_error=sender_buffer_codes[i];
        frame_to_send=applyError(frame_to_send,sender_buffer_codes[i]);
        double delayTime=0;
        if (sender_buffer_codes[i][3]=='1')
        {
             delayTime = frame_to_send->par("delay").doubleValue();

        }
        if (frame_to_send ==nullptr) //the frame is lost
        {
        EV<<"At time ["<<time<<"], Node["<<senderID<<"] [lost] frame "<<endl;
              continue;
        }
        next_frame_to_send++;
        double time=getParentModule()->par("PT").doubleValue()*(index+1)+delayTime;
//        EV<<"start:"<<i<<"seq_no: "<<frame_to_send->getHeader()<<" at time:"<<time+simTime().dbl()<<"\n";
        std::string messageWithCRC =frame_to_send->getPayload();
        std::string message_only=messageWithCRC.substr(0,(messageWithCRC.size()-polynomial.size()-1) );
        std::string crc_only=messageWithCRC.substr((messageWithCRC.size()-polynomial.size()),messageWithCRC.size() );

        output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"], Introducing channel error with code =["<<current_error<<"]"<<"\n";
        output<<"hellooooooooooooooooooooooooo"<<"\n";
        output<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";
        EV<<"At time ["<<time+simTime().dbl()<<"],Node["<<senderID<<"] sent frame with seq_num=["<<frame_to_send->getHeader()<<"] and payload=["<<binaryToString(message_only.c_str())<<"] and trailer=["<<crc_only<<"] ";

        sendDelayed(frame_to_send,time, "out");
        Frame_Base*  temp_frame = frame_to_send->dup();
        EV<<"pushed to timeouts: "<<temp_frame->getHeader()<<"\n";
        timeouts.push_back(temp_frame);
//        timeouts[i%WS]=temp_frame;

        time=simTime().dbl()+getParentModule()->par("PT").doubleValue()*(index+1)+ getParentModule()->par("TO").doubleValue();
        EV<<"timeout: "<<" seq_no: "<<frame_to_send->getHeader()<<" at time:"<<time<<"\n";

        //add TO to handle timeout where it will send a message to itself at timeouts
        scheduleAt(time,temp_frame);
        index++;
    }


}

Frame_Base* Node::applyError(Frame_Base *msg, std::string code_error) {
    if (code_error[1] == '1') { // Loss
        EV << ", Modified [-1], Lost [Yes], Duplicate [0], Delay [0]." << endl;
        return nullptr;
    } else {if (code_error[0] == '1') { // Modification
        // Duplicate and modify the frame
        Frame_Base* modified_frame = msg->dup();
        std::string payload = modified_frame->getPayload();
        if (payload.empty()) {
            EV << "Error: Payload is empty; no modification applied." << endl;
            return modified_frame;
        }

        // Convert payload to binary
        std::string binaryPayload = stringToBinary(payload);
        if (binaryPayload.empty()) {
            EV << "Error: Failed to convert payload to binary; no modification applied." << endl;
            return modified_frame;
        }

        EV << "Original payload: " << payload << endl;
        EV << "Binary payload before modification: " << binaryPayload << endl;

        // Modify a random bit in the binary payload
        int modified_bit = int(uniform(0, binaryPayload.size()));
        EV<<"Modified bit before being modified"<<binaryPayload[modified_bit]<<endl;
        binaryPayload[modified_bit] = (binaryPayload[modified_bit] == '0') ? '1' : '0';
        EV<<"Modified bit after being modified"<<binaryPayload[modified_bit]<<endl;

        EV << "Modified bit: " << modified_bit << endl;
        EV << "Binary payload after modification: " << binaryPayload << endl;

        // Convert binary payload back to string
        if (binaryPayload.size() % 8 != 0) {
            EV << "Error: Binary payload size is not a multiple of 8; modification failed." << endl;
            return modified_frame;
        }

        std::string modifiedPayload = binaryToString(binaryPayload);
        modified_frame->setPayload(modifiedPayload.c_str());

        EV << "Modified payload: " << modifiedPayload << endl;
        EV << "Modified [" << modified_bit << "], Lost [No], Duplicate [0], Delay [0]." << endl;

        return modified_frame;

    } else if (code_error[2] == '1') { // Duplication
        double DD = getParentModule()->par("DD").doubleValue();
        double PT = getParentModule()->par("PT").doubleValue();
        double time=simTime().dbl();
        Frame_Base* duplicated_msg = msg->dup();
        cMessage *convertedMsg = dynamic_cast<cMessage*>(duplicated_msg);
        convertedMsg->setName("messssssssage dup");
        EV<<"duplicate message"<<endl;
        sendDelayed(convertedMsg, time+DD+PT, "out"); //will be sent after the original message
        EV << ", Modified [-1], Lost [No], Duplicate [1], Delay [0]."<<"at time "<<time+DD+PT << endl;
        Frame_Base* tempMsg2 = msg->dup();
        return tempMsg2;

    } else if (code_error[3] == '1') { // Delay
        double delayTime = getParentModule()->par("ED").doubleValue();
        msg->addPar("delay").setDoubleValue(delayTime);

        EV << ", Modified [-1], Lost [No], Duplicate [0], Delay [" << delayTime << "]." << endl;
        return msg;
    }
    }

    return msg;
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
            std::cout<<"size of messsages"<<payloads.size()<<endl;

            for (int i=0 ;i<payloads.size();i++) {

                std::string framed = framing(payloads[i]);

                std::string binaryMessage = stringToBinary(framed);
                std::string messageWithCRC = calculateCRC(binaryMessage, polynomial);
                std::string message_only=messageWithCRC.substr(0,(messageWithCRC.size()-polynomial.size()-1) );
                std::string crc_only=messageWithCRC.substr((messageWithCRC.size()-polynomial.size()),messageWithCRC.size() );
                Frame_Base* frame = new Frame_Base;
                frame->setPayload(messageWithCRC.c_str());
//                frame->setTrailer(crc_only);
                frame->setFrameType(2);
                frame->setHeader(seq_no);
                seq_no=inc(seq_no);
                sender_buffer.push_back(frame);
            }



}




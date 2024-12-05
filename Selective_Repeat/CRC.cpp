#include <iostream>
#include <string>
#include <bitset>

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
std::string calculateCRC(const std::string& input, const std::string& polynomial) {
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
bool verifyCRC(const std::string& receivedMessage, const std::string& polynomial) {
    size_t originalSize = receivedMessage.size() - (polynomial.size() - 1);
    std::string data = receivedMessage.substr(0, originalSize);
    std::string recalculatedMessage = calculateCRC(data, polynomial);

    return receivedMessage == recalculatedMessage;
}

int main() {
    // Test inputs
    std::string message = "A flower blooms"; // Example human-readable input
    std::string polynomial = "1101";         // Example CRC polynomial

    // Convert the message to binary
    std::string binaryMessage = stringToBinary(message);
    std::cout << "Original Message (Binary): " << binaryMessage << std::endl;

    // Calculate the CRC
    std::string messageWithCRC = calculateCRC(binaryMessage, polynomial);
    std::cout << "Message with CRC: " << messageWithCRC << std::endl;

    // Verify the message with CRC
    bool isValid = verifyCRC(messageWithCRC, polynomial);
    std::cout << "Verification result: " << (isValid ? "Valid" : "Error detected") << std::endl;

    // Introduce an error and test again
    std::string corruptedMessage = messageWithCRC;
    corruptedMessage[10] = (corruptedMessage[10] == '0') ? '1' : '0'; // Flip one bit
    std::cout << "Corrupted Message: " << corruptedMessage << std::endl;
    isValid = verifyCRC(corruptedMessage, polynomial);
    std::cout << "Verification result for corrupted message: " << (isValid ? "Valid" : "Error detected") << std::endl;

    return 0;
}

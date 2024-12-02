# Data Link Layer Protocol Simulation  

This project simulates a communication system between two nodes over a noisy channel, implementing key data link layer protocols. The channel introduces realistic challenges, such as packet corruption, duplication, delay, and loss, while both nodes operate with limited buffer sizes.  

### Features  
- **Selective Repeat ARQ**:  
  Implements the Selective Repeat Automatic Repeat Request (ARQ) protocol for efficient error recovery, utilizing sender and receiver windows of size `WS`.  

- **Noisy Channel Simulation**:  
  Simulates real-world channel conditions where packets may experience:  
  - Corruption  
  - Duplication  
  - Delay  
  - Loss  

- **Framing with Byte Stuffing**:  
  Ensures proper packet framing by using Byte Stuffing to handle special characters in the data stream.  

- **Error Detection with CRC**:  
  Integrates Cyclic Redundancy Check (CRC) for reliable error detection and ensures data integrity during transmission.  

### Objectives  
- Develop a robust communication protocol for unreliable channels.  
- Simulate and analyze the behavior of the Selective Repeat ARQ with noisy conditions.  
- Implement framing and error detection mechanisms to enhance protocol reliability.  

### Tools and Technologies  
- **Simulation Environment**: OMNeT++  
- **Programming Language**: C++  

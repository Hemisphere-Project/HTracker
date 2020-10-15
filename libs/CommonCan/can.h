#include <mcp_can.h>
#include "can_msg.h"



#define CAN0_INT 15
MCP_CAN CAN0(12);

bool can_setup()
{
  pinMode(CAN0_INT, INPUT);

  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) {
      CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
      return true;
  }
  return false;
}


bool can_send(CanMessage* msg)
{
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  return (CAN0.sendMsgBuf(msg->uid(), 0, msg->frameSize(), msg->frame()) == CAN_OK);
}


CanMessage* can_read() 
{
  if(digitalRead(CAN0_INT) == LOW)                          // If CAN0_INT pin is low, read receive buffer
  {
    uint8_t   rxBuf[9]    = {0};
    uint8_t   length      = 0;
    long unsigned int uid = 0;

    // Read data: length = data length, buf = data byte(s)
    CAN0.readMsgBuf(&uid, &length, rxBuf);              

    return new CanMessage(uid, length, rxBuf);    
  }
  return nullptr;
}


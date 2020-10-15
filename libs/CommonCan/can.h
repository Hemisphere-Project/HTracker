#include <mcp_can.h>


enum canCode {
  NIET        = 0x00,
  REMOTEFRAME = 0x01,
  EMPTYFRAME  = 0x02,
  MEASURE     = 0xA0
};

struct canMsg {
  canCode code;
  long unsigned int uid;
  uint8_t length;
  uint8_t data[8];
};


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


bool can_send(canMsg msg)
{
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  uint8_t  txBuf[9] = {msg.code};
  for(byte k=0; k<msg.length; k++) txBuf[k+1] = msg.data[k];
  byte sndStat = CAN0.sendMsgBuf(msg.uid, 0, msg.length+1, txBuf);
  return (sndStat == CAN_OK);
}


canMsg can_read() 
{
  canMsg msg = {NIET, 0, 0};
  
  if(digitalRead(CAN0_INT) == LOW)                          // If CAN0_INT pin is low, read receive buffer
  {
    uint8_t   rxBuf[9] = {0};
    uint8_t   length = 0;

    // Read data: length = data length, buf = data byte(s)
    CAN0.readMsgBuf(&msg.uid, &length, rxBuf);              
    
    // Remote Request Frame
    if((msg.uid & 0x40000000) == 0x40000000) msg.code = REMOTEFRAME;

    // Copy data
    if (length > 0) {
      msg.code = (canCode) rxBuf[0];
      msg.length = length-1;
      for(byte k=0; k<msg.length; k++) msg.data[k] = rxBuf[k+1];
    }

    // No Data
    else msg.code = EMPTYFRAME;
    
  }

  return msg;
}


#include "wirish.h"
#include "i2c.h"
#define I2C_MSG_WRITE 0

#define MCP_ADDR         0x60
#define MCP_WRITE_DAC    0b01000000
#define MCP_WRITE_EEPROM 0b01100000
#define MCP_PD_NORMAL    0b00000000
#define MCP_PD_1K        0b00000010
#define MCP_PD_100K      0b00000100
#define MCP_PD_500K      0b00000110

void mcp_write_val(uint16 val) {
  /* save cycles by setting these structs up just once */
  static uint8 msg_data[3];
  
  /* static initiliazer for the i2c_msg struct:
  typedef struct i2c_msg 
    uint16 addr;    < Address
    uint16 flags;   < Bitwise OR of I2C_MSG_READ and I2C_MSG_10BIT_ADDR 
    uint16 length;  < Message length
    uint16 xferred; < Messages transferred
    uint8 *data;    < Data 
  */
  static i2c_msg msg = {MCP_ADDR,I2C_MSG_WRITE,3,0,msg_data};
  uint16 tmp;
  
  msg_data[0] = MCP_WRITE_DAC | MCP_PD_NORMAL;
  
  tmp = val >> 4;
  msg_data[1] = tmp;
  
  tmp = (val << 4) & 0x00FF;
  msg_data[2] = tmp;
  
  i2c_master_xfer(I2C2, &msg, 1, 0);
}


uint16 mcp_read_val() {
  /* save cycles by setting these structs up just once */
  static uint8 msg_data[5];
  
  /* static initiliazer for the i2c_msg struct:
  typedef struct i2c_msg 
    uint16 addr;    < Address
    uint16 flags;   < Bitwise OR of I2C_MSG_READ and I2C_MSG_10BIT_ADDR 
    uint16 length;  < Message length
    uint16 xferred; < Messages transferred
    uint8 *data;    < Data 
  */
  static i2c_msg msg = {MCP_ADDR,I2C_MSG_READ,5,0,msg_data};
  
  uint16 tmp = 0;
  
  /* we dont care about the status and EEPROM bytes (0, 3, and 4) */
  i2c_master_xfer(I2C2, &msg, 1, 2);
  
  tmp = (msg_data[1] << 4);
  tmp += (msg_data[2] >> 4);
  return tmp;
}

int mcp_test() {
  uint16 val;
  uint16 test_val = 0x0101;
  
  SerialUSB.println("Testing the MCP4725...");
  // read the value of the reg (should be 0x0800 if factory fresh)
  val = mcp_read_val();
  SerialUSB.print("DAC Register = ");
  SerialUSB.println(val, HEX);
  
  mcp_write_val(test_val);
  SerialUSB.print("Wrote ");
  SerialUSB.print(test_val,HEX);
  SerialUSB.println(" to the DAC");
  
  val = mcp_read_val();
  SerialUSB.print("DAC Register = ");
  SerialUSB.println(val,HEX);
  
  if (val != test_val) {
    SerialUSB.println("MCP4725 Test failed!");
    return 0;
  }
  
  SerialUSB.println("MCP4725 Test Passed!");
  return 1;
}

void setup() {
  pinMode(BOARD_BUTTON_PIN,INPUT);
  i2c_master_enable(I2C2, 0);

  waitForButtonPress();  
  mcp_test();
  
}

void loop() {
  static uint16 dout = 0;
  mcp_write_val(dout);
  dout++;
  
  if (dout >= 255) {
    dout = 0;
  }
}



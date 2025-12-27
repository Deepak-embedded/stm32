
#include"main.h"
extern SPI_HandleTypeDef hspi2;


void csLOW_B(void){

	HAL_GPIO_WritePin(MCP23_CS_PORT, MCP23_CS_PIN, GPIO_PIN_RESET);

}
 
void csHIGH_B(void){

	HAL_GPIO_WritePin(MCP23_CS_PORT, MCP23_CS_PIN, GPIO_PIN_SET);
}
 
void RST_HIGH_B(void){

	HAL_GPIO_WritePin(MCP23_RST1_PORT, MCP23_RST1_PIN, GPIO_PIN_SET);
}

void SPI_Write_B (uint8_t *data, uint8_t len){

	HAL_SPI_Transmit(&hspi2, data, len,3000);
}

void SPI_Read_B (uint8_t *data, uint8_t len){

	HAL_SPI_Receive(&hspi2, data, len,3000);

}

//INITIALIZE THE MCP23S17
void enable_MCPA_B (void){

	uint8_t tData[3];
	tData[0]=0x4E;     // OPCODE MCP23 READ ENABLE
	tData[1]=0x05;    // IOCONA
	tData[2]=0xFF;    //  FOR BANK1

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData,3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2); ;           // Write cycle delay (5ms)
}

void enable_MCPB_B (void){
	uint8_t tData[3];
	tData[0]=0x4E;     // OPCODE MCP23 READ ENABLE
	tData[1]=0x15;    // IOCONB
	tData[2]=0xFF;    //  FOR BANK1

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData,3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2);            // Write cycle delay (5ms)
}

void port_configB_B (void){
	uint8_t tData[3];
	tData[0]=0x4E;
	tData[1]=0x10;      // IODIRB= 0X10 PORTB BANK=1 OUTPUT address.
	tData[2]=0xff;      // IODIRB 0XFF INPUT read modbus

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData,3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2); ;          // Write cycle delay (5ms)
}

void port_configA_B (void){
	uint8_t tData[3];
	tData[0]=0x4E;
	tData[1]=0x00;      // IODIRA= 0X00 PORTB BANK=1 OUTPUT address.
	tData[2]=0xff;      // IODIRA 0X00 OUTPUT(relay output) oxff for input read modbus mcp

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData,3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2);           // Write cycle delay (5ms)
}


void polarity_configB_B (void){
	uint8_t tData[3];
	tData[0]=0x4E;
	tData[1]=0x11;     // IPOL 0X11 BANK1 IPOLB
	tData[2]=0xff;    // IPOL 0X00 SAME polarity at OUTPUT

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData, 3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2);            // Write cycle delay (5ms)
}

void latch_configA_B (void){     // FOR READING REGISTER

	uint8_t tData[3];
	tData[0]=0x4E;
	tData[1]=0x0A;     // OLATb 0X1A BANK1 address. 0X0A OLATA
	tData[2]=0xff;    // OLATB 0X00 OUTPUT LOW  AND 0XFF FOR HIGH   data

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData, 3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2);            // Write cycle delay (5ms)
}

void latch_configB_B (void){     // FOR READING REGISTER

	uint8_t tData[3];
	tData[0]=0x4E;
	tData[1]=0x1A;     // OLATb 0X1A BANK1 address. 0X0A OLATA
	tData[2]=0xFF;    // OLATB 0X00 OUTPUT LOW  AND 0XFF FOR HIGH   data

	csLOW_B();                    // pull the CS LOW
	SPI_Write_B(tData, 3);
	csHIGH_B();                   // pull the HIGH
	HAL_Delay(2);            // Write cycle delay (5ms)
}


uint8_t GPIO_readA_B (){

	uint8_t tData[2];
	uint8_t buf_A=0;
	tData[0] = 0x4F;         // OPCODE WRITE mcp23
	tData[1] = 0x09;         // 0x19 GPIOB read && GPIOA=0X09     //BANK1

	csLOW_B();
	SPI_Write_B(tData, 2);
	SPI_Read_B(&buf_A,1);
	csHIGH_B();
	HAL_Delay(10);
	return buf_A;

}




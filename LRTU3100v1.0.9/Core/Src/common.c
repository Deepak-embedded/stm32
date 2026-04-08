/*
 * comman.c
 *
 *  Created on: Nov 21, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/
#include "main.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/

/**************************************************************************//**
 * Function name 	: word
 * arguments		: 1) H MSByte
 * 		 		 	: 2) L LSByte
 * return			: 16 bit word
 * Note				:
 * 					:
 *****************************************************************************/

uint16_t word(uint8_t H, uint8_t L)
{
	bytesFields W;
	W.u8[0] = L;
	W.u8[1] = H;

	return W.u16[0];
}
/**************************************************************************//**
 * Function name 	: word
 * arguments		: 1) H MSByte
 * 		 		 	: 2) L LSByte
 * return			: 16 bit word
 * Note				:
 * 					:
 *****************************************************************************/
/**
 * \brief           Convert `unsigned 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       is_hex: Set to `1` to output hex, 0 otherwise
 * \param[in]       width: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 *                      This parameter is valid only when formatting hex numbers
 * \return          Pointer to output variable
 */
char*
lwgsm_u32_to_gen_str(uint32_t num, char* out, uint8_t is_hex, uint8_t width) {
    char* tmp = out;
    uint8_t i, y;

    /* Convert number to string */
    i = 0;
    tmp[0] = '0';
    if (num == 0) {
        ++i;
    } else {
        if (is_hex) {
            uint8_t mod;
            while (num > 0) {
                mod = num & 0x0F;
                if (mod < 10) {
                    tmp[i] = mod + '0';
                } else {
                    tmp[i] = mod - 10 + 'A';
                }
                num >>= 4;
                ++i;
            }
        } else {
            while (num > 0) {
                tmp[i] = (num % 10) + '0';
                num /= 10;
                ++i;
            }
        }
    }
    if (is_hex) {
        while (i < width) {
            tmp[i] = '0';
            ++i;
        }
    }
    tmp[i] = 0;

    /* Rotate string */
    y = 0;
    while (y < ((i + 1) / 2)) {
        char t = out[i - y - 1];
        out[i - y - 1] = tmp[y];
        tmp[y] = t;
        ++y;
    }
    out[i] = 0;
    return out;
}
/**************************************************************************//**
 * Function name 	: word
 * arguments		: 1) H MSByte
 * 		 		 	: 2) L LSByte
 * return			: 16 bit word
 * Note				:
 * 					:
 *****************************************************************************/
/**
 * \brief           Convert `signed 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 */
char*
lwgsm_i32_to_gen_str(int32_t num, char* out) {
    if (num < 0) {
        *out++ = '-';
        return lwgsm_u32_to_gen_str(LWGSM_U32(-num), out, 0, 0) - 1;
    } else {
        return lwgsm_u32_to_gen_str(LWGSM_U32(num), out, 0, 0);
    }
}

/**************************************************************************//**
 * Function name 	: lwgsm_str_to_asciiStr
 * arguments		: 1)
 * 		 		 	: 2)
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

void lwgsm_str_to_asciiStr(const char * _in_Str,unsigned int _in_Str_len,const char * _out_asciiStr,unsigned int _in_Str_out)
{
	unsigned int i;
	unsigned char str[4];

	memset((char *)_out_asciiStr, 0x00, _in_Str_out);

	for(i=0;i<_in_Str_len;i++)
	{
		lwgsm_u8_to_hex_str(_in_Str[i],(char *)str, 2);
		strcat((char *)_out_asciiStr,(const char *)str);
	}
}

/**************************************************************************//**
 * Function name 	: asciiStringToHexString
 * arguments		: 1)
 * 		 		 	: 2)
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
//Write a function to convert ascii string to hex string

void asciiStringToHexString(char* str, char* hexStr, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		sprintf(&hexStr[i*2], "%02X", str[i]);
	}
	hexStr[i*2] = '\0';
}

/**************************************************************************//**
 * hex_to_int
 *****************************************************************************/
//int hex_to_int(char c){
//        int first = c / 16 - 3;
//        int second = c % 16;
//        int result = first*10 + second;
//        if(result > 9) result--;
//        return result;
//}
//int hex_to_int(char c)
//{
//    if (c >= 97)
//        c = c - 32;
//    int first = c / 16 - 3;
//    int second = c % 16;
//    int result = first * 10 + second;
//    if (result > 9) result--;
//    return result;
//}

char HexChartoHexByte(unsigned char a)
{
    if (a >= '0' && a <= '9') {
        return (a - '0');
    }
    if (a >= 'A' && a <= 'F') {
        return ((a - 'A') + 10);
    }
    if (a >= 'a' && a <= 'f') {
        return ((a - 'a') + 10);
    }
    return -1;
}
/**************************************************************************//**
 * hex_to_ascii
 *****************************************************************************/
//int hex_to_ascii(char c, char d)
//{
//        int high = hex_to_int(c) * 16;
//        int low = hex_to_int(d);
//        return high+low;
//}
/**************************************************************************//**
 * convertHextoAsciiString
 *****************************************************************************/
unsigned char convertHextoAsciiString(char* i_HexString,char* O_AsciiString,unsigned int _length)
{
	unsigned char result=0,buf=0;

	unsigned int i_length,i_index,o_index=0;

	i_length=_length;

	if(i_length%2==0)
	{
        for(i_index = 0; i_index < i_length; i_index++)
        {
        	if(i_index == 0)
        	{
        		buf = (HexChartoHexByte(i_HexString[i_index]))<<4;
        	}
        	else if(i_index % 2 != 0)
			{
				O_AsciiString[o_index++]= buf|HexChartoHexByte(i_HexString[i_index]);//hex_to_ascii(buf, HexChartoHexByte(i_HexString[i_index]));
				//O_AsciiString[o_index++]=hex_to_ascii(buf, i_HexString[i_index]);
			}
			else
			{
				buf = (HexChartoHexByte(i_HexString[i_index]))<<4;
			}
        }
	}
	else
	{
		result=1;
	}
	return result;
}

int FindSubstr(char *listPointer, char *itemPointer)
{
  int t;
  char *p, *p2;

  for(t=0; listPointer[t]; t++)
  {
    p = &listPointer[t];
    p2 = itemPointer;

    while(*p2 && *p2==*p)
	{
      p++;
      p2++;
    }
    if(!*p2) return t; /* 1st return */
  }
   return -1; /* 2nd return */
}


//======================================================================
// Production START
//======================================================================
/**************************************************************************//**
 * Function name 	: checkProductionMode
 * arguments		: 1)
 * return 		 	:
 * Note				: USe to check device is connected in production test banch
 * 					: Or not
 * 					: if conected than get slave Ig ans set productiojn mode
 *****************************************************************************/
unsigned char proCheck=0,checkAgain=1;
unsigned int SlotNo_RS232_1,SlotNo_RS232_2,SlotNo;
unsigned char pro_MQTT_Broker_IP[30]="203.88.128.141";
unsigned int pro_MQTT_Broker_Port = 1884;
unsigned char pro_MQTT_Client_ID[30]="Production_Test";
unsigned short int proTestRequest;
unsigned char Pro_Application_flag=0,pro_DO_DI_TestFinish = 0,UART_OTA_ACKflag=0;
volatile unsigned char UART_OTAflag, AI_CALflag, History_READflag, beforeRTOS;
unsigned char pro_RS232_1_state=0,pro_RS232_2_state=0,pro_RS485_1_state=0,pro_RS485_2_state=0;
unsigned char pro_DO_State[26],pro_DI_State[8];
unsigned char pro_key1_status,pro_key2_status,pro_Flash_State=0,pro_I2C_State=0;


void checkProductionMode()
{
  	beforeRTOS=1;  		// before RTOS uart read int handleing
  //	HAL_Delay(10000);
  	HAL_UART_Receive_IT(&huart1, &ModbusH[COM_RS232_1].u8RxBuffer[0], sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
  //	char pro_tx[10];

  	HAL_Delay(3000);

  	checkAgain=1;
  	do
  	{
  	    HAL_Delay(1500);
  		proCheck++;
		#ifdef WATCH_DOG_ENABLE
			HAL_IWDG_Refresh(&hiwdg1);
		#endif


		if(SlotNo_RS232_1 == 0 && EPROM_General.slot_id == 0)
		{
			checkAgain = 1;
		}
		else if (SlotNo_RS232_1 != 0)
		{
			SlotNo = SlotNo_RS232_1;
			EPROM_General.slot_id=SlotNo;      //store in flash
			ExtFlash_update_EPROM_General();
			checkAgain = 0;
		}


	#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
		HAL_IWDG_Refresh(&hiwdg1);
	#endif
  	}
  	while(proCheck<=10 && (checkAgain == 1));

  	beforeRTOS=0;
  	UART_OTAflag=0;

	if((SlotNo_RS232_1 != 0) || EPROM_General.slot_id !=0)
	{
		SlotNo=EPROM_General.slot_id;
		pro_RS232_1_state = 1;
		Pro_Application_flag=1;
	}
	else
	{
		Pro_Application_flag=0;
		pro_RS232_1_state = 0;
		pro_RS232_2_state = 0;
	}

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

#ifdef WATCH_DOG_ENABLE
	HAL_IWDG_Refresh(&hiwdg1);
#endif
}

/**************************************************************************//**
 * Function name 	: checkProductionMode
 * arguments		: 1)
 * return 		 	:
 * Note				: USe to check device is connected in production test banch
 * 					: Or not
 * 					: if conected than get slave Ig ans set productiojn mode
 *****************************************************************************/

void setProductionModePara()
{
#ifdef WATCH_DOG_ENABLE
	HAL_IWDG_Refresh(&hiwdg1);
#endif
}

int atoi_new(const char* str, int len)
{
    int i;
    int ret = 0;
    for(i = 0; i < len; ++i)
    {
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

void HexToAscii(const void *value, char *out, uint8_t numBytes)
{
    const uint8_t *bytes = (const uint8_t *)value;
    static const char hexDigits[] = "0123456789ABCDEF";

    for (uint8_t i = 0; i < numBytes; i++)
    {
        uint8_t b = bytes[i];
        out[i * 2]     = hexDigits[(b >> 4) & 0x0F];  // high nibble
        out[i * 2 + 1] = hexDigits[b & 0x0F];         // low nibble
    }

    out[numBytes * 2] = '\0'; // Null-terminate string
}

void AsciiToHex(const char *in, void *out, uint8_t numBytes)
{
    uint8_t *bytes = (uint8_t *)out;

    for (uint8_t i = 0; i < numBytes; i++)
    {
        char c1 = in[i * 2];     // high nibble
        char c2 = in[i * 2 + 1]; // low nibble

        uint8_t high, low;

        // Convert ASCII hex char to nibble
        if (c1 >= '0' && c1 <= '9') high = c1 - '0';
        else if (c1 >= 'A' && c1 <= 'F') high = c1 - 'A' + 10;
        else if (c1 >= 'a' && c1 <= 'f') high = c1 - 'a' + 10;
        else high = 0; // fallback

        if (c2 >= '0' && c2 <= '9') low = c2 - '0';
        else if (c2 >= 'A' && c2 <= 'F') low = c2 - 'A' + 10;
        else if (c2 >= 'a' && c2 <= 'f') low = c2 - 'a' + 10;
        else low = 0; // fallback

        bytes[i] = (high << 4) | low;
    }
}

void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType)
{
	if(LogEnable == 1 && UART_OTAflag == 0 && Pro_Application_flag == 0 && AI_CALflag == 0 && History_READflag == 0)
	{
		HAL_UART_Transmit(&huart1,(const uint8_t *)pData,strlen((const char *)pData),1000);
	}

}

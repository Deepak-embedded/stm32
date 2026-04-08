/*
 * pcbplcTimerTask.c
 *
 *  Created on: Jan 06, 2023
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "pcbplcTimerTask.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
unsigned char gStopHistoricalDataStoreCounter=0;
unsigned char gStopHistoricalDataStore=0;
unsigned long int plcTimerTicks = 0;
osThreadId plcTimer_TaskHandle;

/**************************************************************************//**
 * Function name 	: plcTimer_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void plcTimer_start()
{
	osThreadDef(plcTimerTask, StartplcTimerTask, osPriorityNormal, 0, 128); //512
	plcTimer_TaskHandle = osThreadCreate(osThread(plcTimerTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartCOM_PORT_RS232_1Task
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartplcTimerTask(void const * argument)
{
	for(;;)
	{
		plcTimerTicks++;
		if(gStopHistoricalDataStoreCounter++ > 60)
		{
			gStopHistoricalDataStore = 0;
		}
		osDelay(1000);
	}
}

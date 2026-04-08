/*
 * pcbplcTask.c
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "pcbplcService.h"
#include "pcbplcTask.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/

unsigned int count_pcbplc=0;
osThreadId pcbplc_TaskHandle;

/**************************************************************************//**
 * Function name 	: pcbplc_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void pcbplc_start()
{
	osThreadDef(pcbplcTask, StartpcbplcTask, osPriorityAboveNormal, 0, 1024*10); //512
	pcbplc_TaskHandle = osThreadCreate(osThread(pcbplcTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartpcbplcTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartpcbplcTask(void const * argument)
{

//    if(argc >= 3)
//    {
//        strncpy(gPlcFile, argv[1], sizeof(gPlcFile));
//        strncpy(gRecFile, argv[2], sizeof(gRecFile));
//    }
//    else
//    {
//        strcpy(gPlcFile, "TEST.PLC");
//        strcpy(gRecFile, "TEST.REC");
//    }

//    if(!service_initialize(CONFIG_FILE_PATH))
//    {
//       printf("Error at service_initialize\n");
//       //return -1;
//    }

    service_start();

    service_stop();

    //return 0;
}

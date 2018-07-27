/*
 * NW_Manager.c
 *
 *  Created on: 19 nov. 2017
 *      Author: HZGF0437
 */

/* Standard includes. */
#include <AdminManagers.h>

#include "CommonStructure.h"
#include "MyAppConfig.h"
#include "validateToken_Interface.h"
#include "ResponseCode.h"
#include "pip/debug.h"
#include "string.h"
#include "structcopy.h"
#include "parser.h"
#include "stdint.h"
#include "InternalCommunication_Interface.h"
#include "Internal_Communication.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <pip/api.h>
#include <pip/paging.h>
#include <pip/compat.h>

/*-----------------------------------------------------------*/
extern int printf0(const char *format, ...);

void printtruc(char * from,int size){
	printf("BUFFER SIZE : %x content ",size);
	for(int i=0; i<size;i++)
		printf0("%x '%c'",from[i],((from[i]>=32)?from[i]:'.'));
}

void SP1D_Task( uint32_t *pvParameters )
{
	/*QueueHandle_t xQueue_2AM = ( (QueueHandle_t*) pvParameters)[0];
	QueueHandle_t xQueue_2IC = ( (QueueHandle_t*) pvParameters)[1];
	QueueHandle_t xQueue_2NW = ( (QueueHandle_t*) pvParameters)[5];
	QueueHandle_t xQueue_P2IC = ( (QueueHandle_t*) pvParameters)[6];*/
	//for(;;)
		printf("Starting SP1_D task\r\n");
	QueueHandle_t xQueue_2NW = (QueueHandle_t) pvParameters[0];
	QueueHandle_t xQueue_2OD_IC =(QueueHandle_t) pvParameters[1];
	QueueHandle_t xQueue_2SP1D = (QueueHandle_t) pvParameters[2];

	event_t EventPartition;
	event_t EventResponse;
	event_t MessageToReturn;
	incomingMessage_t Check;

	char * INMES = (char*)allocPage();
	char * OUTMES = (char*)allocPage();
	uint32_t sizeout;
	uint32_t j;

	/* Remove compiler warning in the case that configASSERT() is not
	defined. */

	( void ) pvParameters;
	for( ;; )
	{
		/* Receive data from Network manager or from Administration Manager*/
		printf("Receiving something ?\r\n");

		EventPartition = myreceive(INMES, xQueue_2SP1D);
		//printtruc((char*)&EventPartition, sizeof(EventPartition));


		incomingMessagecpy(&Check, &(EventPartition.eventData.incomingMessage) );
		DEBUG(INFO,"UserID: %d, DeviceID: %d, DomainID: %d, Instruction: %d, Command Data: %s\n", Check.userID, Check.deviceID, Check.domainID, Check.command.instruction, Check.command.data);

		DEBUG(INFO,"Token:");
		for(j=0 ; j<Check.tokenSize ; j++){
			Pip_Debug_PutDec(Check.token[j]);
		}
		printf("\r\n");
		EventResponse = AdminManager_SP1D_Function(EventPartition);

		eventcpy(&MessageToReturn,&EventResponse);

		DEBUG(INFO,"IntComm-Response code: %#04X \n", MessageToReturn.eventData.response.responsecode);
		DEBUG(INFO, "Data: %s \n", MessageToReturn.eventData.response.data );

		switch(EventResponse.eventType){
		case INT_MESS_0:
			xProtectedQueueSend(xQueue_2OD_IC, &EventResponse, portMAX_DELAY);
			//mysend(1, OUTMES, xQueue_2OD_IC, sizeout);

			break;

		case RESPONSE:
			/* Send Data to Network manager*/
			sizeout = serialize_response(EventResponse.eventData.response, OUTMES);
			mysend(1, OUTMES, xQueue_2NW, sizeout);

			break;

		default:
			DEBUG(TRACE,"Internal Communication: Unknown Event Type\n");

			break;
		}

		/*Reinitialize events*/
		eventreset(&MessageToReturn);
		eventreset(&EventResponse);
		eventreset(&EventPartition);
	}
}

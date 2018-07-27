/*
 * NW_Manager.c
 *
 *  Created on: 19 nov. 2017
 *      Author: HZGF0437
 */

/* Standard includes. */
#include <AdminManagers.h>
#include "GPIO_I2C.h"
#include "Galileo_Gen2_Board.h"
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

	void od_Task( uint32_t * pvParameters )
{
	//for(;;)
	printf("Starting OD TASK\r\n");

	QueueHandle_t xQueue_2NW = (QueueHandle_t) pvParameters[0];
	QueueHandle_t xQueue_2OD = (QueueHandle_t) pvParameters[1];
	QueueHandle_t xQueue_2SP1D = (QueueHandle_t) pvParameters[2];
	QueueHandle_t xQueue_2SP2D = (QueueHandle_t) pvParameters[3];
	QueueHandle_t xQueue_2SP3D = (QueueHandle_t) pvParameters[4];

	printf("Queue are OK\r\n");
	event_t EventPartition;
	event_t EventResponse;
	event_t MessageToReturn;
	incomingMessage_t Check;

	//char INMES[IN_MAX_MESSAGE_SIZE];
	char * INMES = (char*)allocPage();
	char * OUTMES = (char*)allocPage();
	//char OUTMES[OUT_MAX_MESSAGE_SIZE];

	uint32_t sizeout;
	uint32_t j;

	/* Remove compiler warning in the case that configASSERT() is not
	defined. */
	( void ) pvParameters;

	printf("Starting work\r\n");

	for( ;; )
	{
		/* Receive data from Network manager or from Administration Manager*/
		// appelle bloquant
		printf("Receiving packet\r\n");


		EventPartition = myreceive(INMES, xQueue_2OD);

		printf("Received something\r\n");
		incomingMessagecpy(&Check, &(EventPartition.eventData.incomingMessage) );
		DEBUG(INFO,"UserID: %lu, DeviceID: %lu, DomainID: %lu, Instruction: %lu, Command Data: %s\r\n", Check.userID, Check.deviceID, Check.domainID, Check.command.instruction, Check.command.data);

		DEBUG(INFO,"Token:");
		for(j=0 ; j<Check.tokenSize ; j++){
			debug1("%02X", Check.token[j]);
		}
		debug1("\r\n");

		EventResponse = AdminManagerFunction(EventPartition);

		eventcpy(&MessageToReturn,&EventResponse);

		DEBUG(INFO,"IntComm-Response code: %#04X \n", MessageToReturn.eventData.response.responsecode);
		DEBUG(INFO, "Data: %s \n", MessageToReturn.eventData.response.data );

		switch(EventResponse.eventType){

		case INT_RESP_1:
			xProtectedQueueSend( xQueue_2SP1D, &EventResponse, portMAX_DELAY );
			break;

		case INT_RESP_2:
			xProtectedQueueSend( xQueue_2SP2D, &EventResponse, portMAX_DELAY );
			break;

		case INT_RESP_3:
			xProtectedQueueSend( xQueue_2SP3D, &EventResponse, portMAX_DELAY );
			break;

		case RESPONSE:
			/* Send Data to Network manager*/
			// TODO move serialization to NW_Manager
			sizeout=serialize_response(EventResponse.eventData.response, OUTMES);
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

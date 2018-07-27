/*
 * NW_Manager.c
 *
 *  Created on: 19 nov. 2017
 *      Author: HZGF0437
 */

/* Standard includes. */
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

/*-----------------------------------------------------------*/

void InternalCommunicationTask( void *pvParameters )
{
	QueueHandle_t xQueue_2AM = ( (QueueHandle_t*) pvParameters)[0];
	QueueHandle_t xQueue_2IC = ( (QueueHandle_t*) pvParameters)[1];
	QueueHandle_t xQueue_2NW = ( (QueueHandle_t*) pvParameters)[5];
	QueueHandle_t xQueue_P2IC = ( (QueueHandle_t*) pvParameters)[6];

	event_t EventPartition;
	event_t EventResponse;
	event_t MessageToReturn;
	incomingMessage_t Check;

	char INMES[IN_MAX_MESSAGE_SIZE];
	char OUTMES[OUT_MAX_MESSAGE_SIZE];
	uint32_t sizeout;
	uint32_t j;

	/* Remove compiler warning in the case that configASSERT() is not
	defined. */
	( void ) pvParameters;
	for( ;; )
	{
		/* Receive data from Network manager or from Administration Manager*/
		EventPartition = myreceive(INMES, xQueue_P2IC);

		incomingMessagecpy(&Check, &(EventPartition.eventData.incomingMessage) );
		DEBUG(INFO,"UserID: %lu, DeviceID: %lu, DomainID: %lu, Instruction: %lu, Command Data: %s\n", Check.userID, Check.deviceID, Check.domainID, Check.command.instruction, Check.command.data);

		DEBUG(INFO,"Token:");
		for(j=0 ; j<Check.tokenSize ; j++){
			debug1("%02X", Check.token[j]);
		}
		debug1("\n");

		xQueueSend( xQueue_2AM, &EventPartition, 0U );

		xQueueReceive( xQueue_2IC, &EventResponse, portMAX_DELAY );

		switch(EventResponse.eventType){
		case RESPONSE:
			eventcpy(&MessageToReturn,&EventResponse);

			DEBUG(INFO,"IntComm-Response code: %#04X \n", MessageToReturn.eventData.response.responsecode);
			DEBUG(INFO, "Data: %s \n", MessageToReturn.eventData.response.data );

			/* Send Data to Network manager*/
			sizeout=serialize_response(EventResponse.eventData.response, OUTMES);
			mysend(1, OUTMES, xQueue_2NW, sizeout);

			/*Reinitialize events*/
			eventreset(&MessageToReturn);
			eventreset(&EventResponse);
			eventreset(&EventPartition);
			break;

		default:
			DEBUG(TRACE,"Internal Communication: Unknown Event Type\n");
			/*Reinitialize events*/
			eventreset(&MessageToReturn);
			eventreset(&EventResponse);
			eventreset(&EventPartition);
			break;
		}
	}
}

/*
 * Communicate_SimpleAdaptor.c
 *
 *  Created on: 21 d�c. 2017
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
#include "communicateSimple.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <queue.h>


#include <pip/fpinfo.h>
#include <pip/paging.h>
#include <pip/vidt.h>
#include <pip/api.h>
#include <pip/compat.h>


event_t * EventToReturn = NULL;
event_t myreceive(char* data, QueueHandle_t xQueue_P2IC)
{
	printf("Starting myreceive\r\n");

	if(!EventToReturn)
		EventToReturn = (event_t*) allocPage();

	printf("Event reset\r\n");
	//eventreset(EventToReturn);
	printf("Receive something from %x to %x\r\n",xQueue_P2IC,EventToReturn);

	xProtectedQueueReceive( xQueue_P2IC, EventToReturn, portMAX_DELAY );

	printf("Received\r\n");
	return *(event_t*)EventToReturn;
}

/*event_t myreceive(char* data, QueueHandle_t xQueue_P2IC){
	uint32_t result;
	event_t EventToReturn;

	eventreset(&EventToReturn);
	result=receive_simple(data, xQueue_P2IC);
	if (result != 0){
		EventToReturn.eventData.incomingMessage=deserialize_incomingMessage(data, result);
		EventToReturn.eventType=EXT_MESSAGE;

		return EventToReturn;
	}
	else
		return EventToReturn;
}*/

void mysend (uint32_t dest, char* data, QueueHandle_t xQueue_IC2P, uint32_t datasize){
	send_simple(data, xQueue_IC2P, datasize);
}

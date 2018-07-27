/*
 * KeyManager.c
 *
 *  Created on: 21 nov. 2017
 *      Author: hzgf0437
 */


/* Standard includes. */
#include "CommonStructure.h"
#include "ManageKey_Interface.h"
#include "MyAppConfig.h"
#include "string.h"
#include "pip/debug.h"
#include "structcopy.h"
#include "ResponseCode.h"
#include "stdint.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

// TODO use references instead of copy
event_t KeyManagerFunction( event_t ReceivedValue )
{

	DEBUG(TRACE,"Hello! I am the Key Manager !\r\n");

	static int key_manager_initialized = 0;
	/* Initiatilize token key. This part may not be necessary in an implementation where the keys are stored in a file or a DB*/
	static key_t * List_TokenKey=NULL;

	if(!key_manager_initialized)
	{
		key_manager_initialized = 1;
		command_t InitValue={0,ADD_KEY,"1:17"};
		ManageKey(InitValue, &List_TokenKey, NULL);
		DEBUG(TRACE,"Initialize Token Key List\r\n");
	}

	char responseData[DATA_SIZE]={};
	strcpy(responseData,"\0");

	uint32_t result = 0;
	event_t EventToSend;
	response_t ResponseToSend;
	eventreset(&EventToSend);
	responsereset(&ResponseToSend);

	switch(ReceivedValue.eventType){
	case GET_KEY:
		result = ManageKey(ReceivedValue.eventData.command, &List_TokenKey, responseData);
		DEBUG(TRACE,"Get key for token validation: Result : %#04X. Data: %s\r\n", result, responseData);

		ResponseToSend.userID = ReceivedValue.eventData.command.userID;
		strcpy(ResponseToSend.data,responseData);
		ResponseToSend.responsecode= result ;

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		return EventToSend;

		break;
	case EXT_COMMAND:
		result = ManageKey(ReceivedValue.eventData.command, &List_TokenKey, responseData);

		DEBUG(TRACE,"Manage Key: Result : %#04X. Data: %s\r\n", result, responseData);

		ResponseToSend.userID = ReceivedValue.eventData.command.userID;
		strcpy(ResponseToSend.data,responseData);
		ResponseToSend.responsecode= result ;

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		return EventToSend;

		break;
	case EXT_MESSAGE :
	case RESPONSE :
	default :
		DEBUG(INFO, "KeyManager: Unknown Event Type\r\n");
		return EventToSend;
		break;
	}
}


void KeyManagerTask( void *pvParameters ){
	QueueHandle_t xQueue_2AM = ( (QueueHandle_t*) pvParameters)[0];
	QueueHandle_t xQueue_2TV = ( (QueueHandle_t*) pvParameters)[2];
	QueueHandle_t xQueue_2KM = ( (QueueHandle_t*) pvParameters)[4];

	event_t ReceivedValue;
	char responseData[DATA_SIZE]={};
	uint32_t result;
	event_t EventToSend;
	response_t ResponseToSend;

	/* Initiatilize token key. This part may not be necessary in an implementation where the keys are stored in a file or a DB*/
	key_t * List_TokenKey=NULL;

	DEBUG(TRACE,"Initialize Token Key List\n");

	command_t InitValue={0,ADD_KEY,"1:17"};
	ManageKey(InitValue, &List_TokenKey, NULL);

	/* Remove compiler warning in the case that configASSERT() is not
	defined.*/
	( void ) pvParameters;


	for( ;; )
	{
		eventreset(&ReceivedValue);
		result=0;
		strcpy(responseData,"\0");
		eventreset(&EventToSend);
		responsereset(&ResponseToSend);

		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue_2KM, &ReceivedValue, portMAX_DELAY );
		DEBUG(TRACE,"Hello! I am the Key Manager !\n");

		switch(ReceivedValue.eventType){
		case GET_KEY:
			result = ManageKey(ReceivedValue.eventData.command, &List_TokenKey, responseData);
			DEBUG(TRACE,"Get key for token validation: Result : %#04X. Data: %s\n", result, responseData);

			ResponseToSend.userID = ReceivedValue.eventData.command.userID;
			strcpy(ResponseToSend.data,responseData);
			ResponseToSend.responsecode= result ;

			EventToSend.eventType=RESPONSE;
			responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

			xQueueSend( xQueue_2TV, &EventToSend, 0U );
			break;
		case EXT_COMMAND:
			result = ManageKey(ReceivedValue.eventData.command, &List_TokenKey, responseData);

			DEBUG(TRACE,"Manage Key: Result : %#04X. Data: %s\n", result, responseData);

			ResponseToSend.userID = ReceivedValue.eventData.command.userID;
			strcpy(ResponseToSend.data,responseData);
			ResponseToSend.responsecode= result ;

			EventToSend.eventType=RESPONSE;
			responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

			xQueueSend( xQueue_2AM, &EventToSend, 0U );
			break;
		case EXT_MESSAGE :
		case RESPONSE :
		default :
			DEBUG(INFO, "KeyManager: Unknown Event Type\n");
			break;
		}
	}

}

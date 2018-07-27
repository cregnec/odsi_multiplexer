/*
 * TokenValidator.c
 *
 *  Created on: 19 nov. 2017
 *      Author: HZGF0437
 */

/* Standard includes. */
#include "KeyManager.h"
#include "CommonStructure.h"
#include "MyAppConfig.h"
#include "validateToken_Interface.h"
#include "ResponseCode.h"
#include "pip/debug.h"
#include "string.h"
#include "structcopy.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

// TODO use references instead of copy
event_t TokenValidateFunction( event_t ReceivedValue )
{
	response_t ResponseToSend;
	event_t EventToSend;
	event_t KeyRequest;
	event_t KeyResponse;
	char result;

	responsereset(&ResponseToSend);
	eventreset(&EventToSend);
	eventreset(&KeyRequest);
	eventreset(&KeyResponse);

	DEBUG(TRACE,"Hello! I am the token Validator !\r\n");

	switch(ReceivedValue.eventType){
	case EXT_MESSAGE:
		KeyRequest.eventType=GET_KEY;
		incomingMessagecpy(&KeyRequest.eventData.incomingMessage, &ReceivedValue.eventData.incomingMessage);

		/*Create a command to request the key value*/
		KeyRequest.eventData.command.instruction=READ_KEY;
		strcpy( KeyRequest.eventData.command.data , "1:" );

		KeyResponse = KeyManagerFunction(KeyRequest);

		/*Validate the token received */

		result=token_validate(ReceivedValue.eventData.incomingMessage.token, KeyResponse.eventData.response.data);

		/* Send to the queue - causing the Administration Manager to unblock,
		 * send the command to the expected target for processing
		 * 0 is used as the block time so the sending operation
		 * will not block - it shouldn't need to block as the queue should always
		 * be empty at this point in the code. */

		if(result == 1){
			DEBUG(TRACE,"Token is valid\r\n");
			EventToSend.eventType=RESPONSE;
			EventToSend.eventData.incomingMessage.userID=ResponseToSend.userID;
			ResponseToSend.responsecode= SUCCESS ;

			responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

			return EventToSend;
		}
		else{
			DEBUG(TRACE,"Token is not valid\r\n");
			ResponseToSend.userID=ReceivedValue.eventData.incomingMessage.userID;
			strcpy(ResponseToSend.data, "\0");
			ResponseToSend.responsecode= INVALID_TOKEN ;

			EventToSend.eventType=RESPONSE;
			responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

			return EventToSend;
		}
		break;
	case EXT_COMMAND :
	case RESPONSE :
	case GET_KEY :
	default :
		DEBUG(TRACE,"TOKEN_VALIDATOR: Unknown Event Type\r\n");
		break;
	}

	return EventToSend;
}


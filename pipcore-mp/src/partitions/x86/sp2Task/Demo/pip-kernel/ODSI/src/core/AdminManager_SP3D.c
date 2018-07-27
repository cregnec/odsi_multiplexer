/*
 * AdminManagaer.c
 *
 *  Created on: 19 nov. 2017
 *      Author: HZGF0437
 */

/*-----------------------------------------------------------*/

/* Standard includes. */
#include "TokenValidator.h"
#include "CommonStructure.h"
#include "routeCommand_Interface.h"
#include "pip/debug.h"
#include "structcopy.h"
#include "ResponseCode.h"

/*FreeRTOS include*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

// TODO use references instead of copy
event_t AdminManager_SP3D_Function( event_t ReceivedEvent )
{

	event_t ValueToSend;
	event_t TokenRequest;
	event_t EventToSend;
	event_t TokenResponse;

	/*Identify the target of command
	 */
	while(1)
	{
		switch(ReceivedEvent.eventType){
		case INT_RESP_3:
			DEBUG(TRACE,"Sending command to destination\r\n");
			//commandcpy( &ValueToSend.eventData.command , &ReceivedEvent.eventData.incomingMessage.command );
			//ValueToSend.eventType = INT_RESP_3;

			// TODO implement route Response_SP3D
			//ReceivedEvent = routeResponse_SP3D( ValueToSend );
			ReceivedEvent.eventType = RESPONSE;

			continue;

		case EXT_MESSAGE:
			DEBUG(TRACE,"Sending command to token validator\r\n");
			eventcpy( &TokenRequest , &ReceivedEvent );

			//TokenResponse = TokenValidateFunction(TokenRequest);

			TokenResponse.eventType = RESPONSE;
			TokenResponse.eventData.response.responsecode = SUCCESS;

			if (TokenResponse.eventType == RESPONSE && TokenResponse.eventData.response.responsecode == SUCCESS){
				DEBUG(TRACE,"Sending command to destination\r\n");
				commandcpy( &ValueToSend.eventData.command , &ReceivedEvent.eventData.incomingMessage.command );
				ValueToSend.eventType=EXT_COMMAND;

				ReceivedEvent = routeCommand_SP3D( ValueToSend );

				continue;
			}
			else{
				DEBUG(TRACE,"Sending response\r\n");
				eventcpy(&EventToSend, &TokenResponse);

				return EventToSend;
			}
			break;
		case EXT_COMMAND:
			DEBUG(TRACE,"AdminManager: COMMAND Event Type not supported\r\n");
			break;

		case INT_MESS_0:

		case RESPONSE:
			DEBUG(TRACE,"Proceeding response\r\n");
			eventcpy( &EventToSend , &ReceivedEvent );

			return EventToSend;

			break;
		default:
			DEBUG(TRACE,"AdminManager: Unknown Event Type\r\n");
			break;
		}
	}
}

/*-----------------------------------------------------------*/

/*
 * routeCommandSimple.c
 *
 *  Created on: 20 nov. 2017
 *      Author: hzgf0437
 */

/* Standard includes. */
//#include <stdio.h>
#include <LedManager.h>
#include "KeyManager.h"
#include "ConfigManager.h"
#include "CommonStructure.h"
#include "routeCommandSimple.h"
#include "ResponseCode.h"
#include "pip/debug.h"
#include "structcopy.h"
#include "stdint.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

//extern QueueHandle_t xQueue_AM2CM;/* Queue AdminManager 2 ConfigManager. */
//extern QueueHandle_t xQueue_AM2KM;/* Queue AdminManager 2 KeyManager. */

/*-----------------------------------------------------------*/

// TODO use references instead of copy
event_t routeCommandSimple_SP3D(event_t ReceivedValue)
{
	event_t ValueToSend;
	/*Identify the target of command
	 */
	eventcpy( &ValueToSend, &ReceivedValue);

	switch(ReceivedValue.eventData.command.instruction){
	case READ_DOMID :
	case UPDATE_DOMID :
	case CREATE_DOM :
		DEBUG(TRACE,"call config manager\n");

		return ConfigManagerFunction(ValueToSend);

		break;
	case ADD_KEY:
	case READ_KEY:
	case UPDATE_KEY:
	case DELETE_KEY:
		DEBUG(TRACE,"call key ring manager\n");

		return KeyManagerFunction(ValueToSend);

		break;

	case SET_IO:
	case GET_IO:
	case SET_IO_DIR:
	case SET_ALL_IO_DIR:
	case SET_LED:
		DEBUG(TRACE,"call LED manager\n");

		return LedManagerFunction(ValueToSend);

		break;

	default:
		DEBUG(TRACE," Unkown command instruction\n");

		return ValueToSend;
		break;
	}
}

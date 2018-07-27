/*
 * communicateSimple.c
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
#include "mystdlib.h"

#include "communicateSimple.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <queueGlue.h>
/**
 *
 * @param data
 * @param xQueue_P2IC
 * @returnsize of the stream received
 */
uint32_t receive_simple(char* data, QueueHandle_t xQueue_P2IC){

	event_t Event;
	xProtectedQueueReceive( xQueue_P2IC, &Event, portMAX_DELAY ); //receive(data, src)
	switch(Event.eventType){
	case NW_IN:
		mymemset(data, 0, IN_MAX_MESSAGE_SIZE);
		DEBUG(TRACE, "Internal Communication received a message\n");
		mymemcpy(data, Event.eventData.nw.stream, Event.eventData.nw.size);
		return Event.eventData.nw.size;
	default:
		return GENERAL_ERROR;
	}
}

void send_simple(char* data, QueueHandle_t xQueue_IC2P, uint32_t datasize){
	event_t Event;
	Event.eventType=NW_OUT;
	mymemcpy(Event.eventData.nw.stream, data, datasize);
	Event.eventData.nw.size=datasize;
	DEBUG(TRACE, "Internal Communication sent a message\n");
	xProtectedQueueSend( xQueue_IC2P, &Event, portMAX_DELAY );
}

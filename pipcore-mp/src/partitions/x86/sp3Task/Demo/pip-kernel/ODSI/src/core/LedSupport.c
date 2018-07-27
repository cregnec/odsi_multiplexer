/*
 * LedManager.c
 *
 *  Created on: 24 avr. 2018
 *      Author: swhx7055
 */

#include <LedDriver.h>
#include <LedManager.h>
#include "CommonStructure.h"
#include "ResponseCode.h"
#include "Galileo_Gen2_Board.h"
#include "GPIO_I2C.h"
#include "pip/debug.h"
#include "structcopy.h"
#include "string.h"
#include "stdint.h"

#define MIN_LED_NUMBER		0
#define MAX_LED_NUMBER		(sizeof(leds_route_tab)-1)

#define MIN_IO_NUMBER		3
#define MAX_IO_NUMBER		13

// Static functions prototypes
static void init_LEDs();
static void set_IO_value(char *data);
static int get_IO_value(char *data);
static void set_IO_dir(char *data);
static void set_All_IO_dir(char *data);
static void set_LED_value(event_t *ReceivedValue, event_t *EventToSend);

static uint8_t leds_route_tab[] = { 5, 6, 9, 11};

// TODO use references instead of copy
event_t LedManagerFunction( event_t ReceivedValue )
{
	static int LED_manager_initialized = 0;

	if(!LED_manager_initialized)
	{
		DEBUG(TRACE,"Initialize leds \r\n");
		LED_manager_initialized = 1;
		init_LEDs();
	}


/*	switch(ReceivedValue.eventType)
	{
	case INT_RESP_1:
	case INT_RESP_2:
	case INT_RESP_3:
		ReceivedValue.eventType = RESPONSE;
		return ReceivedValue;

	default:
	}*/

	//char responseData[100]={};
	event_t EventToSend;
	response_t ResponseToSend;
	int result;

	//responseData[0]='\0';
	eventreset(&EventToSend);
	responsereset(&ResponseToSend);

	DEBUG(TRACE,"LED Manager !\n");

	ResponseToSend.userID = ReceivedValue.eventData.command.userID;

	ResponseToSend.responsecode= SUCCESS ;
	strcpy(ResponseToSend.data, "\0");

	switch(ReceivedValue.eventData.command.instruction)
	{
	case SET_IO:
		set_IO_value(ReceivedValue.eventData.command.data);

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		break;

	case GET_IO:
		result = get_IO_value(ReceivedValue.eventData.command.data);
		if(result == -1)
		{
			ResponseToSend.responsecode= GENERAL_ERROR ;
		}

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		break;

	case SET_IO_DIR:
		set_IO_dir(ReceivedValue.eventData.command.data);

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		break;

	case SET_ALL_IO_DIR:
		set_All_IO_dir(ReceivedValue.eventData.command.data);

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		break;

	case SET_LED:
		//set_LED_value(ReceivedValue.eventData.command.data);
		commandcpy( &EventToSend.eventData.incomingMessage.command, &ReceivedValue.eventData.command );
		EventToSend.eventType = INT_MESS_0;
		EventToSend.eventData.incomingMessage.userID = 1;
		EventToSend.eventData.incomingMessage.deviceID = 1;
		EventToSend.eventData.incomingMessage.domainID = 0;
		EventToSend.eventData.incomingMessage.tokenSize = 0;

		break;

	default:
		DEBUG(TRACE," Unknown GPIO command instruction\n");
		break;
	}

	return EventToSend;
}

// TODO use references instead of copy
event_t LedDriverFunction( event_t ReceivedValue )
{
	static int LED_manager_initialized = 0;

	if(!LED_manager_initialized)
	{
		DEBUG(TRACE,"Initialize leds \r\n");
		LED_manager_initialized = 1;
		init_LEDs();
	}

	//char responseData[100]={};
	event_t EventToSend;
	response_t ResponseToSend;
	int result;

	//responseData[0]='\0';
	eventreset(&EventToSend);
	responsereset(&ResponseToSend);

	DEBUG(TRACE,"LED Manager !\n");

	ResponseToSend.userID = ReceivedValue.eventData.command.userID;

	ResponseToSend.responsecode= SUCCESS ;
	strcpy(ResponseToSend.data, "\0");

	switch(ReceivedValue.eventData.command.instruction)
	{
	case SET_IO:
		set_IO_value(ReceivedValue.eventData.command.data);
		EventToSend.eventType=RESPONSE;

		break;

	case GET_IO:
		result = get_IO_value(ReceivedValue.eventData.command.data);
		if(result == -1)
		{
			ResponseToSend.responsecode= GENERAL_ERROR ;
		}
		EventToSend.eventType=RESPONSE;

		break;

	case SET_IO_DIR:
		set_IO_dir(ReceivedValue.eventData.command.data);
		EventToSend.eventType=RESPONSE;

		break;

	case SET_ALL_IO_DIR:
		set_All_IO_dir(ReceivedValue.eventData.command.data);

		EventToSend.eventType=RESPONSE;
		responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

		break;

	case SET_LED:
		set_LED_value(&ReceivedValue, &EventToSend);

		break;

	default:
		DEBUG(TRACE," Unknown GPIO command instruction\n");
		break;
	}

	responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

	return EventToSend;
}

static void set_IO_value(char *data)
{
	if(data[1] != ',')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return;
	}

	char string[] = "x";

	string[0] = data[0];
	int io_number = atoi(string);
	string[0] = data[2];
	int io_value = atoi(string);

	if( io_number < MIN_IO_NUMBER || io_number > MAX_IO_NUMBER )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO number\n");
		return;
	}

	if( io_value < 0 || io_value > 1 )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO value\n");
		return;
	}

	// set io level
	Galileo_Gen2_Set_IO_Level(io_number, io_value);
}

static int get_IO_value(char *data)
{
	int ret = -1;

	if(data[1] != ',' || data[2] != '?')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return ret;
	}

	char string[] = "x";
	string[0] = data[0];
	int io_number = atoi(string);

	if( io_number < MIN_IO_NUMBER || io_number > MAX_IO_NUMBER )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO number\n");
		return ret;
	}

	// get the gpio value

	return ret;
}

static void set_IO_dir(char *data)
{
	if(data[1] != ',')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return;
	}

	char string[] = "x";

	string[0] = data[0];
	int io_number = atoi(string);
	string[0] = data[2];
	int io_dir = atoi(string);

	if( io_number < MIN_IO_NUMBER || io_number > MAX_IO_NUMBER )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO number\n");
		return;
	}

	if( io_dir < 0 || io_dir > 1 )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO direction\n");
		return;
	}

	// set gpio direction
	Galileo_Gen2_Set_IO_Direction(io_number, io_dir);
}

static void set_All_IO_dir(char *data)
{
	if(data[0] != '*')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return;
	}

	if(data[1] != ',')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return;
	}

	char string[] = "x";

	string[0] = data[2];
	int io_dir = atoi(string);

	if( io_dir < 0 || io_dir > 1 )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO direction\n");
		return;
	}

	for(uint8_t i = 0; i < sizeof(leds_route_tab); i++)
	{
		// set gpio direction
		Galileo_Gen2_Set_IO_Direction(leds_route_tab[i], io_dir);
	}

}

static void set_LED_value(event_t *ReceivedValue, event_t *EventToSend)
{
	char *data = ReceivedValue->eventData.command.data;

	if(data[1] != ',')
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO command data\n");
		return;
	}

	char string[] = "x";

	string[0] = data[0];
	uint8_t led_number = atoi(string);
	string[0] = data[2];
	uint8_t io_value = atoi(string);

	if( led_number > MAX_LED_NUMBER )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO number\n");
		return;
	}

	if( io_value > 1 )
	{
		DEBUG(TRACE,"[LED Manager] Invalid GPIO value\n");
		return;
	}

	// set io level
	Galileo_Gen2_Set_IO_Level(leds_route_tab[led_number], io_value);

	if(ReceivedValue->eventType == EXT_COMMAND)
	{
		EventToSend->eventType = RESPONSE;
	}
	else if(ReceivedValue->eventType == INT_COMMAND)
	{
		switch(led_number)
		{
		case 0:
			EventToSend->eventType = RESPONSE;
			break;
		case 1:
			EventToSend->eventType = INT_RESP_1;
			break;
		case 2:
			EventToSend->eventType = INT_RESP_2;
			break;
		case 3:
			EventToSend->eventType = INT_RESP_3;
			break;
		default:
			break;
		}
	}
}

static void init_LEDs()
{
	for(uint8_t i; i < sizeof(leds_route_tab); i++)
	{
		Galileo_Gen2_Set_IO_Direction(leds_route_tab[i], GPIO_OUTPUT);
		Galileo_Gen2_Set_IO_Level(leds_route_tab[i], LOW);
	}
}

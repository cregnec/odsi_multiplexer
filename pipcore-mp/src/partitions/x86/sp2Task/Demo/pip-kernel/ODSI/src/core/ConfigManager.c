/*
 * ConfigManager.c
 *
 *  Created on: 20 nov. 2017
 *      Author: hzgf0437
 */


/* Standard includes. */
#include "CommonStructure.h"
#include "ManageDomain_Interface.h"
#include "MyAppConfig.h"
#include "pip/debug.h"
#include "string.h"
#include "structcopy.h"
#include "stdint.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*-----------------------------------------------------------*/

// TODO use references instead of copy
event_t ConfigManagerFunction( event_t ReceivedValue )
{
	DEBUG(TRACE,"Config Manager !\n");

	static int config_manager_initialized = 0;
	static domain_t CurrentDomain;

	if(!config_manager_initialized)
	{
		config_manager_initialized = 1;
		/*Initialize current domain*/
		strcpy(CurrentDomain.DomId,DOM_ID_OWNER);
		CurrentDomain.nbChilDom=NB_CHILD_DOM_DEFAULT;
		DEBUG(TRACE,"[ConfigManager] Initialize Domain\r\n");
	}

	char responseData[100]={};
	event_t EventToSend;
	response_t ResponseToSend;
	responseData[0]='\0';
	eventreset(&EventToSend);
	responsereset(&ResponseToSend);

	ResponseToSend.responsecode=ManageDomain(ReceivedValue,&CurrentDomain,responseData);

	if(responseData[0] != '\0')
	{
		strcpy(ResponseToSend.data,responseData);
	}

	ResponseToSend.userID = ReceivedValue.eventData.command.userID;

	EventToSend.eventType=RESPONSE;
	responsecpy(&(EventToSend.eventData.response), &(ResponseToSend));

	DEBUG(TRACE,"Config-Response code: %#04X, Data: %s\n", ResponseToSend.responsecode, ResponseToSend.data);

	return EventToSend;
}


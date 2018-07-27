/*
 * ManageDomain_Interface.c
 *
 *  Created on: 20 nov. 2017
 *      Author: hzgf0437
 */

/*-----------------------------------------------------------*/

/* Standard includes. */
#include "ManageDomainSimple.h"
#include "CommonStructure.h"
#include "ManageDomain_Interface.h"
#include "ResponseCode.h"
#include "pip/debug.h"

/* Kernel includes. */


/*-----------------------------------------------------------*/

int ManageDomain(event_t com, domain_t *p_dom, char* readData){
		switch(com.eventData.command.instruction){
		case READ_DOMID :
			return readDomID(p_dom, readData);
			break;
		case UPDATE_DOMID :
			return updateDomID(p_dom,com.eventData.command.data);
			break;
		case CREATE_DOM :
			return createDomain();
			break;
		default:
			debug("command is not supported\n");
			return UNKNOWN_COMMAND;
			break;
		}
}

/*
 * ManageDomain.c
 *
 *  Created on: 20 nov. 2017
 *      Author: hzgf0437
 */

/*-----------------------------------------------------------*/

#include "pip/debug.h"
#include "string.h"
#include "CommonStructure.h"
#include "ResponseCode.h"
#include "stdint.h"

/*-----------------------------------------------------------*/

//domain_t CurrentDomain = {DOM_ID_OWNER,NB_CHILD_DOM_DEFAULT};

uint32_t createDomain(){
	DEBUG(TRACE, "Create Domain\n");
	return SUCCESS;
}


uint32_t readDomID(domain_t *dom, char* readData){
	DEBUG(TRACE,"Read ID: %s \n", dom->DomId);
	strcpy(readData, dom->DomId);
	return SUCCESS;
}


uint32_t updateDomID(domain_t *dom, char* domId){
	DEBUG(TRACE,"Update Domain ID: Old ( %s )\n", dom->DomId);

	strcpy(dom->DomId,domId);

	DEBUG(TRACE,"Update Domain ID: New ( %s )\n", dom->DomId);

	return SUCCESS;
}

/*int nbDomChild(domain_t dom){
	printf("Command is not supported");
	fflush( stdout );
	return UNKNOWN_COMMAND;
}*/

/*-----------------------------------------------------------*/

/*
 * ManageKey_SimpleAdaptor.c
 *
 *  Created on: 24 nov. 2017
 *      Author: hzgf0437
 */


/*-----------------------------------------------------------*/

/* Standard includes. */
#include "manageKeySimple.h"
#include "CommonStructure.h"
#include "ManageKey_Interface.h"
#include "ResponseCode.h"
#include "MyAppConfig.h"
#include "string.h"
#include "pip/debug.h"

/* Kernel includes. */

/*-----------------------------------------------------------*/


int ManageKey(command_t com, key_t** l_key, char* readData){

	char key_ID[KEYID_SIZE]={};
	char keyValue[KEY_SIZE]={};
	int result;

	int i;
	for(i=0;com.data[i] != ':';i++)
		key_ID[i]=com.data[i];

	strcpy(keyValue, com.data+i+1);


	switch(com.instruction){
	case ADD_KEY :
		DEBUG(TRACE,"add Key\n");
		return addKey(l_key, key_ID, keyValue);
	case READ_KEY :
		DEBUG(TRACE,"read Key\n");
		result=readKey(*l_key, key_ID, readData);
		return result;
	case UPDATE_KEY :
		DEBUG(TRACE,"update Key, new value: %s\n", keyValue);
		result=updateKey(*l_key, key_ID, keyValue);
		return result;
		break;
	case DELETE_KEY :
		DEBUG(TRACE, "delete Key\n");
		return deleteKey(l_key, key_ID);
	default:
		DEBUG(TRACE,"command is not supported\n");
		return UNKNOWN_COMMAND;
		break;
	}
}

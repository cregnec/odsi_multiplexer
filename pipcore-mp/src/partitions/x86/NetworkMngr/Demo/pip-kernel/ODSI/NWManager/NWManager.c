/*
 * NWManager.c
 *
 *  Created on: 1 April 2018
 *  Author: SWHX7055
 */


/* Standard includes. */
#include "stdint.h"
#include "domains.h"
#include "UART_DMA.h"
#include "esp8266.h"
#include "Quark_x1000_support.h"
#include "Galileo_Gen2_Board.h"
#include "CommonStructure.h"
#include "MyAppConfig.h"
#include "ResponseCode.h"
#include "mystdlib.h"
#include "string.h"
#include "structcopy.h"
#include "parser.h"
#include "NWManager.h"
#include "NWManager_Interface.h"
#include "pip/debug.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queueGlue.h"

#include <pip/fpinfo.h>
#include <pip/debug.h>
#include <pip/paging.h>
#include <pip/compat.h>

// static function prototypes
static int dispatch_message_to_domain(QueueHandle_t xQueue_domains_array[], event_t *p_EventRequest, uint32_t size_in);
static int rcv_tcp_segment(char *rcv_buf);
static int check_send_ok(char *rcv_buf);
static int check_send_fail(char *rcv_buf);
static int check_esp8266_received_data(char *rcv_buf);
static void read_from_esp8266();
static int check_esp8266_busy(char *rcv_buf);
static int check_esp8266_error(char *rcv_buf);
static int check_client_connect(char *rcv_buf);
static int check_client_connect_fail(char *rcv_buf);
static int check_esp8266_ok(char *rcv_buf);
static int check_esp8266_ready_to_send_tcp_payload(char *rcv_buf);
static int check_client_disconnect(char *rcv_buf);
static int check_wifi_connected(char *rcv_buf);
static int check_wifi_disconnect(char *rcv_buf);
static int check_wifi_got_ip(char *rcv_buf);
static int link_is_not_valid(char *rcv_buf);

// static variables
static uint8_t esp8266_responsive = 1;
static uint8_t wifi_connected = 0;
static uint8_t wifi_got_ip = 0;
static uint8_t client_connected = 0;
static uint8_t esp8266_busy = 0;
static uint8_t sending_tcp_payload = 0;
static uint8_t send_fail = 0;
static uint8_t send_ok = 0;
static uint8_t error = 0;
static uint8_t got_ok = 0;
static uint8_t esp8266_ready_to_send_tcp_payload = 0;
static uint8_t waiting_tcp_header_reception_ack = 0;
static uint8_t v_link_is_not_valid = 0;
static uint8_t fatal_error = 0;

void NW_Task( uint32_t *pvParameters )
{
	QueueHandle_t xQueue_2NW = (QueueHandle_t) pvParameters[0];
	QueueHandle_t xQueue_domains_array[4];
	xQueue_domains_array[0] = (QueueHandle_t) pvParameters[1];
	xQueue_domains_array[1] = (QueueHandle_t) pvParameters[2];
	xQueue_domains_array[2] = (QueueHandle_t) pvParameters[3];
	xQueue_domains_array[3] = (QueueHandle_t) pvParameters[4];

	event_t *ICEvent = (event_t *) allocPage();
	event_t *EventRequest = (event_t *) allocPage();

	printf("ICEvent : %x\r\n", ICEvent);
	printf("EventRequest : %x\r\n", EventRequest);

	uint32_t size_in = 0, sizeout = 0;

	void* ServerSocket = NULL;
	void* ClientSocket = NULL;

	// init reset pin of the esp8266
	//esp8266_init_reset_pin(); // Done by the root domain
	// hard reset the esp8266

	esp8266_hard_reset();
	printf("Go back to Network Task\r\n");
	//wait ...
	const TickType_t xDelay_5_sec = 5;
	printf("Waiting 5 secs\r\n");
	vTaskDelay( xDelay_5_sec );
	printf("Starting work\r\n");
	for(;;)
	{
		esp8266_responsive = 1;
		wifi_connected = 0;
		wifi_got_ip = 0;
		client_connected = 0;
		esp8266_busy = 0;
		sending_tcp_payload = 0;
		send_fail = 0;
		error = 0;
		fatal_error = 0;

		// blocking function call until esp8266 successfully initialized
		printf("Initialize serversocket\r\n");
		ServerSocket = initialize();

		// create TCP server
		uint32_t l_result = 0;
		printf("create TCP server\r\n");
		l_result = ext_listen( ServerSocket);

		// reaching here means wifi is connected and has gotten ip addr
		wifi_connected = 1;
		wifi_got_ip = 1;

		while(esp8266_responsive && l_result && !fatal_error)
		{
			printf("receive and dispatch arrived data if any\r\n");
			/*
			 * receive and dispatch arrived data if any
			 */
			//memset(InValue, 0, IN_MAX_MESSAGE_SIZE*sizeof(char));
			// TODO remove the IN_MAX_MESSAGE_SIZE bytes receive limit using receive FIFO
			size_in = ext_receive(ClientSocket, EventRequest->eventData.nw.stream);
			if(size_in > 0)
			{
				EventRequest->eventType = NW_IN;
				EventRequest->eventData.nw.size=size_in;
				// dispatch
				printf("[Network Manager] dispatch_message_to_domain\r\n");
				dispatch_message_to_domain(xQueue_domains_array, EventRequest, size_in);
				printf("Back from dispatch message to domain !!!\r\n");
			}

			/*
			 * send data if any
			 */
			if( xQueue_2NW != 0 )
			{
				/* Receive Response data
				 * Don't block if nothing to read. */
				printf("[Network Manager] xQueueReceive( xQueue_2NW, ICEvent...\r\n");
				if( xProtectedQueueReceive( (uint32_t)xQueue_2NW, (uint32_t)ICEvent, ( TickType_t ) 0 ) )
				{
					if(ICEvent->eventType == NW_OUT)
					{
						printf("[Network Manager] ext_send(...\r\n");
						ext_send(ClientSocket, ICEvent->eventData.nw.stream, ICEvent->eventData.nw.size);
					}
				}
			}

			/*
			 * send tcp header if any
			 */
			printf("[Network Manager] send tcp header if any\r\n");
			if(		client_connected && wifi_connected && wifi_got_ip &&
					!sending_tcp_payload  && !waiting_tcp_header_reception_ack)
			{
				sizeout = try_to_send_tcp_header();

				if(sizeout > 0)
				{
					waiting_tcp_header_reception_ack = 1;
				}
			}

			/*
			 * check header send failure
			 */
			if( waiting_tcp_header_reception_ack && error && v_link_is_not_valid )
			{
				// update states
				v_link_is_not_valid = 0;
				error = 0;
				waiting_tcp_header_reception_ack = 0;
				esp8266_busy = 0;
			}

			/*
			 * send tcp payload if esp8266 ready
			 */
			if(		got_ok && esp8266_ready_to_send_tcp_payload &&
					waiting_tcp_header_reception_ack && !sending_tcp_payload)
			{
				// update states
				got_ok = 0;
				esp8266_ready_to_send_tcp_payload = 0;
				waiting_tcp_header_reception_ack = 0;
				sending_tcp_payload = 1;
				esp8266_busy = 0;
				// send payload
				send_tcp_payload(sizeout);
			}

			if(send_fail && sending_tcp_payload) // re-send if previous send fail (in the next iteration)
			{
				// update states
				send_fail = 0;
				sending_tcp_payload = 0;
				esp8266_busy = 0;
			}

			if(send_ok && sending_tcp_payload)
			{
				// update states
				send_ok = 0;
				sending_tcp_payload = 0;
				esp8266_busy = 0;
				// Pull the item successfully sent from the tcp send fifo
				remove_tcp_payload_from_send_fifo(sizeout);
			}

			/*
			 * read esp8266 feed via UART and update states.
			 */
			read_from_esp8266();

		}
		mycloseSocket(ClientSocket);
	}

}

/*
 * This function read the feed from esp8266 UART communication.
 * It updated the global state variables like client_connected, wifi_got_ip (wifi_disconnect) and esp8266_busy etc...
 * In case of tcp segment arrival it calls specific function to do the necessary.
 * This function returns if nothing to read.
 */
static void read_from_esp8266()
{
	char rcv_buf[100];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	int read_size = 0;

	if(vGalileo_UART0_is_data_available())
	{
		// read only 2 characters (Maybe "> ")
		read_size = vGalileo_UART0_read_line(rcv_buf, 2);
	}

	// returns if nothing to read
	if(read_size < 1)
	{
		return;
	}


	/*
	 * MUST be the first test (Mandatory) because it doesn't end with newline !!!
	 */
	if(read_size == 2)
	{
		/*
		 * check if it is a "> " then return if yes
		 */
		if(check_esp8266_ready_to_send_tcp_payload(rcv_buf))
		{
			esp8266_ready_to_send_tcp_payload = 1;
			return;
		}
		else
		{
			if(vGalileo_UART0_is_data_available())
			{
				// try to read only 3 more characters (Maybe "+IPD,")
				read_size += vGalileo_UART0_read_line(&(rcv_buf[2]), 3);
			}
		}
	}

	/*
	 * MUST be the second test (Mandatory) because it doesn't end with newline !!!
	 */
	if(read_size == 5)
	{
		// check if it is a tcp segment reception then return if yes
		int ret = rcv_tcp_segment(rcv_buf);
		printf("return from rcv_tcp_segment\r\n");
		if(ret)
		{
			return;
		}
		else
		{
			if(vGalileo_UART0_is_data_available())
			{
				// try to continue line reading if it wasn't a tcp segment reception and read_size == 5
				read_size += vGalileo_UART0_read_line(&(rcv_buf[5]), sizeof(rcv_buf) - 5);
			}
		}
	}

	/*
	 * The tests below are sorted in occurrence probability order in order to increase performance.
	 */

	/*
	 * check if it is a send ok then return if yes
	 */
	if( read_size == 7 )
	{
		if(check_send_ok(rcv_buf))
		{
			// update states
			send_ok = 1;
			return;
		}
	}

	/*
	 * check if it is a send fail then return if yes
	 */
	if( read_size == 9 )
	{
		if(check_send_fail(rcv_buf))
		{
			send_fail = 1;
			return;
		}
	}

	/*
	 * check if esp8266 received data for send and return if yes
	 */
	if( read_size >= 12 && read_size <= 15 )
	{
		if(check_esp8266_received_data(rcv_buf))
		{
			return;
		}
	}

	/*
	 * check if esp8266 is busy then return if yes
	 */
	if( read_size == 9 )
	{
		if(check_esp8266_busy(rcv_buf))
		{
			esp8266_busy = 1;
			return;
		}
	}

	/*
	 * check if it is an ERROR then return if yes
	 */
	if( read_size == 5 )
	{
		if(check_esp8266_error(rcv_buf))
		{
			error = 1;
			return;
		}
	}

	/*
	 * check if link is not valid then return if yes
	 */
	if( read_size == 5 )
	{
		if(link_is_not_valid(rcv_buf))
		{
			// update states
			v_link_is_not_valid = 1;
			return;
		}
	}

	/*
	 * check if it is a client connection then return if yes
	 */
	if( read_size == 9 )
	{
		if(check_client_connect(rcv_buf))
		{
			client_connected = 1;
			return;
		}
	}

	/*
	 * check if it is a client connection fail then return if yes
	 */
	if( read_size == 14 )
	{
		if(check_client_connect_fail(rcv_buf))
		{
			client_connected = 0;
			return;
		}
	}

	/*
	 * check if it is a "OK" then return if yes
	 */
	if( read_size == 2 )
	{
		if(check_esp8266_ok(rcv_buf))
		{
			got_ok = 1;
			return;
		}
	}

	/*
	 * check if it is a client disconnection then return if yes
	 */
	if( read_size == 8 )
	{
		if(check_client_disconnect(rcv_buf))
		{
			client_connected = 0;
			return;
		}
	}

	/*
	 * check if it is a wifi connection then return if yes
	 */
	if( read_size == 14 )
	{
		if(check_wifi_connected(rcv_buf))
		{
			wifi_connected = 1;
			return;
		}
	}

	/*
	 * check if it is a wifi disconnection then return if yes
	 */
	if( read_size == 15 )
	{
		if(check_wifi_disconnect(rcv_buf))
		{
			wifi_connected = 0;
			wifi_got_ip = 0;
			return;
		}
	}

	/*
	 * check if it is a wifi got ip then return if yes
	 */
	if( read_size == 11 )
	{
		if(check_wifi_got_ip(rcv_buf))
		{
			wifi_got_ip = 1;
			return;
		}
	}

	// we shouldn't arrive here
	fatal_error = 1;
}

/*
 * try to receive tcp segement if any.
 *
 * returns 1 if succeeded
 */
static int rcv_tcp_segment(char *rcv_buf)
{
	int ret = 0;

	// check if receiving TCP segment (+IPD)
	if(esp8266_check_IPD_reception(rcv_buf))
	{
		ret = 1;

		unsigned int received_bytes_count = 0;
		// continue reading the rest of the TCP header
		printf("get_tcp_header\r\n");
		received_bytes_count = esp8266_get_tcp_header(NULL);

		if(received_bytes_count > 0)
		{
			printf("get_tcp_payload: %d\r\n", received_bytes_count);
			// read TCP segment payload
			esp8266_get_tcp_payload(received_bytes_count);
		}
	}

	return ret;
}

/*
 * check "SEND OK" reception
 *
 * returns 1 if succeeded
 */
static int check_send_ok(char *rcv_buf)
{
	int ret = 0;

	// test if we received "SEND OK"
	if(esp8266_response_contains(rcv_buf, "SEND OK"))
	{
		DEBUG(TRACE, "[ESP8266] SEND OK\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * check "SEND FAIL" reception
 *
 * returns 1 if succeeded
 */
static int check_send_fail(char *rcv_buf)
{
	int ret = 0;

	// test if we received "SEND FAIL"
	if(esp8266_response_contains(rcv_buf, "SEND FAIL"))
	{
		DEBUG(TRACE, "[ESP8266] SEND FAIL\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO detect if "busy s..." or "busy p..."
 * check "busy s..." or "busy p..." reception
 *
 * returns 1 if succeeded
 */
static int check_esp8266_busy(char *rcv_buf)
{
	int ret = 0;

	// test if we received "busy s..." or "busy p..."
	if(esp8266_response_contains(rcv_buf, "busy"))
	{
		DEBUG(TRACE, "[ESP8266] busy\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * check "ERROR" reception
 *
 * returns 1 if succeeded
 */
static int check_esp8266_error(char *rcv_buf)
{
	int ret = 0;

	// test if we received "ERROR"
	if(esp8266_response_contains(rcv_buf, "ERROR"))
	{
		DEBUG(TRACE, "[ESP8266] ERROR\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * check "link is not valid" reception
 *
 * returns 1 if succeeded
 */
static int link_is_not_valid(char *rcv_buf)
{
	int ret = 0;

	// test if we received "link is not valid"
	if(esp8266_response_contains(rcv_buf, "link is not valid"))
	{
		DEBUG(TRACE, "[ESP8266] link is not valid\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO check the link number
 * check ",CONNECT" reception
 *
 * returns 1 if succeeded
 */
static int check_client_connect(char *rcv_buf)
{
	int ret = 0;

	// test if we received ",CONNECT"
	if(esp8266_response_contains(&(rcv_buf[1]), ",CONNECT"))
	{
		DEBUG(TRACE, "[ESP8266] client ,CONNECT\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO check the link number
 * check ",CONNECT FAIL" reception
 *
 * returns 1 if succeeded
 */
static int check_client_connect_fail(char *rcv_buf)
{
	int ret = 0;

	// test if we received ",CONNECT FAIL"
	if(esp8266_response_contains(&(rcv_buf[1]), ",CONNECT FAIL"))
	{
		DEBUG(TRACE, "[ESP8266] client ,CONNECT FAIL\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO remove code duplication compare with manageNW_Simple.c !!!
 * check "OK" reception
 *
 * returns 1 if succeeded
 */
static int check_esp8266_ok(char *rcv_buf)
{
	int ret = 0;

	// test if we received "OK"
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, "[ESP8266] OK\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * check "> " reception
 *
 * returns 1 if succeeded
 */
static int check_esp8266_ready_to_send_tcp_payload(char *rcv_buf)
{
	int ret = 0;

	// test if we received "> "
	if(esp8266_response_contains(rcv_buf, "> "))
	{
		DEBUG(TRACE, "[ESP8266] ><space>\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO check the link number
 * check ",CLOSED" reception
 *
 * returns 1 if succeeded
 */
static int check_client_disconnect(char *rcv_buf)
{
	int ret = 0;

	// test if we received ",CLOSED"
	if(esp8266_response_contains(&(rcv_buf[1]), ",CLOSED"))
	{
		DEBUG(TRACE, "[ESP8266] client ,CLOSED\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO remove code duplication compare with manageNW_Simple.c !!!
 * check "WIFI CONNECTED" reception
 *
 * returns 1 if succeeded
 */
static int check_wifi_connected(char *rcv_buf)
{
	int ret = 0;

	// test if we received "WIFI CONNECTED"
	if(esp8266_response_contains(rcv_buf, "WIFI CONNECTED"))
	{
		DEBUG(TRACE, "[ESP8266] WIFI CONNECTED\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO remove code duplication compare with manageNW_Simple.c !!!
 * check "WIFI DISCONNECT" reception
 *
 * returns 1 if succeeded
 */
static int check_wifi_disconnect(char *rcv_buf)
{
	int ret = 0;

	// test if we received "WIFI DISCONNECT"
	if(esp8266_response_contains(rcv_buf, "WIFI DISCONNECT"))
	{
		DEBUG(TRACE, "[ESP8266] WIFI DISCONNECT\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * TODO remove code duplication compare with manageNW_Simple.c !!!
 * check "WIFI GOT IP" reception
 *
 * returns 1 if succeeded
 */
static int check_wifi_got_ip(char *rcv_buf)
{
	int ret = 0;

	// test if we received "WIFI GOT IP"
	if(esp8266_response_contains(rcv_buf, "WIFI GOT IP"))
	{
		DEBUG(TRACE, "[ESP8266] WIFI GOT IP\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * check "Recv <nbre_of_bytes> bytes" reception
 *
 * returns 1 if succeeded
 */
static int check_esp8266_received_data(char *rcv_buf)
{
	int ret = 0;

	// test if we received "Recv <nbre_of_bytes> bytes"
	if(esp8266_response_contains(rcv_buf, "Recv "))
	{
		DEBUG(TRACE, "[ESP8266] Recv <nbre_of_bytes> bytes\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * Sync call (bloquant)
 * Try to read the domainID in order to dispatch the message
 * to the destination domain.
 *
 * return 1 if succeeded
 */
static int dispatch_message_to_domain(QueueHandle_t xQueue_domains_array[], event_t *p_EventRequest, uint32_t size_in)
{
	int ret = 0;
	event_t *EventToDisptach = allocPage();

	if(EventToDisptach == NULL)
	{
		printf("AllocPage failed !!!\r\n");
		return 0;
	}

	QueueHandle_t xQueue_2OD_IC = xQueue_domains_array[0];
	QueueHandle_t xQueue_2SP1D_IC = xQueue_domains_array[1];
	QueueHandle_t xQueue_2SP2D_IC = xQueue_domains_array[2];
	QueueHandle_t xQueue_2SP3D_IC = xQueue_domains_array[3];

	// deserialize the incoming message
	EventToDisptach->eventData.incomingMessage = deserialize_incomingMessage(p_EventRequest->eventData.nw.stream, size_in);
	printf("Back from deserialize\r\n");
	EventToDisptach->eventType = EXT_MESSAGE;

	// send the message to corresponding domain if any.
	switch(EventToDisptach->eventData.incomingMessage.domainID)
	{
	case DEMO_OWNER_DOMAIN_ID: // owner domain
		if( xQueue_2OD_IC != 0 )
		{
			/* block if the queue is already full. */
			xProtectedQueueSend( xQueue_2OD_IC, EventToDisptach, portMAX_DELAY );
			ret = 1;
		}
		break;
	case DEMO_SERVICE_PROVIDER_1_DOMAIN_ID: // sp 1
		if( xQueue_2SP1D_IC != 0 )
		{
			/* block if the queue is already full. */
			xProtectedQueueSend( xQueue_2SP1D_IC, EventToDisptach, portMAX_DELAY );
			ret = 1;
		}
		break;
	case DEMO_SERVICE_PROVIDER_2_DOMAIN_ID: // sp 2
		if( xQueue_2SP2D_IC != 0 )
		{
			/* block if the queue is already full. */
			xProtectedQueueSend( xQueue_2SP2D_IC, EventToDisptach, portMAX_DELAY );
			ret = 1;
		}
		break;
	case DEMO_SERVICE_PROVIDER_3_DOMAIN_ID: // sp 3
		if( xQueue_2SP3D_IC != 0 )
		{
			/* block if the queue is already full. */
			xProtectedQueueSend( xQueue_2SP3D_IC, EventToDisptach, portMAX_DELAY );
			ret = 1;
		}
		break;
	default:
		DEBUG(TRACE, "[NW_Manager] Incoming message rejected\r\n");
	}

	freePage(EventToDisptach);

	printf("just before return from dispatch message to domain\r\n");

	return ret;
}

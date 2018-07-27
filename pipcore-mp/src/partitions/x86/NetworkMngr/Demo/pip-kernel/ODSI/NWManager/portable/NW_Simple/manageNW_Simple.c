/*
 * manageNW_simple.c
 *
 *  Created on: 14 d�c. 2017
 *      Author: hzgf0437
 */

#include <esp8266.h>
#include <Quark_x1000_support.h>
#include <stdio.h>

/* App includes */
#include "FIFO.h"
#include "UART_DMA.h"
/* Standard includes. */
#include "CommonStructure.h"
#include "MyAppConfig.h"
#include "ResponseCode.h"
#include "pip/debug.h"
#include "string.h"
#include "structcopy.h"
#include "parser.h"
#include "NWManager_Interface.h"
#include "stdint.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <pip/fpinfo.h>
#include <pip/debug.h>
#include <pip/paging.h>
#include <pip/compat.h>

// static functions prototypes
static int esp8266_restart();
static int esp8266_wait_for_wifi_connection();
static int esp8266_wait_for_ip_addr_attribution();
static int esp8266_test_serial_comm();
static int esp8266_disable_echoing();
static int esp8266_disable_transparent_transmission();
static int esp8266_enable_multiple_connections();
static int esp8266_set_maximum_connections_to_one();
static int esp8266_create_tcp_server_port_8080();
static int esp8266_set_tcp_srv_timeout(unsigned int timeout);
static int esp8266_hide_remote_IP_port_with_IPD();

static int esp8266_get_tcp_segment_payload_length();
static char esp8266_get_link_number();

static int esp8266_send_tcp_payload(char *payload, unsigned int payload_size);

static fifo_t tcp_rcv_fifo;
static fifo_t tcp_send_fifo;

/*-----------------------------------------------------------*/
uint32_t iteration=0;

/*-----------------------------------------------------------*/

// TODO Create a network structure to be returned by the initialize method in order to identify the success from failure
// TODO use a timeout instead of indeterministic loop
void* initialize()
{
        const TickType_t xDelay_2_sec = 2;
	int esp8266_init_error = 0;

	fifo_init(&tcp_rcv_fifo);
	fifo_init(&tcp_send_fifo);

	do
	{
		esp8266_init_error = 0;

		DEBUG(TRACE, "\r\nESP8266 initialization ...\r\n");

		// restart esp8266 (try soft restart then hard restart if soft restart is not working)
		// wait for "ready" message sent by esp8266
		if(!esp8266_restart())
		{
			DEBUG(TRACE, "ESP8266 restart [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 restart [OK]\r\n");

		// We could continue the initialization process and even the server creation process
		// instead of waiting for the wifi connection and the ip address attribution,
		// but no advantage in doing that. Moreover, if wifi connection succeeded during
		// initialization process, "WIFI CONNECTED" message could disrupt the process.

		// wait for wifi connection

		DEBUG(TRACE, "ESP8266 waiting WiFi connection ...\r\n");

		if(!esp8266_wait_for_wifi_connection())
		{
			DEBUG(TRACE, "ESP8266 WiFi connection [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 WiFi connection [OK]\r\n");

		// wait for IP address attribution
		if(!esp8266_wait_for_ip_addr_attribution())
		{
			DEBUG(TRACE, "ESP8266 WiFi got IP addr [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 WiFi got IP addr [OK]\r\n");

		// stop echoing
		if(!esp8266_disable_echoing())
		{
			DEBUG(TRACE, "ESP8266 disable echoing [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 disable echoing [OK]\r\n");

		// test serial communication with the esp8266 module
		if(!esp8266_test_serial_comm())
		{
			DEBUG(TRACE, "ESP8266 test serial communication [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 test serial communication [OK]\r\n");

		// disable transparent transmission (mandatory in order to activate multiple connections)
		if(!esp8266_disable_transparent_transmission())
		{
			DEBUG(TRACE, "ESP8266 disable transparent transmission [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 disable transparent transmission [OK]\r\n");

		// activate multiple connections (mandatory to create TCP server)
		if(!esp8266_enable_multiple_connections())
		{
			DEBUG(TRACE, "ESP8266 enable multiple connections [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 enable multiple connections [OK]\r\n");

		// sets the maximum connections allowed by server to one (must be called before server creation)
		/*if(!esp8266_set_maximum_connections_to_one())
		{
			DEBUG(TRACE, "ESP8266 set the maximum connections to one [failed]\r\n");
			vTaskDelay( xDelay_2_sec );
			esp8266_init_error = 1;
			continue;
		}
		DEBUG(TRACE, "ESP8266 set the maximum connections to one [OK]\r\n");
	*/

	} while(esp8266_init_error);

	return NULL;
}

// TODO use a timeout instead of indeterministic loop
// TODO detect when a client is connected
// TODO LED monitoring for a client connection
void* get_connection(void* ListenSocket){
	/* Remove compiler warning about unused parameter. */
	(void) ListenSocket;

	return NULL;
}

// TODO use a meaningful return value
// TODO use a timeout instead of indeterministic loop
// returns 1 if succeeded
uint32_t ext_listen( void* LSocket){
	/* Remove compiler warning about unused parameter. */
	(void) LSocket;

	const TickType_t xDelay_2_sec = 2;
	int ret = 0;

	// create TCP server at port 8080
	if(!esp8266_create_tcp_server_port_8080())
	{
		DEBUG(TRACE, "ESP8266 create TCP server at port 8080 [failed]\r\n");
		//vTaskDelay( xDelay_2_sec );
		return ret;
	}
	DEBUG(TRACE, "ESP8266 create TCP server at port 8080 [OK]\r\n");

	// sets the TCP server Timeout to never
	if(!esp8266_set_tcp_srv_timeout(ESP8266_AT_TCP_SRV_TIMEOUT))
	{
		DEBUG(TRACE, "ESP8266 sets the TCP server Timeout to 180 seconds [failed]\r\n");
		//vTaskDelay( xDelay_2_sec );
		return ret;
	}
	DEBUG(TRACE, "ESP8266 sets the TCP server Timeout to 180 seconds [OK]\r\n");

	// Hide the remote IP and Port with +IPD
	if(!esp8266_hide_remote_IP_port_with_IPD())
	{
		DEBUG(TRACE, "ESP8266 Hide the remote IP and Port with +IPD [failed]\r\n");
		//vTaskDelay( xDelay_2_sec );
		return ret;
	}
	DEBUG(TRACE, "ESP8266 Hide the remote IP and Port with +IPD [OK]\r\n");

	ret = 1;

	return ret;
}

/*
 * parameter data must point to an array of IN_MAX_MESSAGE_SIZE bytes minimum !!!
 * returns the number of bytes actually read
 */
uint32_t ext_receive(void* ClientSocket, char* data)
{
	/* Remove compiler warning about unused parameter. */
	(void) ClientSocket;

	uint32_t target_bytes_count_to_receive = 0;

	/*
	 * check data parameter is not null
	 */
	if(data == NULL)
	{
		DEBUG(CRITICAL, "[ext_receive] data parameter pointer is NULL.\r\n");
		return target_bytes_count_to_receive;
	}

	/*
	 * check "ODSI" reception
	 */
	uint8_t peek_data_length = 4;
	target_bytes_count_to_receive = fifo_peek(&tcp_rcv_fifo, data, peek_data_length);
	data[4] = '\0';

	if( (target_bytes_count_to_receive == peek_data_length) && (strcmp(data, "ODSI") == 0) )
	{
		/*
		 * peek "OSDI" + received size attribute (4 bytes) from fifo
		 */
		peek_data_length = 8;
		target_bytes_count_to_receive = fifo_peek(&tcp_rcv_fifo, data, peek_data_length);

		if(target_bytes_count_to_receive == peek_data_length)
		{
			target_bytes_count_to_receive = *((uint32_t*)(data+4));
			uint32_t fifo_new_length = fifo_get_length(&tcp_rcv_fifo) - peek_data_length;

			if( (target_bytes_count_to_receive > 0) && (target_bytes_count_to_receive <= fifo_new_length) )
			{
				if(target_bytes_count_to_receive > IN_MAX_MESSAGE_SIZE)
				{
					target_bytes_count_to_receive = IN_MAX_MESSAGE_SIZE;
					DEBUG(CRITICAL, "[ext_receive] Received size attribute is bigger than IN_MAX_MESSAGE_SIZE. extra data ignored\r\n");
				}

				fifo_pull(&tcp_rcv_fifo, data, peek_data_length);
				fifo_pull(&tcp_rcv_fifo, data, target_bytes_count_to_receive);
			}
			else
			{
				target_bytes_count_to_receive = 0;
			}
		}
		else
		{
			target_bytes_count_to_receive = 0;
		}
	}
	else
	{
		// pull a character
		fifo_pull(&tcp_rcv_fifo, data, 1);
		target_bytes_count_to_receive = 0;
	}

	return target_bytes_count_to_receive;
}

/*
 * TODO this function needs to return something like (success or failure)
 * TODO use mutex to support multiple task access
 *
 */
void ext_send(void* ClientSocket, char* outData, uint32_t size){
	/* Remove compiler warning about unused parameter. */
	(void) ClientSocket;

	//char magic_data[4] = { 'O', 'D', 'S', 'I'};

	//fifo_push(&tcp_send_fifo, magic_data, sizeof(magic_data));
	fifo_push(&tcp_send_fifo, (char*)&size, sizeof(size));
	fifo_push(&tcp_send_fifo, outData, size);
}

void mycloseSocket(void* Socket){
	/* Remove compiler warning about unused parameter. */
	(void) Socket;
}

/*
 * soft restart the esp8266
 */
static void esp8266_soft_restart()
{
	//DEBUG(TRACE, "ESP8266 soft restart ...\r\n");
  printf("ESP8266 soft restart ...\r\n");
	// send AT+RST
	printf("%s\r\n",(const char *)ESP8266_AT_SOFT_RESTART);
  vInitializeGalileo_client_SerialPort();
	vGalileo_UART0_write((const char *)ESP8266_AT_SOFT_RESTART, strlen((const char *)ESP8266_AT_SOFT_RESTART));

}

/*
 * Try to read messages sent by the esp8266 until "ready" message found,
 * or no more messages to read.
 *
 * returns 1 if succeeded to read the "ready" message
 */
static int esp8266_is_ready_received()
{
	int ready_received = 0;
	char rcv_buf[10];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	while(!ready_received && vGalileo_UART0_is_data_available() )
	{
		if(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) != 0)
		{
			// test if we received ready
			if(esp8266_response_contains(rcv_buf, "ready"))
			{
				DEBUG(TRACE, "ESP8266 ready [OK]\r\n");
				ready_received = 1;
			}
		}
	}

	if(!ready_received)
	{
		DEBUG(TRACE, "ESP8266 ready [Failed]\r\n");
	}

	return ready_received;
}

/*
 * restart esp8266 (try soft restart then hard restart if soft restart is not working)
 * wait for "ready" message sent by esp8266
 *
 * returns 1 if succeeded to restart the esp8266 (ready message received)
 */
static int esp8266_restart()
{
        const TickType_t xDelay_2_sec = 2;
	int ret = 0;

	// flush all received data
	printf("flush all received data\r\n");
	vGalileo_UART0_flush_DMA_rcv_buffer();

	// try soft restart

  printf("try soft restart\r\n");
	esp8266_soft_restart();

	//wait ...

	vTaskDelay( xDelay_2_sec );

  printf("check if ready message received\r\n");
	// check if ready message received
	if(esp8266_is_ready_received()){
		DEBUG(TRACE, "ESP8266 soft restart [OK]\r\n");
		ret = 1;
	} else {
		DEBUG(TRACE, "ESP8266 soft restart [Failed]\r\n");
		// if soft restart not working try hard restart
		esp8266_hard_reset();

		//wait ...
		vTaskDelay( xDelay_2_sec );

		// check if ready message received
		if(esp8266_is_ready_received()){
			DEBUG(TRACE, "ESP8266 hard restart [OK]\r\n");
			ret = 1;
		} else {
			DEBUG(TRACE, "ESP8266 hard restart [Failed]\r\n");
		}
	}

	return ret;
}

/*
 * Wait for "WIFI CONNECTED" message from esp8266.
 * If receives "WIFI DISCONNECTED" message, it continues to wait for "WIFI CONNECTED" message
 *
 * Never returns if nothing received
 *
 * returns 1 if succeeded and returns 0 if not meaningful message received
 */
static int esp8266_wait_for_wifi_connection()
{
	int wifi_connected = 0;
	int unknown_msg_received = 0;
	char rcv_buf[20];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	while(!wifi_connected && !unknown_msg_received )
	{
		if(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) != 0)
		{
			// test if we received "WIFI CONNECTED"
			if(esp8266_response_contains(rcv_buf, "WIFI CONNECTED"))
			{
				DEBUG(TRACE, "ESP8266 WIFI CONNECTED [OK]\r\n");
				wifi_connected = 1;
			} else if(esp8266_response_contains(rcv_buf, "WIFI DISCONNECT")) // test if we received "WIFI DISCONNECT"
			{
				DEBUG(TRACE, "ESP8266 WIFI DISCONNECT [OK]\r\n");
			} else
			{
				DEBUG(TRACE, "ESP8266 unknown message ...\r\n");
				unknown_msg_received = 1;
			}
		}
	}

  printf("Finished wait for wifi connection\r\n");

	return wifi_connected;
}

/*
 * Wait for "WIFI GOT IP" message from esp8266.
 *
 * Never returns if nothing received
 *
 * returns 1 if succeeded and returns 0 if not meaningful message received
 */
static int esp8266_wait_for_ip_addr_attribution()
{
	int wifi_got_ip = 0;
	int unknown_msg_received = 0;
	char rcv_buf[20];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	while(!wifi_got_ip && !unknown_msg_received )
	{
		if(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) != 0)
		{
			// test if we received "WIFI GOT IP"
			if(esp8266_response_contains(rcv_buf, "WIFI GOT IP"))
			{
				DEBUG(TRACE, "ESP8266 WIFI GOT IP [OK]\r\n");
				wifi_got_ip = 1;
			} else
			{
				DEBUG(TRACE, "ESP8266 unknown message ...\r\n");
				unknown_msg_received = 1;
			}
		}
	}

	return wifi_got_ip;
}

/*
 * returns 1 if succeeded to disable echoing
 */
static int esp8266_disable_echoing()
{
	int ret = 0;
	char rcv_buf[10];
	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// send ATE0
	vGalileo_UART0_write((const char *)ESP8266_AT_DISABLE_ECHOING, strlen((const char *)ESP8266_AT_DISABLE_ECHOING));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive ATE0 unless echoing is already disabled
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	// test if we received ATE0
	else if(esp8266_response_contains(rcv_buf, "ATE0"))
	{
		// mettre le buffer � zero
		memset(rcv_buf, 0, sizeof(rcv_buf));

		// should receive OK this time !!!
		while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
		{
			;
		}
		// test if we received OK
		if(esp8266_response_contains(rcv_buf, "OK"))
		{
			DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
			ret = 1;
		}
		else
		{
			DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
		}
	}
	// no valid esp8266 return
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_NO_VALID_VALUE);
	}

	return ret;
}

/*
 * returns 1 if succeeded the serial communication test
 */
static int esp8266_test_serial_comm()
{
	int ret = 0;
	char rcv_buf[2];
	const TickType_t xDelay_1_ms = 1;
	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// send AT
	vGalileo_UART0_write((const char *)ESP8266_AT_TEST_COMM, strlen((const char *)ESP8266_AT_TEST_COMM));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * returns 1 if succeeded to disable transparent transmission
 */
static int esp8266_disable_transparent_transmission()
{
	int ret = 0;
	char rcv_buf[2];
	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// disable transparent transmission (mandatory to activate multiple connections)
	vGalileo_UART0_write((const char *)ESP8266_AT_DISABLE_TRANSPARENT_TRANS, strlen((const char *)ESP8266_AT_DISABLE_TRANSPARENT_TRANS));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * returns 1 if succeeded to enable multiple connections
 */
static int esp8266_enable_multiple_connections()
{
	int ret = 0;
	char rcv_buf[2];
	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// enable multiple connections
	vGalileo_UART0_write((const char *)ESP8266_AT_ENABLE_MULTI_CONN, strlen((const char *)ESP8266_AT_ENABLE_MULTI_CONN));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * returns 1 if succeeded to sets the maximum connections allowed by server to one
 */
static int esp8266_set_maximum_connections_to_one()
{
	int ret = 0;
	char rcv_buf[2];
	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// sets the maximum connections allowed by server to one
	vGalileo_UART0_write((const char *)ESP8266_AT_SET_MAX_CONN_TO_ONE, strlen((const char *)ESP8266_AT_SET_MAX_CONN_TO_ONE));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * returns 1 if succeeded to create TCP server at port 8080
 */
static int esp8266_create_tcp_server_port_8080()
{
	int ret = 0;
	char rcv_buf[20];

	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// create TCP server at port 8080
	vGalileo_UART0_write((const char *)ESP8266_AT_CREATE_TCP_SRV_PORT_8080, strlen((const char *)ESP8266_AT_CREATE_TCP_SRV_PORT_8080));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK unless tcp server is already activated
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	// test if we received no change
	else if(esp8266_response_contains(rcv_buf, "no change"))
	{
		// mettre le buffer � zero
		memset(rcv_buf, 0, sizeof(rcv_buf));

		// should receive OK this time !!!
		while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
		{
			;
		}
		// test if we received OK
		if(esp8266_response_contains(rcv_buf, "OK"))
		{
			DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
			ret = 1;
		}
		else
		{
			DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
		}
	}
	// no valid esp8266 return
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_NO_VALID_VALUE);
	}

	return ret;
}

/*
 * returns 1 if succeeded to sets the TCP server Timeout to 180 seconds
 */
static int esp8266_set_tcp_srv_timeout(unsigned int timeout)
{
	int ret = 0;
	char rcv_buf[2];
	const TickType_t xDelay_1_ms = 1;


	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	switch(timeout)
	{
	case 0:
		// sets the TCP server Timeout to 180 seconds
		vGalileo_UART0_write((const char *)ESP8266_AT_SET_TCP_SRV_TIMEOUT_NEVER, strlen((const char *)ESP8266_AT_SET_TCP_SRV_TIMEOUT_NEVER));
		break;
	default:
	case 180:
		// sets the TCP server Timeout to 180 seconds
		vGalileo_UART0_write((const char *)ESP8266_AT_SET_TCP_SRV_TIMEOUT_180, strlen((const char *)ESP8266_AT_SET_TCP_SRV_TIMEOUT_180));
		break;
	}
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * returns 1 if succeeded to hide the remote IP and Port with +IPD
 */
static int esp8266_hide_remote_IP_port_with_IPD()
{
	int ret = 0;
	char rcv_buf[2];

	const TickType_t xDelay_1_ms = 1;

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));

	// Hide the remote IP and Port with +IPD
	vGalileo_UART0_write((const char *)ESP8266_AT_HIDE_REMOTE_IP_PORT, strlen((const char *)ESP8266_AT_HIDE_REMOTE_IP_PORT));
	vTaskDelay_ms( xDelay_1_ms );

	// should receive OK
	while(vGalileo_UART0_read_line(rcv_buf, sizeof(rcv_buf)) == 0 && vGalileo_UART0_is_data_available())
	{
		;
	}
	// test if we received OK
	if(esp8266_response_contains(rcv_buf, "OK"))
	{
		DEBUG(TRACE, ESP8266_DEBUG_SUCCESS_OK_RECEIVED);
		ret = 1;
	}
	else
	{
		DEBUG(TRACE, ESP8266_DEBUG_ERROR_OK_MISSING);
	}

	return ret;
}

/*
 * Check if "+IPD," is sent from esp8266 over serial communication
 * This function consumes the comma ',' after "+IPD"
 *
 * @returns 1 if "+IPD," is received
 */
int esp8266_check_IPD_reception(char *rcv_buf)
{
	int ret = 0;

	// add '\0' at the end
	rcv_buf[5] = '\0';

	// test if we received "+IPD,"
	if(esp8266_response_contains(rcv_buf, "+IPD,"))
	{
		DEBUG(TRACE, "+IPD, reception [OK]\r\n");
		ret = 1;
	}

	return ret;
}

/*
 * Get the link number from the tcp header (after "+IPD")
 * This functions consumes the comma ',' after the link number
 *
 * @returns the link number in the range of [0-4] or '\0' if an error occurred
 */
static char esp8266_get_link_number()
{
	char link_number = '\0';
	char rcv_buf[2];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));
	// read 2 characters "<link_number>,"
	int read_size = vGalileo_UART0_read(rcv_buf, sizeof(rcv_buf));

	if(read_size > 0)
	{
		// test if the second character is a comma ',' and the first character is >= '0' and <= '4'
		if((rcv_buf[1] == ',') && (rcv_buf[0] >= '0') && (rcv_buf[0] <= '4') )
		{
			DEBUG(TRACE, "link number, reception [OK]\r\n");
			link_number = rcv_buf[0];
		}
		else
		{
			DEBUG(TRACE, "link number, reception [failed]\r\n");
		}
	}

	return link_number;
}

/*
 * Get the payload length of the tcp segment payload
 * this function read from the serial communication
 *
 * @returns the payload length
 */
static int esp8266_get_tcp_segment_payload_length()
{
	int payload_length = 0;
	char rcv_buf[5];

	// mettre le buffer � zero
	memset(rcv_buf, 0, sizeof(rcv_buf));
	// read 4 characters maximum
	unsigned int i = 0;
	do
	{
		// read only 1 character
		while(vGalileo_UART0_read(&rcv_buf[i], 1) == 0 && vGalileo_UART0_is_data_available())
		{
			;
		}
		i++;
	} while(i < (sizeof(rcv_buf) - 1) && rcv_buf[i-1] != ':');

	// check if we already read ':'
	if(rcv_buf[i-1] != ':')
	{
		// read only 1 character
		while(vGalileo_UART0_read(&rcv_buf[i], 1) == 0 && vGalileo_UART0_is_data_available())
		{
			;
		}
		i++;
	}

	// check if the last character is equal to ':'
	if(rcv_buf[i-1] == ':')
	{
		DEBUG(TRACE, " ':' after IPD reception [OK]\r\n");
		// replace ':' with '\0'
		rcv_buf[i-1] = '\0';
		// transform string to integer
		payload_length = atoi(rcv_buf);
	}
	else
	{
		DEBUG(TRACE, " ':' after IPD reception [failed]\r\n");
	}

	return payload_length;
}

/*
 * Attempts to read TCP header
 *
 * @param header point to an array of 12 bytes minimum in order to hold the header
 * @return the payload size of tcp segment
 */
unsigned int esp8266_get_tcp_header(char *header)
{
	/* Remove compiler warning about unused parameter. */
	(void)header;

	unsigned int payload_size = 0;

	// get link number
	char link_number = '\0';
	link_number = esp8266_get_link_number();
	if(link_number == '\0')
	{
		return payload_size;
	}
	DEBUG(TRACE, "link number is : %c\r\n", link_number);

	// get tcp segment payload length
	payload_size = esp8266_get_tcp_segment_payload_length();

	printf("tcp segment payload length : %d\r\n", 10);
	printf("tcp segment payload length : %d\r\n", payload_size);

	return payload_size;
}

// TODO manage fifo overflow case
/*
 * attempts to get tcp payload of the received tcp segment from the ESP8266 module
 *
 * @param (out) payload point to an array in order to hold the payload
 * @param (in) payload size
 * @return payload size
 */
int esp8266_get_tcp_payload(unsigned int payload_size)
{
	unsigned int remaining_bytes = payload_size;
	uint32_t rcv_buf_size = 4096;
	char *rcv_buf = pvPortMalloc(rcv_buf_size);


	if(rcv_buf == NULL)
	{
		printf("AllocPage failed !!!\r\n");
		return 0;
	}

	//char rcv_buf[ESP8266_RCV_PAYLOAD_MAX_SIZE];

	const TickType_t xDelay_1_ms = 1 ;

	printf("hello from get_tcp_payload 0\r\n");

	if(remaining_bytes > rcv_buf_size)
	{
		remaining_bytes = 0;
		DEBUG(CRITICAL, "tcp received segment payload is bigger than ESP8266_RCV_PAYLOAD_MAX_SIZE\r\n");
	}

	printf("hello from get_tcp_payload 1\r\n");

	while(remaining_bytes > 0)
	{
		printf("hello from get_tcp_payload 2\r\n");
		int read_size = vGalileo_UART0_read(rcv_buf, remaining_bytes);
		printf("hello from get_tcp_payload 3\r\n");
		if(read_size > 0)
		{
			// push the received bytes in the receive fifo
			printf("push the received bytes in the receive fifo\r\n");

			while(!fifo_push(&tcp_rcv_fifo, rcv_buf, read_size))
			{
				DEBUG(CRITICAL, "tcp rcv FIFO overflow\r\n");
				vTaskDelay_ms(xDelay_1_ms);
			}

			// update remaining bytes to get
			remaining_bytes -= read_size;
		}
	}

	vPortFree(rcv_buf);

	printf("just before return from get_tcp_payload\r\n");

	return payload_size;
}

/*
 * sends the payload
 *
 * returns	1 if succeeded
 */
static int esp8266_send_tcp_payload(char *payload, unsigned int payload_size)
{
	int ret = 0;

	const TickType_t xDelay_1_ms = 1;

	// test if payload_size is less than the send max size and bigger than 0
	if( (payload_size < 1) || (payload_size > ESP8266_SEND_BUFFER_MAX_SIZE) )
	{
		return ret;
	}

	// send the payload
	vGalileo_UART0_write(payload, payload_size);
	vTaskDelay_ms( xDelay_1_ms );

	ret = 1;

	return ret;
}

uint32_t try_to_send_tcp_header()
{
	uint32_t size = fifo_get_length(&tcp_send_fifo);

	if(size > ESP8266_SEND_BUFFER_MAX_SIZE)
	{
		size = ESP8266_SEND_BUFFER_MAX_SIZE;
	}

	if(!esp8266_send_tcp_header(size))
	{
		size = 0;
	}

	return size;
}

void send_tcp_payload(uint32_t size)
{
	char data[ESP8266_SEND_BUFFER_MAX_SIZE];
	uint32_t available_size = 0;

	available_size = fifo_peek(&tcp_send_fifo, data, size);

	if(available_size != size)
	{
		DEBUG(CRITICAL, "tcp send fifo length error\r\n");
	}

	esp8266_send_tcp_payload(data, size);
}

void remove_tcp_payload_from_send_fifo(uint32_t size)
{
	char data[ESP8266_SEND_BUFFER_MAX_SIZE];
	uint32_t available_size = 0;

	available_size = fifo_pull(&tcp_send_fifo, data, size);

	if(available_size != size)
	{
		DEBUG(CRITICAL, "tcp send fifo length error\r\n");
	}
}

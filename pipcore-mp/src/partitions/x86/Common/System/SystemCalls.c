/*
 * SystemCalls.c
 *
 *  Created on: 24 juil. 2018
 *      Author: odsi
 */

/*
 * Standard includes
 */
#include "stdint.h"
#include "stddef.h"

/*
 * PIP includes
 */
#include "queueGlue.h"
#include <pip/compat.h>
#include <pip/paging.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "domains.h"

#define PAGE_SIZE	4096

uint32_t queue_receive_adaptor( void * queue, void * const buffer, uint32_t ticksToWait )
{
	return xProtectedQueueReceive((uint32_t) queue, (uint32_t) buffer, ticksToWait);
}

uint32_t queue_send_adaptor( void * queue, const void * const buffer, uint32_t ticksToWait )
{
	return xProtectedQueueSend((uint32_t) queue, (uint32_t) buffer, ticksToWait);
}

void * malloc_for_queues_adaptor(uint32_t size)
{
	if(size > PAGE_SIZE)
	{
		return NULL;
	}

	return allocPage();
}

void mfree_for_queues_adaptor(void * mem)
{
	freePage(mem);
}

void * alloc_memory_adaptor(uint32_t size)
{
	return pvPortMalloc(size);
}

void free_memory_adaptor(void * mem)
{
	vPortFree(mem);
}


void delay_s_adaptor(uint32_t secondsToDelay)
{
	vTaskDelay(secondsToDelay);
}

void delay_ms_adaptor(uint32_t milliSecondsToDelay)
{
	vTaskDelay_ms(milliSecondsToDelay);
}

uint32_t create_led_blinker_task_adaptor(void * led_blinker_handler, int led_number, void * const taskReference)
{
	return xTaskCreate(led_blinker_handler, "Led blink task", configMINIMAL_STACK_SIZE * 16, (void *) led_number, LED_BLINK_TASK_PRIORITY, taskReference);
}

void task_delete_adaptor(void * taskToDelete)
{
	vTaskDelete(taskToDelete);
}

void task_self_delete_adaptor()
{
	vTaskDelete(NULL);
}





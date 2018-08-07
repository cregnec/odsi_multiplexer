/*
 * UART_DMA.c
 *
 *  Created on: 10 avr. 2018
 *      Author: swhx7055
 */


#include <Quark_x1000_support.h>
#include "UART_DMA.h"
#include "task.h"
#include "string.h"

#define DMA_BUFFER_SIZE		4095

static char dma_buffer[DMA_BUFFER_SIZE];

static uint32_t bGalileo_UART0_DMAC_Initialized = FALSE;
//static uint32_t bGalileo_UART1_DMAC_Initialized = FALSE;

static uint32_t DMA_UART_0_MMIO_Base = 0UL;
static uint32_t DMA_UART_1_MMIO_Base = 0UL;

static void vInitializeGalileoUART_RCVR_DMA(uint32_t portnumber);

static uint32_t buffer_index = 0;

/*
 * Use DMA
 * returns 1 if their is data available to be read
 */
int vGalileo_UART0_is_data_available()
{
	volatile uint32_t dest_addr = 0;
	int ret = 0;

	dest_addr = mem_read(DMA_UART_0_MMIO_Base, R_DMA_DAR0, 4);

	// checks if the dest_addr changed
	if((dest_addr - (uint32_t)dma_buffer) != buffer_index)
	{
		ret = 1;
	}

	return ret;
}

/*
 * ignore all incoming buffered messages
 */
void vGalileo_UART0_flush_DMA_rcv_buffer()
{
	volatile uint32_t dest_addr = 0;

	dest_addr = mem_read(DMA_UART_0_MMIO_Base, R_DMA_DAR0, 4);

	// checks if the dest_addr changed
	if((dest_addr - (uint32_t)dma_buffer) != buffer_index)
	{
		buffer_index = dest_addr - (uint32_t)dma_buffer;
	}
}

char uart0_dma_buffer_read_8()
{
	volatile uint32_t dest_addr = 0;
	char data_8 = 0;

	/* Block for 1ms. */
	const TickType_t xDelay = 1 / portTICK_PERIOD_MS;

	dest_addr = mem_read(DMA_UART_0_MMIO_Base, R_DMA_DAR0, 4);

	// checks if the dest_addr changed
	while((dest_addr - (uint32_t)dma_buffer) == buffer_index)
	{
		dest_addr = mem_read(DMA_UART_0_MMIO_Base, R_DMA_DAR0, 4);
	}

	// there is something to read
	vTaskDelay( xDelay );
	data_8 = dma_buffer[buffer_index];
	buffer_index++;
	if(buffer_index >= sizeof(dma_buffer))
	{
		buffer_index = 0;
	}


	return data_8;
}
/*
char uart0_dma_buffer_read_8()
{
	char data_8 = 0;

	data_8 = dma_buffer[buffer_index];
	while(data_8 == 0)
	{
		data_8 = dma_buffer[buffer_index];
	}

	dma_buffer[buffer_index] = 0;
	buffer_index++;
	if(buffer_index >= sizeof(dma_buffer))
	{
		buffer_index = 0;
	}

	return data_8;
}
*/


void vInitializeGalileo_client_SerialPort_RCVR_DMA()
{
 	if( bGalileo_UART0_DMAC_Initialized == FALSE )
	{
		/* Initialise for 115200, 8, 1, none and no handshaking */
 		vInitializeGalileoUART_RCVR_DMA(CLIENT_SERIAL_PORT);
 		bGalileo_UART0_DMAC_Initialized = TRUE;
	}
}

void client_SerialPort_RCVR_DMA_channel_enable()
{
	volatile uint32_t input_data = 0;
	input_data = mem_read(DMA_UART_0_MMIO_Base, R_DMA_CH_EN_REG, 4);
	input_data &= ~MASK_DMA_CH_EN_REG;
	input_data |= R_DMA_CH_EN_REG_CH_EN;
	mem_write(DMA_UART_0_MMIO_Base, R_DMA_CH_EN_REG, 4, input_data);
}

void client_SerialPort_RCVR_DMA_enable()
{
	 mem_write(DMA_UART_0_MMIO_Base, R_DMA_CFG_REG, 1, (uint8_t)(B_DMA_CFG_REG_DMA_EN));
}

void client_SerialPort_RCVR_DMA_disable()
{
	 mem_write(DMA_UART_0_MMIO_Base, R_DMA_CFG_REG, 1, (uint8_t)(~B_DMA_CFG_REG_DMA_EN));
}

static void vInitializeGalileoUART_RCVR_DMA(uint32_t portnumber)
 {
	uint32_t bar1;
	uint32_t UART_0_PCI_Base = 0UL;
	uint32_t UART_1_PCI_Base = 0UL;
	volatile uint32_t input_data = 0;

	if (portnumber == DEBUG_SERIAL_PORT)
	{
		UART_1_PCI_Base = MMIO_PCI_ADDRESS(0, 20, 5, 0);
		bar1 = mem_read(UART_1_PCI_Base, 0x14, 4);
		DMA_UART_1_MMIO_Base = bar1;
	}
	else
	{
		UART_0_PCI_Base = MMIO_PCI_ADDRESS(0, 20, 1, 0);
		bar1 = mem_read(UART_0_PCI_Base, 0x14, 4);
		DMA_UART_0_MMIO_Base = bar1;
	}


	memset(dma_buffer, 0, 4096);

	// Disable channel
	input_data = mem_read(bar1, R_DMA_CH_EN_REG, 4);
	input_data &= ~MASK_DMA_CH_EN_REG;
	//input_data |= R_DMA_CH_EN_REG_CH_EN;
	input_data |= R_DMA_CH_EN_REG_CH_EN_WE;
	mem_write(bar1, R_DMA_CH_EN_REG, 4, input_data);


	//mem_write(bar1, R_DMA_CFG_REG, 1, (uint8_t)(~B_DMA_CFG_REG_DMA_EN));
	mem_write(bar1, R_DMA_CFG_REG, 1, (uint8_t)B_DMA_CFG_REG_DMA_EN);


	//src_dma_test = 'H';
	mem_write(bar1, R_DMA_SAR0, 4, B_DMA_SAR0_SAR_RBR); // &src_dma_test);0

	mem_write(bar1, R_DMA_DAR0, 4, dma_buffer);

	input_data = mem_read(bar1, R_DMA_CTL0_L, 4);
	input_data &= ~MASK_DMA_CTL0_L;
	input_data |= B_DMA_CTL0_L_INT_EN;
	input_data |= B_DMA_CTL0_L_DST_TR_WIDTH_8;
	input_data |= B_DMA_CTL0_L_SRC_TR_WIDTH_8;
	input_data |= B_DMA_CTL0_L_DINC_INC;
	input_data |= B_DMA_CTL0_L_SINC_NOCHANGE;
	input_data |= B_DMA_CTL0_L_DEST_MSIZE_1;
	input_data |= B_DMA_CTL0_L_SRC_MSIZE_1;
	input_data |= B_DMA_CTL0_L_TT_FC;
	mem_write(bar1, R_DMA_CTL0_L, 4, input_data);

	input_data = mem_read(bar1, R_DMA_CTL0_U, 4);
	input_data &= ~MASK_DMA_CTL0_U;
	input_data |= B_DMA_CTL0_U_BLOCK_TS;
	mem_write(bar1, R_DMA_CTL0_U, 4, input_data);

	input_data = mem_read(bar1, R_DMA_CFG0_L, 4);
	input_data &= ~MASK_DMA_CFG0_L;
	input_data |= B_DMA_CFG0_L_CH_PRIOR;
	input_data |= B_DMA_CFG0_L_HS_SEL_SRC;
	input_data |= B_DMA_CFG0_L_DST_HS_POL_LOW;
	input_data |= B_DMA_CFG0_L_SRC_HS_POL_LOW;
	input_data |= B_DMA_CFG0_L_RELOAD_SRC;
	input_data |= B_DMA_CFG0_L_RELOAD_DST;
	mem_write(bar1, R_DMA_CFG0_L, 4, input_data);

	input_data = mem_read(bar1, R_DMA_CFG0_U, 4);
	input_data &= ~MASK_DMA_CFG0_U;
	input_data |= B_DMA_CFG0_U_SRC_PER;
	input_data |= B_DMA_CFG0_U_DEST_PER;
	mem_write(bar1, R_DMA_CFG0_U, 4, input_data);

	//mem_write(bar1, R_DMA_CFG_REG, 1, (uint8_t)B_DMA_CFG_REG_DMA_EN);

	input_data = mem_read(bar1, R_DMA_CH_EN_REG, 4);
	input_data &= ~MASK_DMA_CH_EN_REG;
	input_data |= R_DMA_CH_EN_REG_CH_EN;
	input_data |= R_DMA_CH_EN_REG_CH_EN_WE;
	mem_write(bar1, R_DMA_CH_EN_REG, 4, input_data);
 }

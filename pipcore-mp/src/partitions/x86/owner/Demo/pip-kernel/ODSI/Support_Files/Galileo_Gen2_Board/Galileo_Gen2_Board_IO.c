/*
 * Galileo_Gen2_Board_IO.c
 *
 *  Created on: 26 avr. 2018
 *      Author: swhx7055
 */

#include "Galileo_Gen2_Board.h"
#include "Expander_Pcal9555_Driver.h"
#include "GPIO_I2C.h"
#include "pip/debug.h"
#include "FreeRTOS.h"
#include "task.h"

/*
 * Initialize IOs of the Galileo Board and GPIOs of the Quark x1000 SoC
 */
void Galileo_Gen2_Init_IO7_and_IO8()
{
	// Disable Pull-ups / pull downs on EXP1 pin for IO7_PU signal.
	PlatformPcal9555GpioPullup (
	GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
	7,                                   // P0_7.
	FALSE
	);

	// Disable Pull-ups / pull downs on EXP1 pin for IO8_PU signal.
	PlatformPcal9555GpioPullup (
	GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
	9,                                   // P1_1.
	FALSE
	);

	// set IO7 and IO8 to low
	Galileo_Gen2_Set_IO_Level(GALILEO_GEN2_IO_7, LOW);
	Galileo_Gen2_Set_IO_Level(GALILEO_GEN2_IO_8, LOW);

	// set IO7 and IO8 as inputs
	Galileo_Gen2_Set_IO_Direction(GALILEO_GEN2_IO_7, GPIO_INPUT);
	Galileo_Gen2_Set_IO_Direction(GALILEO_GEN2_IO_8, GPIO_INPUT);
}

/*
 * set GPIO direction mapped to Galileo Gen2 Board IO
 *
 * @param Galileo_Board_IO_Number: 0 ~ 13
 * @param Direction: 1 for input and 0 for output
 */
void Galileo_Gen2_Set_IO_Direction(uint32_t Galileo_Board_IO_Number, uint32_t Direction)
{
	switch(Galileo_Board_IO_Number)
	{
	case GALILEO_GEN2_IO_0:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_3, Direction);
		// Set level of LVL_C_OE0_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(0), Direction); // P0-0.

		break;

	case GALILEO_GEN2_IO_1:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_4, Direction);
		// Set level of LVL_B_OE6_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT1_PIN(4), Direction); // P1-4.

		break;

	case GALILEO_GEN2_IO_2:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_5, Direction);
		// Set level of LVL_C_OE1_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(2), Direction); // P0-2.

		break;

	case GALILEO_GEN2_IO_3:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_6, Direction);
		// Set level of LVL_B_OE0_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(0), Direction); // P0-0.

		break;

	case GALILEO_GEN2_IO_4:
		Quark_x1000_Set_GPIO_SUS_BitDirection(QUARK_X1000_GPIO_SUS_4, Direction);
		// Set level of LVL_C_OE2_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(4), Direction); // P0-4.

		break;

	case GALILEO_GEN2_IO_5:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_8, Direction);
		// Set level of LVL_B_OE1_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(2), Direction); // P0-2.

		break;

	case GALILEO_GEN2_IO_6:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_9, Direction);
		// Set level of LVL_B_OE2_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(4), Direction); // P0-4.

		break;

	case GALILEO_GEN2_IO_7:
		PlatformPcal9555GpioSetDir (
		GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
		6,                                   // P0_6.
		Direction);
		break;

	case GALILEO_GEN2_IO_8:
		PlatformPcal9555GpioSetDir (
		GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
		8,                                   // P1_0.
		Direction);
		break;

	case GALILEO_GEN2_IO_9:
		Quark_x1000_Set_GPIO_SUS_BitDirection(QUARK_X1000_GPIO_SUS_2, Direction);
		// Set level of LVL_B_OE3_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT0_PIN(6), Direction); // P0-6.

		break;

	case GALILEO_GEN2_IO_10:
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_2, Direction);
		// Set level of LVL_B_OE5_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT1_PIN(2), Direction); // P1-2.

		break;

	case GALILEO_GEN2_IO_11:
		Quark_x1000_Set_GPIO_SUS_BitDirection(QUARK_X1000_GPIO_SUS_3, Direction);
		// Set level of LVL_B_OE4_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT1_PIN(0), Direction); // P1-0.

		break;

	case GALILEO_GEN2_IO_12:

		// Set level of LVL_C_OE5_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR, EXP_PORT1_PIN(2), Direction); // P1-2.
		Quark_x1000_Set_GPIO_BitDirection(QUARK_X1000_GPIO_7, Direction);
		break;

	case GALILEO_GEN2_IO_13:
		Quark_x1000_Set_GPIO_SUS_BitDirection(QUARK_X1000_GPIO_SUS_5, Direction);
		// Set level of LVL_B_OE7_N.
		PlatformPcal9555GpioSetLevel (GALILEO_GEN2_IOEXP0_7BIT_SLAVE_ADDR, EXP_PORT1_PIN(6), Direction); // P1-6.

		break;

	default:
		DEBUG(TRACE, "[Galileo Gen2 Board] Unknown IO number\r\n");
	}
}

 /*
 * set GPIO level mapped to Galileo Gen2 Board IO
 *
 * @param GPIONumber: 0 ~ 13
 * @param Galileo_Board_IO_Number: 0 for low and 1 for high
 */
 void Galileo_Gen2_Set_IO_Level(uint32_t Galileo_Board_IO_Number, uint32_t Level)
 {
	 switch(Galileo_Board_IO_Number)
	 {
	 case 0:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_3, Level);
		 break;
	 case 1:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_4, Level);
		 break;
	 case 2:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_5, Level);
		 break;
	 case 3:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_6, Level);
		 break;
	 case 4:
		 Quark_x1000_Set_GPIO_SUS_BitLevel(QUARK_X1000_GPIO_SUS_4, Level);
		 break;
	 case 5:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_8, Level);
		 break;
	 case 6:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_9, Level);
		 break;
	 case 7:
		 PlatformPcal9555GpioSetLevel (
		 GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
		 6,                                   // P0_6.
		 Level);
		 break;
	 case 8:
		 PlatformPcal9555GpioSetLevel (
		 GALILEO_GEN2_IOEXP1_7BIT_SLAVE_ADDR,  // IO Expander 1.
		 8,                                   // P1_0.
		 Level);
		 break;
	 case 9:
		 Quark_x1000_Set_GPIO_SUS_BitLevel(QUARK_X1000_GPIO_SUS_2, Level);
		 break;
	 case 10:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_2, Level);
		 break;
	 case 11:
		 Quark_x1000_Set_GPIO_SUS_BitLevel(QUARK_X1000_GPIO_SUS_3, Level);
		 break;
	 case 12:
		 Quark_x1000_Set_GPIO_BitLevel(QUARK_X1000_GPIO_7, Level);
		 break;
	 case 13:
		 Quark_x1000_Set_GPIO_SUS_BitLevel(QUARK_X1000_GPIO_SUS_5, Level);
		 break;
	 default:
		 DEBUG(TRACE, "[Galileo Gen2 Board] Unknown IO number\r\n");
	 }
 }

 static void Test_Galileo_Gen2_Set_IOs_Out()
 {
	 for(int i = 3; i <= 13; i++)
	 {
		 Galileo_Gen2_Set_IO_Direction(i, GPIO_OUTPUT);
	 }
 }

 static void Test_Galileo_Gen2_Set_IOs_In()
 {
	 for(int i = 3; i <= 13; i++)
	 {
		 Galileo_Gen2_Set_IO_Direction(i, GPIO_INPUT);
	 }
 }

 void Test_Galileo_Gen2_Blink_IOs()
 {
	 //  Block for 500ms.
	 const TickType_t xDelay = 500 / portTICK_PERIOD_MS;

	Test_Galileo_Gen2_Set_IOs_Out();

	for(int j = 3; j <= 13; j++)
	{
		Galileo_Gen2_Set_IO_Level(j, HIGH);
		vTaskDelay( xDelay );
		Galileo_Gen2_Set_IO_Level(j, LOW);
		vTaskDelay( xDelay );
	}

	Test_Galileo_Gen2_Set_IOs_In();
 }

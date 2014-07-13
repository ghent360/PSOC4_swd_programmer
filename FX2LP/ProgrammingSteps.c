/******************************************************************************
* File Name: ProgrammingSteps.c
* Version 1.0
*
* Description:
*  This file provides the source code for the high level Programming functions 
*  used by the main code to program target PSoC 4
*
* Owner:
*	Tushar Rastogi, Application Engineer (tusr@cypress.com)
*
* Related Document:
*	AN84858 - PSoC 4 Programming using an External Microcontroller (HSSP)
*
* Hardware Dependency:
*   PSoC 5LP Development Kit - CY8CKIT-050
*
* Code Tested With:
*	PSoC Creator 2.2
*	ARM GCC 4.4.1
*	CY8CKIT-050
*
*******************************************************************************
* Copyright (2013), Cypress Semiconductor Corporation.
*******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation
* of this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
******************************************************************************/

#include "lp.h"
#include "lpregs.h"
#include "intrins.h"

/******************************************************************************
*   Header file Inclusion
******************************************************************************/
#include "ProgrammingSteps.h"
#include "SWD_PhysicalLayer.h"
#include "SWD_UpperPacketLayer.h"
#include "SWD_UpperLayer.h"
#include "SWD_PacketLayer.h"
#include "Timeout.h"

static void Delay100Us()
{
  unsigned char loop;
  for(loop=0;loop<200;loop++)
  {
    _nop_();
  }
}

/******************************************************************************
* Function Name: PollSromStatus
*******************************************************************************
* Summary:
*  Polls the SROM_SYSREQ_BIT and SROM_PRIVILEGED_BIT in the CPUSS_SYSREQ 
*  register till it is reset or a timeout condition occurred, whichever is 
*  earlier. For a SROM polling timeout error, the timeout error status bit is 
*  set in swd_PacketAck variable and CPUSS_SYSARG register is read to get the 
*  error status code. If timeout does not happen, the CPUSS_SYSARG register is 
*  read to determine if the task executed successfully.
*
* Parameters:
*  None.
*
* Return:
*  SUCCESS - SROM executed the task successfully
*  FAILURE - SROM task is not executed successfully and a timeout error occured.
*            The failure code is stored in the statusCode global variable.
*
* Note:
*  This function is called after non volatile memory operations like Read,  
*  Write of Flash, to check if SROM task has been executed which is indicated
*  by SUCCESS. The status is read from the CPUSS_SYSARG register.
*
******************************************************************************/
unsigned char PollSromStatus(void)
{
    unsigned short time_elapsed;
    
	time_elapsed = 0;
    do
    {
	    /* Read CPUSS_SYSREQ register and check if SROM_SYSREQ_BIT and 
		SROM_PRIVILEGED_BIT are reset to 0 */
		Read_IO(CPUSS_SYSREQ);

		//statusCode &= (SROM_SYSREQ_BIT | SROM_PRIVILEGED_BIT);
		//Mask_Data(SROM_SYSREQ_BIT | SROM_PRIVILEGED_BIT);
		swd_PacketData[3] &= (unsigned char)((SROM_SYSREQ_BIT | SROM_PRIVILEGED_BIT) >> 24);
		time_elapsed++;
    } while (swd_PacketData[3] != 0 && (time_elapsed <= SROM_POLLING_TIMEOUT));
	
	/* If time exceeds the timeout value, set the SROM_TIMEOUT_ERROR bit in 
	   swd_PacketAck */
    if (swd_PacketData[3] != 0)
    {
		swd_PacketAck = swd_PacketAck | SROM_TIMEOUT_ERROR;
		Read_IO(CPUSS_SYSARG);
		return 1;
    }
	
	/* Read CPUSS_SYSARG register to check if the SROM command executed 
	successfully else set SROM_TIMEOUT_ERROR in swd_PacketAck */
	Read_IO (CPUSS_SYSARG);
	//if ((statusCode & SROM_STATUS_SUCCESS_MASK) != SROM_STATUS_SUCCEEDED)
	if (Mask_CMPNE_Data(SROM_STATUS_SUCCESS_MASK, SROM_STATUS_SUCCEEDED))
	{
		swd_PacketAck = swd_PacketAck | SROM_TIMEOUT_ERROR;
		return 2;
    }
    return SROM_SUCCESS;
}

#if defined CY8C40xx_FAMILY
/******************************************************************************
* Function Name: SetIMO48MHz
*******************************************************************************
* Summary:
* Set IMO to 48 MHz 
*
* Parameters:
*  None.
*
* Return:
*  None
*
* Note:
*  This function is required to be called before any flash operation.
*  This function sets the IMO to 48 MHz before flash write/erase operations
*  and is part of the device acquire routine.
*
******************************************************************************/
void SetIMO48MHz(void)
{    
	/* Write the command to CPUSS_SYSARG register */
    Write_IO (CPUSS_SYSARG, SROM_KEY1 | ((unsigned short)(SROM_KEY2 | SROM_CMD_SET_IMO_48MHZ) << 8));
    Write_IO (CPUSS_SYSREQ, SROM_SYSREQ_BIT | SROM_CMD_SET_IMO_48MHZ);
}
#endif

/******************************************************************************
*Function Name: DeviceAcquire
*******************************************************************************
*
* Summary:
*  This is Step 1 of the programming sequence. In this Step, target PSoC 4 is 
*  acquired by the host microcontroller by sending specific Port Acquiring 
*  Sequence in a 1.5 ms time-window. After acquiring SWD port, debug port is 
*  configured and bit 31 in TEST_MODE control register is set.
*
* Parameters:
*  None
*
* Return:
*  SUCCESS - Returns SUCCESS if the device is successfully acquired.
*  FAILURE - Returns Failure if the function fails in any of the intermediate
*			 step.
*
* Note:
* This function has very strict timing requirements. The device must be
* acquired as per the timing requirements given in PSoC 4 Device Programming 
* Specification Document.
*
******************************************************************************/
unsigned char DeviceAcquire(void)
{
    unsigned char total_packet_count;
    unsigned char result;
	    
	/* Aquiring Sequence */

	XRES_OUTPUT_HIGH;
	XRES_OUTPUT_ENABLE;

	SWDCK_OUTPUT_LOW;
	SWDCK_OUTPUT_ENABLE;

	SWDIO_OUTPUT_LOW;
	SWDIO_OUTPUT_ENABLE;
	
	/* Set XRES of PSoC 4 low for 100us with SWDCK and SWDIO low (min delay 
	   required is 5us) */
	XRES_OUTPUT_LOW;
	Delay100Us();
	XRES_OUTPUT_HIGH;
	
	total_packet_count = 0;
    do
    {
		/* Call Swd_LineReset (Standard ARM command to reset DAP) and read
		   DAP_ID from chip */
    	Swd_LineReset();
		Read_DAP(DPACC_DP_IDCODE_READ);
		total_packet_count++;
    } while((swd_PacketAck != SWD_OK_ACK) && (total_packet_count < DEVICE_ACQUIRE_TIMEOUT));
	/* Set PORT_ACQUIRE_TIMEOUT_ERROR bit in swd_PacketAck if time
	   exceeds 1.5 ms */
	if (total_packet_count == DEVICE_ACQUIRE_TIMEOUT)
	{
		swd_PacketAck = swd_PacketAck | PORT_ACQUIRE_TIMEOUT_ERROR;
        return 1;
	}
	
	/* Set VERIFICATION_ERROR bit in swd_PacketAck if the DAP_ID read
	   from chip does not match with the ARM CM0_DAP_ID (MACRO defined in
	   ProgrammingSteps.h file - 0x0BB11477) */
	if (!CMP_Data_EQ(CM0_DAP_ID))
	{
		swd_PacketAck = swd_PacketAck | VERIFICATION_ERROR;
		return 2;
	}
	
	/* Initialize Debug Port */
	Write_DAP (DPACC_DP_CTRLSTAT_WRITE, 0x54000000);
	if( swd_PacketAck != SWD_OK_ACK )
    {
        return 3;
    }
	
	Write_DAP (DPACC_DP_SELECT_WRITE, 0x00000000);
	if( swd_PacketAck != SWD_OK_ACK )
    {
        return 4;
    }
	
	Write_DAP (DPACC_AP_CSW_WRITE, 0x00000002);
	if( swd_PacketAck != SWD_OK_ACK )
    {
        return 5;
    }

	/* Enter CPU into Test Mode */
    Write_IO (TEST_MODE, 0x80000000);
	if( swd_PacketAck != SWD_OK_ACK )
    {
        return 6;
    }

    Read_IO(TEST_MODE);
	if( swd_PacketAck != SWD_OK_ACK )
    {
        return 7;
    }
    if ((swd_PacketData[3] & 0x80) != 0x80) // ((status & 0x80000000) != 0x80000000)
    {
        return 8;
    }
	/* Read status of the operation */
	result = PollSromStatus();
	if (result != SROM_SUCCESS)
	{
		return 9;
	}

#if defined CY8C40xx_FAMILY	
	/* Set IMO to 48 MHz */
	SetIMO48MHz();
	
	/* Read status of the operation */
	result = PollSromStatus();
	if (result != SROM_SUCCESS)
	{
		return 10;
	}
#endif
	
    return SUCCESS;
}

/******************************************************************************
* Function Name: ExitProgrammingMode
*******************************************************************************
*
* Summary:
*  Releases the target PSoC 4 device from Programming mode.
*
******************************************************************************/
void ExitProgrammingMode(void)
{
    /* Drive the SWDIO, SWDCK outputs low */
	SWDCK_OUTPUT_LOW;
	SWDIO_OUTPUT_LOW;
    
    /* Make SWDIO, SWDCK High-Z after completing Programming */
	SWDIO_OUTPUT_DISABLE;
	SWDCK_OUTPUT_DISABLE;
    
    /* Generate active low rest pulse for 100 uS */
	XRES_OUTPUT_LOW;
    Delay100Us();
	XRES_OUTPUT_HIGH;

    /* Make XRES High-Z after generating the reset pulse */  
  	XRES_OUTPUT_DISABLE;
}

/* [] END OF FILE */

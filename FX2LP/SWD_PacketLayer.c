/******************************************************************************
* File Name: SWD_PacketLayer.c
* Version 1.0
*
* Description:
*  This file provides the source code for the packet layer functions of the 
*  SWD protocol. This includes SWD Read packet, SWD Write packet.
*
* Owner:
*	Tushar Rastogi, Application Engineer (tusr@cypress.com)
*
* Related Document:
*	AN84858 - PSoC 4 Programming using an External Microcontroller (HSSP)
*
* Hardware Dependency:
*   PSoC 4 Pioneer Kit (CY8CKIT-042)
*
* Code Tested With:
*	PSoC Creator 2.2
*	ARM GCC 4.4.1
*	CY8CKIT-050
*
* Note:
*  The functions in SWD packet layer use the bit banging macros, functions
*  in "SWD_PhysicalLayer.h"
*
******************************************************************************
* Copyright (2013), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
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

#include "SWD_PacketLayer.h"
#include "SWD_PhysicalLayer.h"

/******************************************************************************
*   Global Variable definitions
******************************************************************************/
/* Stores the 8-bit SWD Packet header */
unsigned char swd_PacketHeader;

/* Stores the 3-bit SWD Packet ACK data */
unsigned char swd_PacketAck;

/* 4-byte Read, Write packet data */
unsigned char swd_PacketData[DATA_BYTES_PER_PACKET];

extern void Swd_SendByte(unsigned char txByte);
extern unsigned char Swd_ReceiveByte();
extern void Swd_FirstTurnAroundPhase();
extern void Swd_SecondTurnAroundPhase();
extern unsigned char Swd_GetAckSegment();

static void Swd_SendParity(bit parity)
{
    SWDCK_OUTPUT_LOW;
	SWDIO_PIN = parity;
    SWDCK_OUTPUT_HIGH;
}

static bit Swd_ReceiveParity()
{
    bit parity;
    
    SWDCK_OUTPUT_LOW;
    parity = SWDIO_PIN;
    SWDCK_OUTPUT_HIGH;    
    
    return(parity);
}

static void Swd_SendDummyClocks()
{
    unsigned char loop;
    
    /* Send three SWDCK clock cycles with SWDIO line held low */   
    SWDIO_OUTPUT_LOW;
    for(loop = 0; loop < NUMBER_OF_DUMMY_SWD_CLOCK_CYCLES; loop++)
    {
       SWDCK_OUTPUT_LOW;
       SWDCK_OUTPUT_HIGH; 
    }
}

static unsigned char Swd_CountOneBits(unsigned char dataByte)
{
    unsigned char count = 0;
    unsigned char loop;
	for (loop=0;loop < 8; loop++)
    {
        count += dataByte & 1; 
        dataByte >>= 1;
    }
    
    return(count);
}

static bit Swd_ComputeDataParity()
{
    unsigned char count = 0;
    unsigned char i;
    
    /* Count number of 1's in 4-byte data */
    for(i = 0; i < DATA_BYTES_PER_PACKET; i++)
    {
        count += Swd_CountOneBits(swd_PacketData[i]);
    }
    
    /* Return even parity. If Lsb bit is 1, it implies the number of 1's is odd,
    and hence the even parity bit is set. For even number of 1's, parity bit
	is 0 */
    return (count & LSB_BIT_MASK);
}

/******************************************************************************
* Function Name: Swd_ReadPacket
*******************************************************************************
*
* Summary:
*  Sends a single SWD Read packet
*
* Parameters:
* swd_PacketHeader - Global variable that holds the 8-bit header data for packet
*
* Return:
*  swd_PacketData[DATA_BYTES_PER_PACKET] - Global variable that holds the 4-byte
*										   data that has been read
*
*  swd_PacketAck - Global variable that holds the status of the SWD transaction
*    ACK response is stored as a byte. Three possible ACK return values are:
*       0x01 - SWD_OK_ACK
*       0x02 - SWD_WAIT_ACK
*       0x04 - SWD_FAULT_ACK
*       0x09 - SWD_OK_ACK, but Parity error in data received
*       Any other ACK value - Undefined ACK code.
*                             Treat it similar to SWD_FAULT_ACK and abort
*							  operation.
*
* Note:
*  1.)Swd_RawReadPacket() is called during SPC Polling operations when the host
*	  must read the SPC status register continuously till a Programming operation
*	  is completed or data is ready to be read.
*
*  2.)This function is called duing continuous multi byte read operations from
*	  the SPC_DATA register
*  
*  3.)This function is called during IDCODE instruction for reading the Device
*	  JTAG ID
*
*  4.)This function is not called during normal SWD read transactions. To read
*     a register data, two SWD Read packets must be sent (call this function 
*     twice). This method of reading twice has been implemented in
*	  SWD_ReadPacket() function which will be called during normal read 
*	  transactions. 
*
******************************************************************************/
void Swd_ReadPacket()
{
    unsigned char loop = 0;
    bit parity;
    
    do
    {
        unsigned char i;
		/* 8-bit Header data */
        Swd_SendByte(swd_PacketHeader);        
        
		/* First Turnaround phase */
        Swd_FirstTurnAroundPhase(); 
        
		/* Get the 3-bit ACK data */
        swd_PacketAck = Swd_GetAckSegment();
        
        /* Read 4-byte data and store in Global array swd_PacketData[] */
        for(i = 0; i < DATA_BYTES_PER_PACKET; i++)
        {
            swd_PacketData[i] = Swd_ReceiveByte();
        }
        
		/* Parity phase */
        parity = Swd_ReceiveParity();        
        
		/* Second Turnaround phase */
        Swd_SecondTurnAroundPhase();
        
		/* Dummy clock phase since clock is not free running */
        Swd_SendDummyClocks();
        
        /* Repeat for a WAIT ACK till timeout loop expires */
        loop++;
    } while((swd_PacketAck == SWD_WAIT_ACK ) && (loop < NUMBER_OF_WAIT_ACK_LOOPS));
    
    /* For a OK ACK, check the parity bit received with parity computed */
    if (swd_PacketAck == SWD_OK_ACK)
    {
        if (parity != Swd_ComputeDataParity())
        {
			/* Set the Parity error bit in ACK code */
            swd_PacketAck = swd_PacketAck | SWD_PARITY_ERROR;
        }
    }
}


/******************************************************************************
* Function Name: Swd_WritePacket
*******************************************************************************
*
* Summary:
*  Sends a single SWD Write packet
*
* Parameters:
*  swd_PacketHeader - Global variable that holds the 8-bit header data for packet
*  swd_PacketData[DATA_BYTES_PER_PACKET] - Global variable that holds the data
*										   to be sent
*
* Return:
*  swd_PacketAck - Global variable that holds the status of the SWD transaction
*    ACK response is stored as a byte. Three possible ACK return values are:
*       0x01 - SWD_OK_ACK
*       0x02 - SWD_WAIT_ACK
*       0x04 - SWD_FAULT_ACK
*       Any other ACK value - Undefined ACK code.
*       Treat it similar to SWD_FAULT_ACK and abort operation.
*
* Note:
*  This function is called for Address Write, Data Write operations
*
******************************************************************************/
void Swd_WritePacket()
{
    bit parity;
    unsigned char loop = 0;
    unsigned char i;
    
	/* Compute Even parity for 4-byte data */
    parity = Swd_ComputeDataParity();  
    
    do
    {
		/* 8-bit Header data */
        Swd_SendByte(swd_PacketHeader); 
        
		/* First Turnaround phase */
        Swd_FirstTurnAroundPhase();
        
		/* Get the 3-bit ACK data */
        swd_PacketAck = Swd_GetAckSegment();
        
		/* Second Turnaround phase */
        Swd_SecondTurnAroundPhase();
        
        /* Send 4-byte data stored in Global array swd_PacketData[] */
        for(i = 0; i < DATA_BYTES_PER_PACKET; i++)
        {
            Swd_SendByte(swd_PacketData[i]); 
        }
        
		/* Parity phase */
        Swd_SendParity(parity);
        
		/* Dummy clock phase since clock is not free running */
        Swd_SendDummyClocks();
        
        loop++;
    } while((swd_PacketAck == SWD_WAIT_ACK ) && (loop < NUMBER_OF_WAIT_ACK_LOOPS));
    
    /* swd_PacketAck global variable holds the status of the SWD transaction */
}

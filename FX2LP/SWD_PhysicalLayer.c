/*******************************************************************************
* File Name: SWD_PhysicalLayer.c
* Version 1.0
*
* Description:
*  This file provides the source code for the physical layer functions of the
*  SWD protocol. These function definitions are the same as the corresponding
*  macro definitions in SWD_PhysicalLayer.h
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
*  The functions defined here are specific to PSoC 5LP Host Programmer.
*  Modify the definitions as applicable for your Host Programmer.
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

*******************************************************************************/

/********************************************************************************
*   Header file Inclusion
********************************************************************************/
#include "lp.h"
#include "lpregs.h"

#include "SWD_PhysicalLayer.h"

/********************************************************************************
*   Function Definitions
********************************************************************************/

/***************************** USER ATTENTION REQUIRED **************************
***************************** HOST PROCESSOR SPECIFIC ***************************
******************* SWD Protocol Physical Layer functions ***********************
*
* All of the function definitions below are for a PSoC 5LP host programmer.
* Modify the definitions as applicable for the specific host programmer.
********************************************************************************/

/*******************************************************************************
* Function Name: SetSwdckCmosOutput
********************************************************************************
*
* Summary:
*  Sets the SWDCK pin drive mode as CMOS drive mode.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDCK_DRIVEMODE_CMOSOUT in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdckCmosOutput()
{
    OED &= ~0x02;
}

/*******************************************************************************
* Function Name: SetSwdckHizInput
********************************************************************************
*
* Summary:
*  Sets the SWDCK pin drive mode as High impedance digital input.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDCK_DRIVEMODE_HIGHZIN in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdckHizInput()
{
    OED |= 0x02;
}

/*******************************************************************************
* Function Name: SetSwdioCmosOutput
********************************************************************************
*
* Summary:
*  Sets the SWDIO pin drive mode as CMOS drive mode.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDIO_DRIVEMODE_CMOSOUT in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdioCmosOutput()
{
    OED &= ~0x01;
}

/*******************************************************************************
* Function Name: SetSwdioHizInput
********************************************************************************
*
* Summary:
*  Sets the SWDIO pin drive mode as High impedance digital input.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDIO_DRIVEMODE_HIGHZIN in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdioHizInput()
{
    OED |= 0x01;
}

/*******************************************************************************
* Function Name: SetXresCmosOutput
********************************************************************************
*
* Summary:
*  Sets the XRES pin drive mode as CMOS drive mode.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition XRES_DRIVEMODE_CMOSOUT in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetXresCmosOutput()
{
    OEB &= ~0x04;
}

/*******************************************************************************
* Function Name: SetXresHizInput
********************************************************************************
*
* Summary:
*  Sets the XRES pin drive mode as High impedance digital input.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition XRES_DRIVEMODE_HIGHZIN in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetXresHizInput()
{
    OEB |= 0x04;
}

/*******************************************************************************
* Function Name: SetSwdckHigh
********************************************************************************
*
* Summary:
*  Sets the SWDCK pin high
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDCK_OUTPUT_HIGH in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdckHigh()
{
    SWDCK_PIN = 1;
}

/*******************************************************************************
* Function Name: SetSwdckLow
********************************************************************************
*
* Summary:
*  Sets the SWDCK pin low
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDCK_OUTPUT_LOW in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdckLow()
{
    SWDCK_PIN = 0;
}

/*******************************************************************************
* Function Name: SetSwdioHigh
********************************************************************************
*
* Summary:
*  Sets the SWDIO pin high
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDIO_OUTPUT_HIGH in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdioHigh()
{
    SWDIO_PIN = 1;
}

/*******************************************************************************
* Function Name: SetSwdioLow
********************************************************************************
*
* Summary:
*  Sets the SWDIO pin low
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition SWDIO_OUTPUT_LOW in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetSwdioLow()
{
    SWDIO_PIN = 0;
}

/*******************************************************************************
* Function Name: SetXresHigh
********************************************************************************
*
* Summary:
*  Sets the XRES pin high
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition XRES_OUTPUT_HIGH in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetXresHigh()
{
    XRES_PIN = 1;
}

/*******************************************************************************
* Function Name: SetXresLow
********************************************************************************
*
* Summary:
*  Sets the XRES pin low
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  The function definition is the same as the macro definition XRES_OUTPUT_LOW in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
void SetXresLow()
{
    XRES_PIN = 0;
}

/*******************************************************************************
* Function Name: ReadSwdio
********************************************************************************
*
* Summary:
*  Returns the state of the SWDIO input pin
*
* Parameters:
*  None.
*
* Return:
*  unsigned char (only one of the two values as given below)
*   0x00 - If the SWDIO pin state is low
*   0x01 - If the SWDIO pin state is high
*
* Note:
*  The function definition is the same as the macro definition SWDIO_INPUT_READ in
*  SWD_PhysicalLayer.h.
*
*******************************************************************************/
bit ReadSwdio()
{
    return SWDIO_PIN;
}

/* [] END OF FILE */

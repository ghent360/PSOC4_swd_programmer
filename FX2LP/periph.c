//-----------------------------------------------------------------------------
//   File:      periph.c
//   Contents:  Hooks required to implement USB peripheral function.
//
#pragma NOIV               // Do not generate interrupt vectors

#include "lp.h"
#include "lpregs.h"
#include "syncdly.h"
#include "SWD_PacketLayer.h"
#include "ProgrammingSteps.h"
#include "SWD_UpperPacketLayer.h"
#include "SWD_UpperLayer.h"

extern BOOL   GotSUD;         // Received setup data flag
extern BOOL   Sleep;
extern BOOL   Rwuen;
extern BOOL   Selfpwr;

BYTE   Configuration;      // Current configuration
BYTE   AlternateSetting;   // Alternate settings

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)
{
   BREAKPT &= ~bmBPEN;      
   Rwuen = TRUE;       
   CPUCS=((CPUCS & ~bmCLKSPD) | bmCLKSPD1);
   IFCONFIG |=0x40;
}

void TD_Poll(void)             // USB空闲的时候循环调用
{
}

BOOL TD_Suspend(void)          // Called before the device goes into suspend mode
{
   return(TRUE);
}

BOOL TD_Resume(void)          // Called after the device resumes
{
   return(TRUE);
}

//-----------------------------------------------------------------------------
// Device Request hooks
//   The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void)
{
   return(TRUE);
}

BOOL DR_SetConfiguration(void)   // Called when a Set Configuration command is received
{
   Configuration = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetConfiguration(void)   // Called when a Get Configuration command is received
{
   EP0BUF[0] = Configuration;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_SetInterface(void)       // Called when a Set Interface command is received
{
   AlternateSetting = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetInterface(void)       // Called when a Set Interface command is received
{
   EP0BUF[0] = AlternateSetting;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_GetStatus(void)
{
   return(TRUE);
}

BOOL DR_ClearFeature(void)
{
   return(TRUE);
}

BOOL DR_SetFeature(void)
{
   return(TRUE);
}

void DR_DeviceAcquire(void)
{
  EP0BUF[0] = DeviceAcquire();
  EP0BUF[1] = swd_PacketAck;
  EP0BUF[2] = swd_PacketData[0];
  EP0BUF[3] = swd_PacketData[1];
  EP0BUF[4] = swd_PacketData[2];
  EP0BUF[5] = swd_PacketData[3];
  EP0BCH = 0;
  EP0BCL = 6;
  EP0CS |= bmHSNAK;
}

void DR_ExitProgrammingMode(void)
{
  ExitProgrammingMode();
  EP0BUF[0] = 0;
  EP0BUF[1] = swd_PacketAck;
  EP0BUF[2] = swd_PacketData[0];
  EP0BUF[3] = swd_PacketData[1];
  EP0BUF[4] = swd_PacketData[2];
  EP0BUF[5] = swd_PacketData[3];
  EP0BCH = 0;
  EP0BCL = 6;
  EP0CS |= bmHSNAK;
}

void DR_PollSromStatus(void)
{
  EP0BUF[0] = PollSromStatus();
  EP0BUF[1] = swd_PacketAck;
  EP0BUF[2] = swd_PacketData[0];
  EP0BUF[3] = swd_PacketData[1];
  EP0BUF[4] = swd_PacketData[2];
  EP0BUF[5] = swd_PacketData[3];
  EP0BCH = 0;
  EP0BCL = 6;
  EP0CS |= bmHSNAK;
}

void DR_FetchStatus(void)
{
  EP0BUF[0] = swd_PacketAck;
  EP0BUF[1] = swd_PacketData[0];
  EP0BUF[2] = swd_PacketData[1];
  EP0BUF[3] = swd_PacketData[2];
  EP0BUF[4] = swd_PacketData[3];
  EP0BCH = 0;
  EP0BCL = 5;
  EP0CS |= bmHSNAK;
}

#define Write_DAP2(r, v, i) \
    Copy_Data(v, i); \
    swd_PacketHeader = (r); \
    Swd_WritePacket()

void DR_WriteIO(void)
{
  BYTE len;
  EP0BCH=0;
  EP0BCL=0;
  while(EP0CS & bmEPBUSY);
  len = EP0BCL;
  if (8 == len)
  {
    Write_DAP2(DPACC_AP_TAR_WRITE, EP0BUF, 0);
    if (swd_PacketAck == SWD_OK_ACK)
    {
	    Write_DAP2(DPACC_AP_DRW_WRITE, EP0BUF, 4);
    }
  }
}

void DR_ReadIO(void)
{
  BYTE len;
  EP0BCH=0;
  EP0BCL=0;
  while(EP0CS & bmEPBUSY);
  len = EP0BCL;
  if (4 == len)
  {
    Write_DAP2(DPACC_AP_TAR_WRITE, EP0BUF, 0);
    if (swd_PacketAck == SWD_OK_ACK)
    {
      Read_DAP(DPACC_AP_DRW_READ);
	  if (swd_PacketAck == SWD_OK_ACK)
	  {
        Read_DAP(DPACC_AP_DRW_READ);
	  }
    }
  }
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
   GotSUD = TRUE;            // Set flag
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUDAV;         // Clear SUDAV IRQ
}

// Setup Token Interrupt Handler
void ISR_Sutok(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUTOK;         // Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSOF;            // Clear SOF IRQ
}

void ISR_Ures(void) interrupt 0
{
   // whenever we get a USB reset, we should revert to full speed mode
   pConfigDscr = pFullSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
   pOtherConfigDscr = pHighSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmURES;         // Clear URES IRQ
}

void ISR_Susp(void) interrupt 0
{
   Sleep = TRUE;
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
   if (EZUSB_HIGHSPEED())
   {
      pConfigDscr = pHighSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
      pOtherConfigDscr = pFullSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   }

   EZUSB_IRQ_CLEAR();
   USBIRQ = bmHSGRANT;
}

void ISR_Ep0ack(void) interrupt 0
{
}
void ISR_Stub(void) interrupt 0
{
}
void ISR_Ep0in(void) interrupt 0
{
}
void ISR_Ep0out(void) interrupt 0
{
}
void ISR_Ep1in(void) interrupt 0
{
}
void ISR_Ep1out(void) interrupt 0
{
}
void ISR_Ep2inout(void) interrupt 0
{
}
void ISR_Ep4inout(void) interrupt 0
{
}
void ISR_Ep6inout(void) interrupt 0
{
}
void ISR_Ep8inout(void) interrupt 0
{
}
void ISR_Ibn(void) interrupt 0
{
}
void ISR_Ep0pingnak(void) interrupt 0
{
}
void ISR_Ep1pingnak(void) interrupt 0
{
}
void ISR_Ep2pingnak(void) interrupt 0
{
}
void ISR_Ep4pingnak(void) interrupt 0
{
}
void ISR_Ep6pingnak(void) interrupt 0
{
}
void ISR_Ep8pingnak(void) interrupt 0
{
}
void ISR_Errorlimit(void) interrupt 0
{
}
void ISR_Ep2piderror(void) interrupt 0
{
}
void ISR_Ep4piderror(void) interrupt 0
{
}
void ISR_Ep6piderror(void) interrupt 0
{
}
void ISR_Ep8piderror(void) interrupt 0
{
}
void ISR_Ep2pflag(void) interrupt 0
{
}
void ISR_Ep4pflag(void) interrupt 0
{
}
void ISR_Ep6pflag(void) interrupt 0
{
}
void ISR_Ep8pflag(void) interrupt 0
{
}
void ISR_Ep2eflag(void) interrupt 0
{
}
void ISR_Ep4eflag(void) interrupt 0
{
}
void ISR_Ep6eflag(void) interrupt 0
{
}
void ISR_Ep8eflag(void) interrupt 0
{
}
void ISR_Ep2fflag(void) interrupt 0
{
}
void ISR_Ep4fflag(void) interrupt 0
{
}
void ISR_Ep6fflag(void) interrupt 0
{
}
void ISR_Ep8fflag(void) interrupt 0
{
}
void ISR_GpifComplete(void) interrupt 0
{
}
void ISR_GpifWaveform(void) interrupt 0
{
}

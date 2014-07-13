// becker_read.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ProgrammingSteps.h"
#include <stdio.h>

#define VC_DEVICE_ACQ          0xa1
#define VC_EXIT_PROG_MODE      0xa2
#define VC_POLL_SROM_STATUS    0xa3
#define VC_FETCH_STATUS        0xa4
#define VC_WRITE_IO            0xa5
#define VC_READ_IO             0xa6

#define NUMBER_OF_FLASH_ROWS_HEX_FILE        256
#define FLASH_ROW_BYTE_SIZE_HEX_FILE         128
#define FLASH_PROTECTION_BYTE_SIZE_HEX_FILE  32

static CCyUSBDevice USBDevice;
static CCyControlEndPoint *ept = USBDevice.ControlEndPt;

BYTE swd_PacketAck;
BYTE swd_PacketData[4];

static BYTE RM_FetchStatus()
{
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode = VC_FETCH_STATUS;
	ept->Value = 0;
	ept->Index = 0;

  UCHAR buf[5];
	LONG buflen = sizeof(buf);
  ept->XferData(buf, buflen);
  if (buflen != 5)
  {
    return -1;
  }
  swd_PacketAck = buf[0];
  swd_PacketData[0] = buf[1];
  swd_PacketData[1] = buf[2];
  swd_PacketData[2] = buf[3];
  swd_PacketData[3] = buf[4];
  return 0;
}

static BYTE RM_WriteIO(DWORD a, DWORD d)
{
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_VENDOR;
	ept->Direction = DIR_TO_DEVICE;
	ept->ReqCode = VC_WRITE_IO;
	ept->Value = 0;
	ept->Index = 0;

  UCHAR buf[8];
	LONG buflen = sizeof(buf);
  buf[0] = (BYTE)a;
  buf[1] = (BYTE)(a >> 8);
  buf[2] = (BYTE)(a >> 16);
  buf[3] = (BYTE)(a >> 24);
  buf[4] = (BYTE)d;
  buf[5] = (BYTE)(d >> 8);
  buf[6] = (BYTE)(d >> 16);
  buf[7] = (BYTE)(d >> 24);
  ept->XferData(buf, buflen);
  return RM_FetchStatus();
}

static BYTE RM_ReadIO(DWORD a)
{
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_VENDOR;
	ept->Direction = DIR_TO_DEVICE;
	ept->ReqCode = VC_READ_IO;
	ept->Value = 0;
	ept->Index = 0;

  UCHAR buf[4];
	LONG buflen = sizeof(buf);
  buf[0] = (BYTE)a;
  buf[1] = (BYTE)(a >> 8);
  buf[2] = (BYTE)(a >> 16);
  buf[3] = (BYTE)(a >> 24);
  ept->XferData(buf, buflen);
  return RM_FetchStatus();
}

static BYTE RM_ReadFunc(BYTE fnCode)
{
  BYTE result;
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode = fnCode;
	ept->Value = 0;
	ept->Index = 0;

  UCHAR buf[6];
	LONG buflen = sizeof(buf);
  ept->XferData(buf, buflen);
  if (buflen != 6)
  {
    return -1;
  }
  result = buf[0];
  swd_PacketAck = buf[1];
  swd_PacketData[0] = buf[2];
  swd_PacketData[1] = buf[3];
  swd_PacketData[2] = buf[4];
  swd_PacketData[3] = buf[5];
  return result;
}

unsigned char DeviceAcquire(void)
{
  return RM_ReadFunc(VC_DEVICE_ACQ);
}

void ExitProgrammingMode(void)
{
  RM_ReadFunc(VC_EXIT_PROG_MODE);
}

unsigned char PollSromStatus(void)
{
  return RM_ReadFunc(VC_POLL_SROM_STATUS);
}

void Write_IO(unsigned long addr_32, unsigned long data_32)
{
  RM_WriteIO(addr_32, data_32);
}

void Read_IO(unsigned long addr_32, unsigned long *data_32)
{
  RM_ReadIO(addr_32);
  *data_32 = (((unsigned long)swd_PacketData[3] << 24) | 
              ((unsigned long)swd_PacketData[2] << 16) | 
              ((unsigned long)swd_PacketData[1] << 8) | 
              ((unsigned long)swd_PacketData[0]));
}

void ExitProgrammingMode();

HexFileParser hexParser;

void HEX_ReadSiliconId(unsigned long *hexSiliconId)
{
  unsigned char i;
  unsigned char metadata[12];
  hexParser.getDataFrom(0x90500000, sizeof(metadata), metadata);

  for(i = 0; i < 4; i++)
  {
    *hexSiliconId = *hexSiliconId | ((unsigned long)(metadata[2 + i]) << (8*i));		
  }
}

void HEX_ReadRowData(unsigned short rowCount, unsigned char * rowData)
{
  hexParser.getDataFrom(rowCount * FLASH_ROW_BYTE_SIZE_HEX_FILE, 
    FLASH_ROW_BYTE_SIZE_HEX_FILE, rowData);
}

void HEX_ReadChipProtectionData(unsigned char * chipProtectionData)
{
  hexParser.getDataFrom(0x90600000, 1, chipProtectionData);
}

void HEX_ReadRowProtectionData(unsigned char rowProtectionByteSize, unsigned char * rowProtectionData)
{
  hexParser.getDataFrom(0x90400000, rowProtectionByteSize, rowProtectionData);
}

void HEX_ReadChecksumData(unsigned short * checksumData)
{
  unsigned char data[2];
  hexParser.getDataFrom(0x90300000, 2, data);
  *checksumData = ((unsigned short)data[0] << 8) | (data[1]);
}

void ProgramDevice(void)
{
  printf("Acquring device...");
  if(DeviceAcquire() != SUCCESS)
  {
    printf("error.\n");
    return;
  }

  printf("done\nVerifying chip ID...");
  if(VerifySiliconId() != SUCCESS)    
  {
    printf("error.\n");
    return;
  }

  printf("done\nErasing flash...");
  if(EraseAllFlash() != SUCCESS)             
  {
    printf("error.\n");
    return;
  }

  printf("done\nChecking flash checksum...");
  if(ChecksumPrivileged() != SUCCESS)                    
  {
    printf("error.\n");
    return;
  }

  printf("done\nProgramming...");
  if(ProgramFlash() != SUCCESS)          
  {
    printf("error.\n");
    return;
  }

  printf("done\nVerifying...");
  if(VerifyFlash() != SUCCESS)       
  {
    printf("error.\n");
    return;
  }

  printf("done\nProgramming sector protection...");
  if(ProgramProtectionSettings() != SUCCESS) 
  {
    printf("error.\n");
    return;
  }

  printf("done\nVerifying sector protection...");
  if(VerifyProtectionSettings() != SUCCESS)
  {
    printf("error.\n");
    return;
  }

  printf("done\nVerifying final checksum...");
  if(VerifyChecksum() != SUCCESS) 
  {
    printf("error.\n");
    return;
  }

  printf("done\nSUCCESS\n");
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    printf("Usage: swd_prog <hex file name>\n");
    return 1;
  }
  if (!hexParser.parse(argv[1]))
  {
    printf("Error persing the input file.\n");
    return -1;
  }

  if (!USBDevice.IsOpen()) {
    printf("Can't connect to the FX2LP USB device. Check the cable.\n");
    return -1;
  }
  ProgramDevice();
  ExitProgrammingMode();
  USBDevice.Close();
	return 0;
}

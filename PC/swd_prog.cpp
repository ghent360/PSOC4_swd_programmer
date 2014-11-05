// becker_read.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ProgrammingSteps.h"
#include "DataFetch.h"
#include <stdio.h>

#define VC_DEVICE_ACQ          0xa1
#define VC_EXIT_PROG_MODE      0xa2
#define VC_POLL_SROM_STATUS    0xa3
#define VC_FETCH_STATUS        0xa4
#define VC_WRITE_IO            0xa5
#define VC_READ_IO             0xa6

static CCyUSBDevice gUSBDevice;
BYTE gSWD_PacketAck;
static BYTE gSWD_PacketData[4];
static HexFileParser gHexParser;
static bool gProgram = false;
static bool gProgramProtection = false;
static bool gProg16k = false;

static BYTE RM_FetchStatus()
{
  CCyControlEndPoint *ept = gUSBDevice.ControlEndPt;

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
  gSWD_PacketAck = buf[0];
  gSWD_PacketData[0] = buf[1];
  gSWD_PacketData[1] = buf[2];
  gSWD_PacketData[2] = buf[3];
  gSWD_PacketData[3] = buf[4];
  return 0;
}

static BYTE RM_WriteIO(DWORD a, DWORD d)
{
  CCyControlEndPoint *ept = gUSBDevice.ControlEndPt;

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
  CCyControlEndPoint *ept = gUSBDevice.ControlEndPt;

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
  CCyControlEndPoint *ept = gUSBDevice.ControlEndPt;

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
  gSWD_PacketAck = buf[1];
  gSWD_PacketData[0] = buf[2];
  gSWD_PacketData[1] = buf[3];
  gSWD_PacketData[2] = buf[4];
  gSWD_PacketData[3] = buf[5];
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
  *data_32 = (((unsigned long)gSWD_PacketData[3] << 24) | 
              ((unsigned long)gSWD_PacketData[2] << 16) | 
              ((unsigned long)gSWD_PacketData[1] << 8) | 
              ((unsigned long)gSWD_PacketData[0]));
}

void HEX_ReadSiliconId(unsigned long *hexSiliconId)
{
  unsigned char i;
  unsigned char metadata[12];
  gHexParser.getDataFrom(0x90500000, sizeof(metadata), metadata);

  for(i = 0; i < 4; i++)
  {
    *hexSiliconId = *hexSiliconId | ((unsigned long)(metadata[2 + i]) << (8*i));		
  }
}

void HEX_ReadRowData(unsigned short rowCount, unsigned char * rowData)
{
  gHexParser.getDataFrom(rowCount * FLASH_ROW_BYTE_SIZE_HEX_FILE, 
    FLASH_ROW_BYTE_SIZE_HEX_FILE, rowData);
}

void HEX_ReadChipProtectionData(unsigned char * chipProtectionData)
{
  gHexParser.getDataFrom(0x90600000, 1, chipProtectionData);
}

void HEX_ReadRowProtectionData(unsigned char rowProtectionByteSize, unsigned char * rowProtectionData)
{
  gHexParser.getDataFrom(0x90400000, rowProtectionByteSize, rowProtectionData);
}

void HEX_ReadChecksumData(unsigned short * checksumData)
{
  unsigned char data[2];
  gHexParser.getDataFrom(0x90300000, 2, data);
  *checksumData = ((unsigned short)data[0] << 8) | (data[1]);
}

unsigned short GetFlashRowCount()
{
    return gProg16k ? 128 : 256;
}

void ProgramDevice(void)
{
  unsigned char status;
  printf("Acquring device...");
  status = DeviceAcquire();
  if (status != SUCCESS)
  {
    printf("error code %d.\n", status);
    return;
  }

  printf("done\nVerifying chip ID...");
  status = VerifySiliconId();
  if (status != SUCCESS)
  {
    unsigned long hexSiliconId;
    printf("error code %d.\n", status);
    HEX_ReadSiliconId(&hexSiliconId);
    printf("Device reports silicon ID:%08x, but the file was compiled for %08x\n",
      GetSiliconId(), hexSiliconId);
    return;
  }

  if (gProgram)
  {
    printf("done\nErasing flash...");
    status = EraseAllFlash();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      return;
    }

    printf("done\nChecking flash checksum...");
    status = ChecksumPrivileged();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      return;
    }

    printf("done\nProgramming...");
    status = ProgramFlash();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      return;
    }
  }

  printf("done\nVerifying...");
  status = VerifyFlash();
  if (status != SUCCESS)
  {
    printf("error code %d.\n", status);
    if (status >= 2)
    {
      PrintFlashVerificationError(status - 2);
    }
    return;
  }

  if (gProgramProtection)
  {
    printf("done\nProgramming sector protection...");
    status = ProgramProtectionSettings();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      return;
    }
    printf("done\nVerifying sector protection...");
    status = VerifyProtectionSettings();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      return;
    }
  }

  printf("done\nVerifying final checksum...");
  status = VerifyChecksum();
  if (status != SUCCESS) 
  {
    printf("error code %d.\n", status);
    return;
  }

  printf("done\nSUCCESS\n");
}

void printUsage()
{
    printf("Usage: swd_prog [-y] [-yp] [-16] <hex file name>\n");
    printf("    by default it would perform only verification of the flash content.\n\n");
    printf("    specify -y to perform programming of the flash (WARNING: this \n"
           "    operation will erase all flash content, existing programming would be\n"
           "    lost)\n\n");
    printf("    add -yy to program the flash security settings. By default all flash \n"
           "    protection is off and anyone can read the flash content. You can set \n"
           "    flash protection bits, to secure some or all content. Use with caution\n"
           "    as you may render the SWD port unusable if you set the 'KILL' bit by \n"
           "    mistake.\n\n");
    printf("    specify -16 to program a device with 16KB flash. Default is 32KB flash.\n");
}

int main(int argc, char* argv[])
{
  int fileIdx = -1;
  if (argc < 2)
  {
    printUsage();
    return 1;
  }
  for (int idx = 1; idx < argc; idx++)
  {
    if (argv[idx][0] == '-')
    {
      if (argv[idx][1] == 'y')
      {
        if (argv[idx][2] == 0)
        {
          gProgram = true;
        }
        else if (argv[idx][2] == 'p')
        {
          gProgramProtection = true;
        }
        else
        {
          printf("Unknow argument %s\n", argv[idx]);
          printUsage();
          return 2;
        }
      }
      else if (argv[idx][1] == '1')
      {
        gProg16k = true;
      }
      else 
      {
        printf("Unknow argument %s\n", argv[idx]);
        printUsage();
        return 2;
      }
    }
    else
    {
      fileIdx = idx;
    }
  }
  if (fileIdx < 0)
  {
    printf("Please specify input file name.\n");
    printUsage();
    return 2;
  }
  if (!gHexParser.parse(argv[fileIdx]))
  {
    printf("Error persing the input file: %s.\n", argv[fileIdx]);
    return -1;
  }

  if (!gUSBDevice.IsOpen()) {
    printf("Can't connect to the FX2LP USB device. Check the USB cable.\n");
    return -1;
  }
  ProgramDevice();
  ExitProgrammingMode();
  gUSBDevice.Close();
	return 0;
}

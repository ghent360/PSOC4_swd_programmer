// becker_read.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ProgrammingSteps.h"
#include "DataFetch.h"
#include "DeviceFunctions.h"
#include <stdio.h>

static HexFileParser gHexParser;
static bool gProgram = false;
static bool gProgramProtection = false;
static bool gProg16k = false;
static bool gRead = false;

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

void ReadDevice(const char* fileName)
{
    static unsigned char buffer[32768];
    unsigned char status;
    size_t flashSize = sizeof(buffer);
    FILE* output = fopen(fileName, "wb");
    if (NULL == output)
    {
      printf("Can not open %s for writing.\n", fileName);
      return;
    }

    printf("Acquring device...");
    status = DeviceAcquire();
    if (status != SUCCESS)
    {
      printf("error code %d.\n", status);
      fclose(output);
      return;
    }
    printf("done\nReading Flash...");
    ReadFlash(buffer);
    printf("done\n");
    if (gProg16k)
    {
      flashSize = 16*1024;
    }
    fwrite(buffer, 1, flashSize, output);
    fclose(output);
}

void printUsage()
{
    printf("Usage: swd_prog [-r] [-y] [-yp] [-16] <file name>\n");
    printf("    by default it would perform only verification of the flash content.\n\n");
    printf("    specify -y to perform programming of the flash (WARNING: this \n"
           "    operation will erase all flash content, existing programming would be\n"
           "    lost)\n\n");
    printf("    add -yp to program the flash security settings. By default all flash \n"
           "    protection is off and the software can write the flash content. You can set \n"
           "    flash protection bits, to secure some or all content. Use with caution\n"
           "    as you may render the SWD port unusable if you set the 'KILL' bit by \n"
           "    mistake.\n\n");
    printf("    specify -16 to program a device with 16KB flash. Default is 32KB flash.\n\n");
    printf("    specify -r to read the flash content and write it to a binary file. If the \n");
    printf("    device has been switched to protected mode the flash content would be all 0s\n");
}

int main(int argc, char* argv[])
{
  int fileIdx = -1;
  const char* serial = NULL;
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
      else if (argv[idx][1] == 'r')
      {
        gRead = true;
      }
      else if (argv[idx][1] == 's')
      {
        serial = argv[idx+1];
        idx++;
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
    printf("Please specify file name.\n");
    printUsage();
    return 2;
  }

  if (!DeviceOpen()) {
    printf("Can't connect to the FX2LP USB device. Check the USB cable.\n");
    return -1;
  }
  if (gRead)
  {
    ReadDevice(argv[fileIdx]);
  }
  else
  {
    if (!gHexParser.parse(argv[fileIdx]))
    {
      printf("Error persing the input file: %s.\n", argv[fileIdx]);
    }
    else
    {
      if (serial)
      {
        if (!gHexParser.updateSerial(serial))
        {
          printf("Can not locate serial number placeholder in the input file\n");
        }
      }
      ProgramDevice();
    }
  }
  ExitProgrammingMode();
  DeviceClose();
	return 0;
}

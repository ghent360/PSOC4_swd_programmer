// Windows.cpp : Defines the windows platform speciffic layer
//

#include "stdafx.h"
#include "DeviceFunctions.h"

static CCyUSBDevice gUSBDevice;

BYTE gSWD_PacketAck;
BYTE gSWD_PacketData[4];

BYTE RM_FetchStatus()
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

BYTE RM_WriteIO(DWORD a, DWORD d)
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

BYTE RM_ReadIO(DWORD a)
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

BYTE RM_ReadFunc(BYTE fnCode)
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

int DeviceOpen()
{
  return gUSBDevice.IsOpen();
}

void DeviceClose()
{
  gUSBDevice.Close();
}
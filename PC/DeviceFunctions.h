#ifndef __DEVICE_FUNCTIONS_H__
#define __DEVICE_FUNCTIONS_H__

#define VC_DEVICE_ACQ          0xa1
#define VC_EXIT_PROG_MODE      0xa2
#define VC_POLL_SROM_STATUS    0xa3
#define VC_FETCH_STATUS        0xa4
#define VC_WRITE_IO            0xa5
#define VC_READ_IO             0xa6

extern BYTE gSWD_PacketAck;
extern BYTE gSWD_PacketData[4];

BYTE RM_FetchStatus();
BYTE RM_WriteIO(DWORD a, DWORD d);
BYTE RM_ReadIO(DWORD a);
BYTE RM_ReadFunc(BYTE fnCode);
int DeviceOpen();
void DeviceClose();

#endif // __DEVICE_FUNCTIONS_H__

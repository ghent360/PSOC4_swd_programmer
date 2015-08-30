// Windows.cpp : Defines the windows platform speciffic layer
//

#include "stdafx.h"
#include "DeviceFunctions.h"
#include <libusb-1.0/libusb.h>

static libusb_device_handle *gDevice = NULL;
BYTE gSWD_PacketAck;
BYTE gSWD_PacketData[4];

static int usb_write(
	uint8_t opcode,
	const unsigned char *data,
	uint16_t len) {
	int status;

	status = libusb_control_transfer(gDevice,
		LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		opcode,
		0,  // Value
		0,  // Index
		(unsigned char*)data,
		len,
		1000);  // Timeout in ms
	if (status != len) {
		if (status < 0)
			printf("usb_write: %s\n", libusb_error_name(status));
		else
			printf("usb_write: %d\n", status);
	}
	return status;
}

static int usb_read(
	uint8_t opcode,
	const unsigned char *data,
	uint16_t len) {
	int status;

	status = libusb_control_transfer(gDevice,
		LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
		opcode,
		0,  // Value
		0,  // Index
		(unsigned char*)data,
		len,
		1000);  // Timeout in ms
	if (status != len) {
		if (status < 0)
			printf("usb_read: %s\n", libusb_error_name(status));
		else
			printf("usb_read: %d\n", status);
	}
	return status;
}

BYTE RM_FetchStatus()
{
  unsigned char buf[5];
  int result;
  result = usb_read(VC_FETCH_STATUS, buf, sizeof(buf));
  if (result != 5)
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
  unsigned char buf[8];
  int result;

  buf[0] = (BYTE)a;
  buf[1] = (BYTE)(a >> 8);
  buf[2] = (BYTE)(a >> 16);
  buf[3] = (BYTE)(a >> 24);
  buf[4] = (BYTE)d;
  buf[5] = (BYTE)(d >> 8);
  buf[6] = (BYTE)(d >> 16);
  buf[7] = (BYTE)(d >> 24);
  result = usb_write(VC_WRITE_IO, buf, sizeof(buf));
  return RM_FetchStatus();
}

BYTE RM_ReadIO(DWORD a)
{
  unsigned char buf[4];
  int result;

  buf[0] = (BYTE)a;
  buf[1] = (BYTE)(a >> 8);
  buf[2] = (BYTE)(a >> 16);
  buf[3] = (BYTE)(a >> 24);

  result = usb_write(VC_READ_IO, buf, sizeof(buf));
  return RM_FetchStatus();
}

BYTE RM_ReadFunc(BYTE fnCode)
{
  BYTE result;
  unsigned char buf[6];
  int read_size;

  read_size = usb_read(fnCode, buf, sizeof(buf));
  if (read_size != 6)
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
  int result;

  result = libusb_init(NULL);
  if (result < 0) {
	printf("libusb_init failed\n");
    return 0;
  }
  gDevice = libusb_open_device_with_vid_pid(NULL, 0x04b4, 0x1004);
  if (gDevice == NULL) {
    printf("Programmer is not attached, check the power button?\n");
    return 0;
  }
  libusb_set_auto_detach_kernel_driver(gDevice, 1);
  result = libusb_claim_interface(gDevice, 0);
  if (result != LIBUSB_SUCCESS) {
    printf("USB claim interface failed\n");
    return 0;
  }
  return 1;
}

void DeviceClose()
{
  libusb_release_interface(gDevice, 0);
  if (gDevice != NULL) {
    libusb_close(gDevice);
    gDevice = NULL;
  }
  libusb_exit(NULL);
}

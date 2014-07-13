#ifndef __SWD_UPPERLAYER_H__
#define __SWD_UPPERLAYER_H__

#include "swd_packetlayer.h"

#define Read_DAP(x) \
    swd_PacketHeader = x; \
    Swd_ReadPacket()

#define Wrt_Data(l) \
    swd_PacketData[0] = (unsigned char)(l); \
	swd_PacketData[1] = (unsigned char)((l) >> 8); \
	swd_PacketData[2] = (unsigned char)((l) >> 16); \
	swd_PacketData[3] = (unsigned char)((l) >> 24)

#define Copy_Data(s, i) \
    swd_PacketData[0] = s[(i)]; \
	swd_PacketData[1] = s[(i)+1]; \
	swd_PacketData[2] = s[(i)+2]; \
	swd_PacketData[3] = s[(i)+3]

#define Write_DAP(r, v) \
    Wrt_Data(v); \
    swd_PacketHeader = (r); \
    Swd_WritePacket()

#define Write_IO(a, d) \
    Write_DAP(DPACC_AP_TAR_WRITE, (a));\
    if (swd_PacketAck == SWD_OK_ACK)\
    {\
	    Write_DAP(DPACC_AP_DRW_WRITE, (d));\
    }\

#define Read_IO(a) \
    Write_DAP(DPACC_AP_TAR_WRITE, a); \
    if (swd_PacketAck == SWD_OK_ACK) \
    { \
        Read_DAP(DPACC_AP_DRW_READ); \
		if (swd_PacketAck == SWD_OK_ACK) \
            Read_DAP(DPACC_AP_DRW_READ); \
    }

#define CMP_Data_EQ(l) \
    ((swd_PacketData[0] == (unsigned char)(l)) && \
	 (swd_PacketData[1] == (unsigned char)((l) >> 8)) && \
	 (swd_PacketData[2] == (unsigned char)((l) >> 16)) && \
	 (swd_PacketData[3] == (unsigned char)((l) >> 24)))

#define CMP_Data_NE(l) \
    ((swd_PacketData[0] != (unsigned char)(l)) || \
	 (swd_PacketData[1] != (unsigned char)((l) >> 8)) || \
	 (swd_PacketData[2] != (unsigned char)((l) >> 16)) || \
	 (swd_PacketData[3] != (unsigned char)((l) >> 24)))

#define Mask_Data(m) \
    swd_PacketData[0] &= (unsigned char)(m); \
	swd_PacketData[1] &= (unsigned char)((m) >> 8); \
	swd_PacketData[2] &= (unsigned char)((m) >> 16); \
	swd_PacketData[3] &= (unsigned char)((m) >> 24)

#define Mask_CMPEQ_Data(m, v) \
    (((swd_PacketData[0] & (unsigned char)(m)) == (unsigned char)(v)) && \
	 ((swd_PacketData[1] & (unsigned char)((m) >> 8))  == (unsigned char)((v) >> 8)) && \
	 ((swd_PacketData[2] & (unsigned char)((m) >> 16)) == (unsigned char)((v) >> 16)) && \
	 ((swd_PacketData[3] & (unsigned char)((m) >> 24)) == (unsigned char)((v) >> 24)))

#define Mask_CMPNE_Data(m, v) \
    (((swd_PacketData[0] & (unsigned char)(m)) != (unsigned char)(v)) || \
	 ((swd_PacketData[1] & (unsigned char)((m) >> 8))  != (unsigned char)((v) >> 8)) || \
	 ((swd_PacketData[2] & (unsigned char)((m) >> 16)) != (unsigned char)((v) >> 16)) || \
	 ((swd_PacketData[3] & (unsigned char)((m) >> 24)) != (unsigned char)((v) >> 24)))

#endif
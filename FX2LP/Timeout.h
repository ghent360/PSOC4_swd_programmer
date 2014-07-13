#ifndef __TIMEOUT_H
#define __TIMEOUT_H

#if defined CY8C40xx_FAMILY
	#define DEVICE_ACQUIRE_TIMEOUT 20
#else
	#define DEVICE_ACQUIRE_TIMEOUT 15
#endif
#define SROM_POLLING_TIMEOUT 10416

#endif /* __TIMEOUT_H */

#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <stdint.h>

uint32_t RVM_LwipRand(void);
#define LWIP_RAND()                  RVM_LwipRand()

#define BYTE_ORDER                   LITTLE_ENDIAN

#define U16_F                        "hu"
#define S16_F                        "hd"
#define X16_F                        "hx"
#define U32_F                        "u"
#define S32_F                        "d"
#define X32_F                        "x"
#define SZT_F                        "u"

#if defined(__CC_ARM)
#define PACK_STRUCT_BEGIN            __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x)         x
#else
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT           __attribute__((packed))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x)         x
#endif

#define LWIP_PLATFORM_DIAG(x)        do { (void)0; } while (0)
#define LWIP_PLATFORM_ASSERT(x)      do { (void)(x); __disable_irq(); for (;;) { } } while (0)

#endif /* LWIP_ARCH_CC_H */

#ifndef RVM_SERIAL_PROTOCOL_H
#define RVM_SERIAL_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define RVM_SERIAL_FRAME_HEADER_SIZE  18U
#define RVM_SERIAL_PROTOCOL_VERSION   1U
#define RVM_SERIAL_FORMAT_JPEG        1U
#define RVM_SERIAL_FORMAT_RGB565_LE   2U

uint32_t RVM_SerialProtocol_Crc32(const uint8_t *payload, size_t length);
void RVM_SerialProtocol_BuildHeader(uint8_t header[RVM_SERIAL_FRAME_HEADER_SIZE],
                                    uint8_t format,
                                    uint16_t width,
                                    uint16_t height,
                                    uint32_t payload_length,
                                    uint32_t payload_crc32);

#endif /* RVM_SERIAL_PROTOCOL_H */

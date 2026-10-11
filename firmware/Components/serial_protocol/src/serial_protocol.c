#include "serial_protocol.h"

static void RVM_SerialProtocol_WriteBe16(uint8_t *destination,
                                         uint16_t value)
{
    destination[0] = (uint8_t)(value >> 8U);
    destination[1] = (uint8_t)value;
}

static void RVM_SerialProtocol_WriteBe32(uint8_t *destination,
                                         uint32_t value)
{
    destination[0] = (uint8_t)(value >> 24U);
    destination[1] = (uint8_t)(value >> 16U);
    destination[2] = (uint8_t)(value >> 8U);
    destination[3] = (uint8_t)value;
}

uint32_t RVM_SerialProtocol_Crc32(const uint8_t *payload, size_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    size_t index;
    uint8_t bit;

    if ((payload == 0) && (length != 0U))
    {
        return 0U;
    }

    for (index = 0U; index < length; ++index)
    {
        crc ^= payload[index];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc ^ 0xFFFFFFFFUL;
}

void RVM_SerialProtocol_BuildHeader(uint8_t header[RVM_SERIAL_FRAME_HEADER_SIZE],
                                    uint8_t format,
                                    uint16_t width,
                                    uint16_t height,
                                    uint32_t payload_length,
                                    uint32_t payload_crc32)
{
    if (header == 0)
    {
        return;
    }

    header[0] = 'R';
    header[1] = 'V';
    header[2] = 'M';
    header[3] = 'F';
    header[4] = RVM_SERIAL_PROTOCOL_VERSION;
    header[5] = format;
    RVM_SerialProtocol_WriteBe16(&header[6], width);
    RVM_SerialProtocol_WriteBe16(&header[8], height);
    RVM_SerialProtocol_WriteBe32(&header[10], payload_length);
    RVM_SerialProtocol_WriteBe32(&header[14], payload_crc32);
}

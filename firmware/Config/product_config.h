#ifndef RVM_PRODUCT_CONFIG_H
#define RVM_PRODUCT_CONFIG_H

#define RVM_PRODUCT_NAME                    "RVM F407 Camera"
#define RVM_DISPLAY_WIDTH                   800U
#define RVM_DISPLAY_HEIGHT                  480U
#define RVM_CAMERA_PREVIEW_WIDTH            320U
#define RVM_CAMERA_PREVIEW_HEIGHT           240U
#define RVM_SERIAL_IMAGE_WIDTH              320U
#define RVM_SERIAL_IMAGE_HEIGHT             240U
#define RVM_SERIAL_IMAGE_BAUD_RATE          1500000UL
#define RVM_SERIAL_IMAGE_USE_JPEG            1U
#define RVM_CAMERA_JPEG_QUALITY              4U
#define RVM_SERIAL_LOG_BAUD_RATE            115200UL
#define RVM_SERIAL_RX_RING_SIZE             128U
#define RVM_SERIAL_DEFERRED_LOG_SIZE         1024U
#define RVM_SERIAL_TX_DMA_CHUNK_SIZE         60000UL
#define RVM_CAMERA_RGB565_SWAP_BYTES        0
#define RVM_CAMERA_PREVIEW_LCD_X            270U
#define RVM_CAMERA_PREVIEW_LCD_Y            120U
#define RVM_LCD_SCAN_MODE                    5U
#define RVM_CAMERA_STATS_PERIOD_MS           5000UL
#define RVM_EXPECTED_OV5640_PRODUCT_ID_HIGH  0x56U

#if (RVM_SERIAL_IMAGE_WIDTH > 320U) || (RVM_SERIAL_IMAGE_HEIGHT > 240U)
#error "Serial image dimensions exceed the reserved frame slot"
#endif

#if (RVM_SERIAL_IMAGE_USE_JPEG > 1U)
#error "RVM_SERIAL_IMAGE_USE_JPEG must be 0 or 1"
#endif

#if (RVM_CAMERA_JPEG_QUALITY > 63U)
#error "OV5640 JPEG quantization scale must be in the range 0..63"
#endif

#if (((RVM_SERIAL_IMAGE_WIDTH * RVM_SERIAL_IMAGE_HEIGHT * 2U) % 4U) != 0U)
#error "DCMI frame size must be a whole number of 32-bit DMA words"
#endif

#if (RVM_SERIAL_TX_DMA_CHUNK_SIZE > 65535UL)
#error "USART DMA chunk exceeds the 16-bit NDTR limit"
#endif

#endif /* RVM_PRODUCT_CONFIG_H */

#ifndef RVM_SERIAL_IMAGE_SERVICE_H
#define RVM_SERIAL_IMAGE_SERVICE_H

#include <stdbool.h>

void RVM_SerialImageService_Init(void);
void RVM_SerialImageService_Process(void);
bool RVM_SerialImageService_IsBusy(void);

#endif /* RVM_SERIAL_IMAGE_SERVICE_H */

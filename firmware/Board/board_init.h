#ifndef RVM_BOARD_INIT_H
#define RVM_BOARD_INIT_H

typedef enum
{
    RVM_BOARD_STATUS_OK = 0,
    RVM_BOARD_STATUS_ERROR
} RVM_BoardStatus;

RVM_BoardStatus RVM_Board_Init(void);

#endif /* RVM_BOARD_INIT_H */

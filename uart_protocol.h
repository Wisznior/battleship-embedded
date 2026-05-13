/**
 * 3 bajty:
 * [TYPE] [ROW] [COL]
 * TYPE:
 * -'S' - strał - nadajemy row, col
 * -'R' - wynik strzału - row=ShotResult, col=0 (lub zatopiony statek idx)
 * -'A' - potwierdzenie - handshake przy starcie
 * -'W' - koniec gry
 */

#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include "Driver_USART.h"
#include "game_logic/battleship.h"

extern ARM_DRIVER_USART Driver_USART1;
#define UART_DRIVER  Driver_USART1
#define UART_BAUD    9600u

typedef enum {
    FRAME_SHOT   = 'S',
    FRAME_RESULT = 'R',
    FRAME_ACK    = 'A',
    FRAME_WIN    = 'W'
} FrameType;

typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} UartFrame;

void uart_init(void);

void uart_send_shot(uint8_t row, uint8_t col);

void uart_send_result(ShotResult result);

void uart_send_ack(void);

uint8_t uart_try_receive(UartFrame *out);

void uart_wait_receive(UartFrame *out);

#endif
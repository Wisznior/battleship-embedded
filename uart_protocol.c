#include "uart_protocol.h"
#include <string.h>

// wskaznik na sterownik
static ARM_DRIVER_USART *drv = &UART_DRIVER;

// Bufor odbiorczy
static volatile uint8_t  rx_buf[3];
static volatile uint8_t  rx_count = 0;
static volatile uint8_t  rx_ready = 0;

//Callback USART
static void uart_event_handler(uint32_t event)
{
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        rx_count++;
        if (rx_count < 3) {
            // Odbierz kolejny bajt ramki
            drv->Receive((void *)(rx_buf + rx_count), 1);
        } else {
            // Pełna ramka odebrana
            rx_ready  = 1;
            rx_count  = 0;
        }
    }

    if (event & ARM_USART_EVENT_RX_OVERFLOW) {
        // Przepełnienie bufora - zignoruj dane i zacznij od nowa
        rx_count = 0;
        drv->Receive((void *)rx_buf, 1);
    }
}

void uart_init(void)
{
    drv->Initialize(uart_event_handler);
    drv->PowerControl(ARM_POWER_FULL);
    drv->Control(
        ARM_USART_MODE_ASYNCHRONOUS |
        ARM_USART_DATA_BITS_8       |
        ARM_USART_PARITY_NONE       |
        ARM_USART_STOP_BITS_1       |
        ARM_USART_FLOW_CONTROL_NONE,
        UART_BAUD
    );
    drv->Control(ARM_USART_CONTROL_TX, 1);
    drv->Control(ARM_USART_CONTROL_RX, 1);

    drv->Receive((void *)rx_buf, 1);
}

static void send_frame(uint8_t type, uint8_t b1, uint8_t b2)
{
    uint8_t frame[3];
    frame[0] = type;
    frame[1] = b1;
    frame[2] = b2;

    while (drv->GetTxCount() != 0);
    drv->Send(frame, 3);
}

void uart_send_shot(uint8_t row, uint8_t col)
{
    send_frame(FRAME_SHOT, row, col);
}

void uart_send_result(ShotResult result)
{
    send_frame(FRAME_RESULT, (uint8_t)result, 0);
}

void uart_send_ack(void)
{
    send_frame(FRAME_ACK, 0, 0);
}

//nieblokujące sprawdzanie czy jest ramka do odebrania
uint8_t uart_try_receive(UartFrame *out)
{
    if (!rx_ready) {
        return 0;
    }

    out->type = rx_buf[0];
    out->row  = rx_buf[1];
    out->col  = rx_buf[2];

    rx_ready = 0;
    drv->Receive((void *)rx_buf, 1);

    return 1;
}

void uart_wait_receive(UartFrame *out)
{
    while (!uart_try_receive(out));
}
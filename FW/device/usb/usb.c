/*
 * File: usb.c
 * Date: 07.02.2014
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */
#include "main.h"
#include "usb.h"
#include "usb_desc.h"

extern void usb_tx(void);

uint8_t usb_rx_buffer[16];
uint8_t * usb_tx_buffer_pointer;
uint8_t usb_tx_count;
uint8_t usb_rx_count;
uint8_t count = 0;
uint8_t index = 0;
uint8_t data[2];
struct usb_type usb;

extern const uint8_t usb_device_descriptor[];
extern const uint8_t usb_configuration_descriptor[];
extern const uint8_t usb_report_descriptor[];

void usb_init(void) {
    usb.state = USB_STATE_IDLE;
    usb.event = USB_EVENT_NO;
    usb.setup_address  = 0;
    usb.enum_complete = 0;
    usb.data_sync=USB_PID_DATA1;
    usb.device_address = 0;
}

#define CLEAR6 0xBF
#define CLEAR7 0x7F
#define SET6 0x40
#define SET7 0x80
#define CLEAR67 0x3F
#define SET67 0xC0
@inline void usb_send_nack(void) {
    GPIOC->ODR|=SET6;
    GPIOC->CR1|=SET67;
    GPIOC->CR2|=SET67;
    GPIOC->DDR|=SET67;

    data[0] = USB_SYNC;
    data[1] = USB_PID_NACK;

    usb_tx_count = 2;
    usb_tx_buffer_pointer = &data[0];

    usb_tx();

    nop();
    nop();
    nop();
    nop();
    nop();  //EOP high
    GPIOC->ODR|=SET6;
    nop();
    //GPIOC->ODR=0;

    GPIOC->CR2&=CLEAR67;
    GPIOC->CR1&=CLEAR67;
    GPIOC->DDR&=CLEAR67;
}

@inline void usb_send_ack(void) {
    GPIOC->ODR|=SET6;
    GPIOC->CR1|=SET67;
    GPIOC->CR2|=SET67;
    GPIOC->DDR|=SET67;

    data[0] = USB_SYNC;
    data[1] = USB_PID_ACK;

    usb_tx_count = 2;
    usb_tx_buffer_pointer = &data[0];

    usb_tx();

    nop();
    nop();
    nop();
    nop();
    nop();  //EOP high
    GPIOC->ODR|=SET6;
    nop();
    //GPIOC->ODR=0;

    GPIOC->CR2&=CLEAR67;
    GPIOC->CR1&=CLEAR67;
    GPIOC->DDR&=CLEAR7;
    GPIOC->DDR&=CLEAR67;
}

@inline void usb_send_answer(void) {
    GPIOC->ODR|=SET6;
    GPIOC->CR1|=SET67;
    GPIOC->CR2|=SET67;
    GPIOC->DDR|=SET67;

    usb_tx_count = usb.tx_length;
    usb_tx_buffer_pointer = &usb.tx_buffer[0];

    usb_tx();
    nop();
    nop();
    nop();
    nop();
    nop();  //EOP high
    GPIOC->ODR|=SET6;
    nop();
    //GPIOC->ODR=0;
    GPIOC->CR2&=CLEAR67;
    GPIOC->CR1&=CLEAR67;
    GPIOC->DDR&=CLEAR67;
}

@inline void usb_copy_rx_buffer(void) {
    for (index = 0; index < 16; index++)
        usb.rx_buffer[index] = usb_rx_buffer[index];
}

void usb_rx_ok(void) {
    switch (usb_rx_buffer[1]) {
    case (USB_PID_SETUP): {
        usb.state = USB_STATE_SETUP;
        usb.data_sync=USB_PID_DATA1;
        break;
    }
    case (USB_PID_OUT): {
        usb.state = USB_STATE_OUT;
        break;
    }
    case (USB_PID_IN): {
        if((usb_rx_buffer[2]&0x7F)==usb.device_address){
            //detect comm timeout
            watchdog_timeout=0;
            IWDG->KR = IWDG_KEY_REFRESH;
            if(usb.setup_address!=0)	//需要找个更好的地方
            {
                usb.device_address=usb.setup_address;
            }
            if (usb.event == USB_EVENT_READY_DATA_IN) {
                usb.state = USB_STATE_IN;
                usb_send_answer();
                usb.event = USB_EVENT_WAIT_DATA_IN;
            } else {
                usb_send_nack();
            }
        }
        break;
    }
    case (USB_PID_DATA0): {
        if (usb.state == USB_STATE_SETUP) {
            usb.state = USB_STATE_IN;
            usb_send_ack();
            usb_copy_rx_buffer();
            usb.event = USB_EVENT_RECEIVE_SETUP_DATA;
        } else if (usb.state == USB_STATE_OUT) {
            usb_send_ack();
            usb.event = USB_EVENT_NO;
        }

        break;
    }
    case (USB_PID_DATA1): {
        if (usb.state == USB_STATE_OUT) {
            usb_send_ack();
            usb.event = USB_EVENT_NO;
        }
        break;
    }
    case (USB_PID_ACK): {
        break;
    }
    case (USB_PID_NACK): {
        break;
    }
    default: {
        usb.state = USB_STATE_IDLE;
        break;
    }
    }
}
@inline void usb_calc_crc16(uint8_t * buffer, uint8_t length) {
    uint16_t crc = 0xFFFF;
    uint8_t index;
    uint8_t i;

    for (index = 0; index < length; index++) {
        crc ^= *buffer++;
        for (i = 8; i--;) {
            if ((crc & BIT(0)) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    crc = ~crc;

    *buffer++ = (uint8_t) crc;
    *buffer = (uint8_t) (crc >> 8);
}

void usb_prepare_data(uint8_t * buffer, uint8_t length, uint8_t mode) {
    uint8_t index;

    if(mode)
        usb.data_sync = USB_PID_DATA1;

    while (length > 0) {
        usb.tx_buffer[0] = USB_SYNC;
        usb.tx_buffer[1] = usb.data_sync;

        if (length >= 8) {
            usb.tx_length = 12;

            for (index = 2; index < 10; index++)
                usb.tx_buffer[index] = *buffer++;

            length -= 8;
        } else {
            usb.tx_length = (uint8_t) (4 + length);

            for (index = 2; index < 2 + length; index++)
                usb.tx_buffer[index] = *buffer++;

            length = 0;
        }

        // calculate CRC
        usb_calc_crc16(&usb.tx_buffer[2], (uint8_t) (usb.tx_length - 4));

        // toggle data0 data1
        usb.data_sync = (uint8_t)((usb.data_sync==(uint8_t)USB_PID_DATA1)?USB_PID_DATA0:USB_PID_DATA1);

        // data is available to send out
        usb.event = USB_EVENT_READY_DATA_IN;

        // wait for transmission and then start the next
        while (usb.event == USB_EVENT_READY_DATA_IN) {
            if (usb.event == USB_EVENT_WAIT_DATA_IN)
                break;
        }
    }
}

@inline void usb_send_null_data(void) {
    usb.tx_length = 4;
    usb.tx_buffer[0] = 0x80;
    usb.tx_buffer[1] = usb.data_sync;
    usb.tx_buffer[2] = 0;
    usb.tx_buffer[3] = 0;
    usb.data_sync = (uint8_t)((usb.data_sync==(uint8_t)USB_PID_DATA1)?USB_PID_DATA0:USB_PID_DATA1);
    usb.event = USB_EVENT_READY_DATA_IN;
}

@inline void usb_send_stall(void) {
    usb.tx_length = 2;
    usb.tx_buffer[0] = 0x80;
    usb.tx_buffer[1] = USB_PID_STALL;
    usb.event = USB_EVENT_READY_DATA_IN;
}

int usb_process(void) {
    if (usb.event == USB_EVENT_RECEIVE_SETUP_DATA) {
        switch (usb.rx_buffer[2]) {
        case (USB_REQUEST_TYPE_FROM_DEVICE): { //0x80
            switch (usb.rx_buffer[3]) {
            case (USB_REQUEST_GET_DESCRIPTOR): {
                switch (usb.rx_buffer[5]) {
                case (1): {	// device descriptor
                    usb_prepare_data(&usb_device_descriptor[0], ARRAY_LENGTH(usb_device_descriptor), 1);
                    break;
                }
                case (2): {	// configuration descriptor
                    if(usb.rx_buffer[8]<ARRAY_LENGTH(usb_configuration_descriptor))
                        usb_prepare_data(&usb_configuration_descriptor[0],usb.rx_buffer[8], 1);
                    else
                        usb_prepare_data(&usb_configuration_descriptor[0], ARRAY_LENGTH(usb_configuration_descriptor), 1);
                    break;
                }
                default:
                    break;
                }
                break;
            }
            }

            break;
        }
        case (USB_REQUEST_TYPE_TO_DEVICE): {	//0x00
            switch (usb.rx_buffer[3]) {
            case (USB_REQUEST_SET_ADDRESS): {	//0x05
                usb.setup_address = usb.rx_buffer[4];
                usb_send_null_data();
                break;
            }
            case (USB_REQUEST_SET_CONFIGURATION): {	//0x09
                usb_send_null_data();
                break;
            }
            default:
                break;
            }
            break;
        }
        case (0x81): {
            switch (usb.rx_buffer[3]) {
            case(USB_REQUEST_GET_DESCRIPTOR): { //0x06
                usb_prepare_data(&usb_report_descriptor[0], ARRAY_LENGTH(usb_report_descriptor), 1);
                usb.enum_complete=1;
                break;
            }
            default:
                break;
            }
            break;
        }
        case (0x21): {
            usb_send_stall();
            break;
        }
        }
    }
    return usb.event;
}


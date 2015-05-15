/*
 * File: usb.h
 * Date: 07.02.2014
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#ifndef USB_H_
#define USB_H_

extern uint8_t usb_tx_count;
extern uint8_t usb_tx_buffer[16];

struct usb_type
{
	volatile uint8_t state;
	volatile uint8_t event;

	uint8_t endpoint_number;
	uint8_t setup_address;
	uint8_t setup_endpoint;

	uint8_t rx_buffer[16];
	uint8_t rx_length;

	uint8_t tx_buffer[16];
	uint8_t tx_length;
    uint8_t data_sync;
    uint8_t enum_complete;
    uint8_t device_address;
    
};

extern struct usb_type usb;

enum usb_events_list
{
	USB_EVENT_NO = 0,
	USB_EVENT_RECEIVE_SETUP_DATA = 1,
	USB_EVENT_READY_DATA_IN = 2,
	USB_EVENT_WAIT_DATA_IN = 3,
	USB_EVENT
};

enum usb_states_list
{
	USB_STATE_IDLE = 0, //
	USB_STATE_SETUP = 1,
	USB_STATE_IN = 2,
	USB_STATE_OUT = 3,
	USB_STATE_DATA = 4,
	USB_STATE_ADDRESS_CHANGE = 5
};

enum usb_packets_id_list
{
    USB_SYNC = 0x80,
	USB_PID_SETUP = 0x2D,
	USB_PID_DATA0 = 0xC3,
	USB_PID_DATA1 = 0x4B,
	USB_PID_IN = 0x69,
	USB_PID_OUT = 0xE1,
	USB_PID_ACK = 0xD2,
	USB_PID_NACK = 0x5A,
	USB_PID_STALL = 0x1E
};

enum usb_request_standart_type_list
{
	USB_REQUEST_TYPE_TO_DEVICE = 0x00,
	USB_REQUEST_TYPE_TO_INTERFACE = 0x01,
	USB_REQUEST_TYPE_TO_ENDPOINT = 0x02,
	USB_REQUEST_TYPE_FROM_DEVICE = 0x80,
	USB_REQUEST_TYPE_FROM_INTERFACE = 0x81,
	USB_REQUEST_TYPE_FROM_ENDPOINT = 0x82
};

enum usb_request_list
{
	USB_REQUEST_GET_STATUS = 0x00,
	USB_REQUEST_SET_ADDRESS = 0x05,
	USB_REQUEST_GET_DESCRIPTOR = 0x06,
	USB_REQUEST_SET_DESCRIPTOR = 0x07,
	USB_REQUEST_GET_CONFIGURATION = 0x08,
	USB_REQUEST_SET_CONFIGURATION = 0x09
};

void usb_init(void);
int  usb_process(void);
void usb_prepare_data(uint8_t * buffer, uint8_t length, uint8_t mode) ;

#endif /* USB_H_ */

/** \file
 *
 *  Header file usbio.h.
 */
 
#ifndef _USBIO_H_
#define _USBIO_H_

#include "LUFA/Drivers/USB/USB.h"
#include "Descriptors.h"

#define RX_BUFFER_SIZE	64
#define TX_BUFFER_SIZE	64

uint16_t _rx_buffer_size;
uint8_t _tx_buffer_ix, _rx_buffer_ix;
uint8_t _tx_buffer[TX_BUFFER_SIZE], _rx_buffer[RX_BUFFER_SIZE];

static inline void usbio_initbuffers(void) __attribute__ ((always_inline));
static inline void usbio_initbuffers(void) {
	_tx_buffer_ix = _rx_buffer_ix = _rx_buffer_size = 0;
	memset(_tx_buffer, 0, TX_BUFFER_SIZE);
	memset(_rx_buffer, 0, RX_BUFFER_SIZE);
}

static inline void send_tx_buffer(void) __attribute__ ((always_inline));
static inline void send_tx_buffer(void) {
	/* Device must be connected and configured for the task to run */
	//while (USB_DeviceState != DEVICE_STATE_Configured)
	//	USB_USBTask();

	/* Select the IN stream endpoint */
	Endpoint_SelectEndpoint(IN_EP);
	
	/* Check if the current endpoint can be written to and that the next sample should be stored */
	while (!Endpoint_IsINReady())
		USB_USBTask();

	Endpoint_Write_8(_tx_buffer[0]);
	Endpoint_Write_8(_tx_buffer[1]);
	Endpoint_Write_8(_tx_buffer[2]);
	Endpoint_Write_8(_tx_buffer[3]);
	Endpoint_Write_8(_tx_buffer[4]);
	Endpoint_Write_8(_tx_buffer[5]);
	Endpoint_Write_8(_tx_buffer[6]);
	Endpoint_Write_8(_tx_buffer[7]);
	Endpoint_Write_8(_tx_buffer[8]);
	Endpoint_Write_8(_tx_buffer[9]);
	Endpoint_Write_8(_tx_buffer[10]);
	Endpoint_Write_8(_tx_buffer[11]);
	Endpoint_Write_8(_tx_buffer[12]);
	Endpoint_Write_8(_tx_buffer[13]);
	Endpoint_Write_8(_tx_buffer[14]);
	Endpoint_Write_8(_tx_buffer[15]);
	Endpoint_Write_8(_tx_buffer[16]);
	Endpoint_Write_8(_tx_buffer[17]);
	Endpoint_Write_8(_tx_buffer[18]);
	Endpoint_Write_8(_tx_buffer[19]);
	Endpoint_Write_8(_tx_buffer[20]);
	Endpoint_Write_8(_tx_buffer[21]);
	Endpoint_Write_8(_tx_buffer[22]);
	Endpoint_Write_8(_tx_buffer[23]);
	Endpoint_Write_8(_tx_buffer[24]);
	Endpoint_Write_8(_tx_buffer[25]);
	Endpoint_Write_8(_tx_buffer[26]);
	Endpoint_Write_8(_tx_buffer[27]);
	Endpoint_Write_8(_tx_buffer[28]);
	Endpoint_Write_8(_tx_buffer[29]);
	Endpoint_Write_8(_tx_buffer[30]);
	Endpoint_Write_8(_tx_buffer[31]);
	Endpoint_Write_8(_tx_buffer[32]);
	Endpoint_Write_8(_tx_buffer[33]);
	Endpoint_Write_8(_tx_buffer[34]);
	Endpoint_Write_8(_tx_buffer[35]);
	Endpoint_Write_8(_tx_buffer[36]);
	Endpoint_Write_8(_tx_buffer[37]);
	Endpoint_Write_8(_tx_buffer[38]);
	Endpoint_Write_8(_tx_buffer[39]);
	Endpoint_Write_8(_tx_buffer[40]);
	Endpoint_Write_8(_tx_buffer[41]);
	Endpoint_Write_8(_tx_buffer[42]);
	Endpoint_Write_8(_tx_buffer[43]);
	Endpoint_Write_8(_tx_buffer[44]);
	Endpoint_Write_8(_tx_buffer[45]);
	Endpoint_Write_8(_tx_buffer[46]);
	Endpoint_Write_8(_tx_buffer[47]);
	Endpoint_Write_8(_tx_buffer[48]);
	Endpoint_Write_8(_tx_buffer[49]);
	Endpoint_Write_8(_tx_buffer[50]);
	Endpoint_Write_8(_tx_buffer[51]);
	Endpoint_Write_8(_tx_buffer[52]);
	Endpoint_Write_8(_tx_buffer[53]);
	Endpoint_Write_8(_tx_buffer[54]);
	Endpoint_Write_8(_tx_buffer[55]);
	Endpoint_Write_8(_tx_buffer[56]);
	Endpoint_Write_8(_tx_buffer[57]);
	Endpoint_Write_8(_tx_buffer[58]);
	Endpoint_Write_8(_tx_buffer[59]);
	Endpoint_Write_8(_tx_buffer[60]);
	Endpoint_Write_8(_tx_buffer[61]);
	Endpoint_Write_8(_tx_buffer[62]);
	Endpoint_Write_8(_tx_buffer[63]);

	Endpoint_ClearIN();
	//USB_USBTask();
}

static inline void usbio_set_byte(const uint8_t c, const uint8_t transmit) __attribute__ ((always_inline));
static inline void usbio_set_byte(const uint8_t c, const uint8_t transmit) {
	_tx_buffer[_tx_buffer_ix] = c; ++_tx_buffer_ix;
	
	if (transmit) {
		_tx_buffer_ix = 0;
		send_tx_buffer();
	}
}

static inline void receive_rx_buffer(void) __attribute__ ((always_inline));
static inline void receive_rx_buffer(void) {
	/* Device must be connected and configured for the task to run */
	//while (USB_DeviceState != DEVICE_STATE_Configured)
	//	USB_USBTask();

	/* Select the OUT stream endpoint */
	Endpoint_SelectEndpoint(OUT_EP);

	while (!Endpoint_IsOUTReceived())
		USB_USBTask();

	_rx_buffer[0] = Endpoint_Read_8();
	_rx_buffer[1] = Endpoint_Read_8();
	_rx_buffer[2] = Endpoint_Read_8();
	_rx_buffer[3] = Endpoint_Read_8();
	_rx_buffer[4] = Endpoint_Read_8();
	_rx_buffer[5] = Endpoint_Read_8();
	_rx_buffer[6] = Endpoint_Read_8();
	_rx_buffer[7] = Endpoint_Read_8();
	_rx_buffer[8] = Endpoint_Read_8();
	_rx_buffer[9] = Endpoint_Read_8();
	_rx_buffer[10] = Endpoint_Read_8();
	_rx_buffer[11] = Endpoint_Read_8();
	_rx_buffer[12] = Endpoint_Read_8();
	_rx_buffer[13] = Endpoint_Read_8();
	_rx_buffer[14] = Endpoint_Read_8();
	_rx_buffer[15] = Endpoint_Read_8();
	_rx_buffer[16] = Endpoint_Read_8();
	_rx_buffer[17] = Endpoint_Read_8();
	_rx_buffer[18] = Endpoint_Read_8();
	_rx_buffer[19] = Endpoint_Read_8();
	_rx_buffer[20] = Endpoint_Read_8();
	_rx_buffer[21] = Endpoint_Read_8();
	_rx_buffer[22] = Endpoint_Read_8();
	_rx_buffer[23] = Endpoint_Read_8();
	_rx_buffer[24] = Endpoint_Read_8();
	_rx_buffer[25] = Endpoint_Read_8();
	_rx_buffer[26] = Endpoint_Read_8();
	_rx_buffer[27] = Endpoint_Read_8();
	_rx_buffer[28] = Endpoint_Read_8();
	_rx_buffer[29] = Endpoint_Read_8();
	_rx_buffer[30] = Endpoint_Read_8();
	_rx_buffer[31] = Endpoint_Read_8();
	_rx_buffer[32] = Endpoint_Read_8();
	_rx_buffer[33] = Endpoint_Read_8();
	_rx_buffer[34] = Endpoint_Read_8();
	_rx_buffer[35] = Endpoint_Read_8();
	_rx_buffer[36] = Endpoint_Read_8();
	_rx_buffer[37] = Endpoint_Read_8();
	_rx_buffer[38] = Endpoint_Read_8();
	_rx_buffer[39] = Endpoint_Read_8();
	_rx_buffer[40] = Endpoint_Read_8();
	_rx_buffer[41] = Endpoint_Read_8();
	_rx_buffer[42] = Endpoint_Read_8();
	_rx_buffer[43] = Endpoint_Read_8();
	_rx_buffer[44] = Endpoint_Read_8();
	_rx_buffer[45] = Endpoint_Read_8();
	_rx_buffer[46] = Endpoint_Read_8();
	_rx_buffer[47] = Endpoint_Read_8();
	_rx_buffer[48] = Endpoint_Read_8();
	_rx_buffer[49] = Endpoint_Read_8();
	_rx_buffer[50] = Endpoint_Read_8();
	_rx_buffer[51] = Endpoint_Read_8();
	_rx_buffer[52] = Endpoint_Read_8();
	_rx_buffer[53] = Endpoint_Read_8();
	_rx_buffer[54] = Endpoint_Read_8();
	_rx_buffer[55] = Endpoint_Read_8();
	_rx_buffer[56] = Endpoint_Read_8();
	_rx_buffer[57] = Endpoint_Read_8();
	_rx_buffer[58] = Endpoint_Read_8();
	_rx_buffer[59] = Endpoint_Read_8();
	_rx_buffer[60] = Endpoint_Read_8();
	_rx_buffer[61] = Endpoint_Read_8();
	_rx_buffer[62] = Endpoint_Read_8();
	_rx_buffer[63] = Endpoint_Read_8();

	Endpoint_ClearOUT();
	//USB_USBTask();
}

static inline int16_t usbio_get_byte(void) __attribute__ ((always_inline));
static inline int16_t usbio_get_byte(void) {
	//first 2 bytes of packet equal buffer size
	if (_rx_buffer_size == 0) {
		receive_rx_buffer();
		_rx_buffer_size = (_rx_buffer[0] << 8) | _rx_buffer[1];
		_rx_buffer_ix = 2;
	}
	else if (_rx_buffer_ix == RX_BUFFER_SIZE) {
		receive_rx_buffer();
		_rx_buffer_ix = 0;
	}
	
	if ((_rx_buffer_size > 0) && (_rx_buffer_ix < RX_BUFFER_SIZE)) {
		--_rx_buffer_size;
		++_rx_buffer_ix;
		return _rx_buffer[_rx_buffer_ix - 1];
	}
	
	return -1;
}

#endif //_USBIO_H_

/************************************************************************
  norflash.v - NOR flasher for PS3

Copyright (C) 2010-2011  Hector Martin "marcan" <hector@marcansoft.com>

This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*************************************************************************
 NORway.c (v0.6 beta) - Teensy++ 2.0 port by judges@eEcho.com
*************************************************************************/

#include <avr/io.h>
#include "nor.h"
#include "usbio.h"
#include "NANDORway.h"
// Define data ports
#define NOR_DATA1_PORT	PORTD
#define NOR_DATA1_PIN	PIND
#define NOR_DATA1_DDR	DDRD

#define NOR_DATA2_PORT	PORTC
#define NOR_DATA2_PIN	PINC
#define NOR_DATA2_DDR	DDRC

// Define address line ports
#define NOR_ADDR1_PORT	PORTF
#define NOR_ADDR1_PIN	PINF
#define NOR_ADDR1_DDR	DDRF

#define NOR_ADDR2_PORT	PORTA
#define NOR_ADDR2_PIN	PINA
#define NOR_ADDR2_DDR	DDRA

#define NOR_ADDR3_PORT	PORTB
#define NOR_ADDR3_PIN	PINB
#define NOR_ADDR3_DDR	DDRB

// Define control port and pins
#define NOR_CONT_PORT	PORTE
#define NOR_CONT_DDR	DDRE
#define NOR_CONT_PIN	PINE

#define NOR_CONT_CE		0b00000001		// 0: Chip Enable
#define NOR_CONT_OE		0b00000010		// 1: Output Enable
#define NOR_CONT_RESET	0b00010000		// 4: Reset
#define NOR_CONT_WE		0b00100000		// 5: Write Enable
#define NOR_CONT_RYBY	0b01000000		// 6: Ready/Busy
#define NOR_CONT_TRI	0b10000000		// 7: Tristate

static uint8_t _increment_address, _offset_2nddie;
static uint8_t _address1, _address2, _address3;

void nor_initports(void) {
	NOR_DATA1_DDR = NOR_DATA2_DDR = 0xFF;	// set for output
	NOR_ADDR1_DDR = NOR_ADDR2_DDR = NOR_ADDR3_DDR = 0xFF; //address ports are always output
	NOR_ADDR1_PORT = NOR_ADDR2_PORT = NOR_ADDR3_PORT = 0;

	NOR_CONT_DDR = 0xFF; //all control ports are always output

	NOR_CONT_DDR &= ~NOR_CONT_RYBY; //except RY/BY# (input)
	NOR_CONT_PORT |= NOR_CONT_RYBY; //enable pull up

	NOR_CONT_PORT &= ~(NOR_CONT_TRI | NOR_CONT_CE); //LOW
	NOR_CONT_PORT |= (NOR_CONT_WE | NOR_CONT_OE | NOR_CONT_RESET); //HIGH

	_offset_2nddie = 0;
	_address1 = _address2 = _address3 = 0;


/*	*(nand0.gpio_cont0_ddr) = 0xFF; 			// all control ports - output
	*(nand0.gpio_cont0_ddr) &= ~NAND_CONT_RB;	// ready / busy - input
	*(nand0.gpio_cont0_ddr) &= ~NAND_CONT_TRI;	// tri - input
	*(nand0.gpio_cont_port) = 0;				// all low
	*(nand0.gpio_cont_port) |= NAND_CONT_WE;
	*(nand0.gpio_cont_port) |= NAND_CONT_WP;

	*(nand1.gpio_cont0_ddr) = 0xFF; 			// all control ports - output
	*(nand1.gpio_cont0_ddr) &= ~NAND_CONT_RB;	// ready / busy - input
	*(nand1.gpio_cont0_ddr) &= ~NAND_CONT_TRI;	// tri - input
	*(nand1.gpio_cont_port) = 0;				// all low
	*(nand1.gpio_cont_port) |= NAND_CONT_WE;
	*(nand1.gpio_cont_port) |= NAND_CONT_WP;
*/
}

void nor_releaseports(void) {
	NOR_ADDR1_DDR = NOR_ADDR2_DDR = NOR_ADDR3_DDR = NOR_DATA1_DDR = NOR_DATA2_DDR = NOR_CONT_DDR = 0; //all ports are always input
	//NOR_ADDR1_PORT = NOR_ADDR2_PORT = NOR_ADDR3_PORT = NOR_DATA1_PORT = NOR_DATA2_PORT = NOR_CONT_PORT = 0; //disable pull ups for all ports
	NOR_ADDR1_PORT = NOR_ADDR2_PORT = NOR_ADDR3_PORT = NOR_DATA1_PORT = NOR_DATA2_PORT = NOR_CONT_PORT = 0xFF; //enable pull ups for all ports
}

void nor_address_set(const uint8_t address3, const uint8_t address2, const uint8_t address1) {
	_address3 = address3;
	_address2 = address2;
	_address1 = address1;
	NOR_ADDR3_PORT = address3;
	NOR_ADDR2_PORT = address2;
	NOR_ADDR1_PORT = address1;
}

void nor_address_increment_set(const uint8_t state) {
	_increment_address = state;
}

void nor_2nd_die_offset(const uint8_t offset) {
	_offset_2nddie = offset;
}

static inline void increment_address(const uint8_t lock_address) __attribute__ ((always_inline));
static inline void increment_address(const uint8_t lock_address) {
	++_address1;
	if (_address1 == 0) ++_address2;
	if ((_address1 == 0) && (_address2 == 0)) ++_address3;

	if (lock_address == 0) return;

	NOR_ADDR3_PORT = _address3; NOR_ADDR2_PORT = _address2; NOR_ADDR1_PORT = _address1;
}

void nor_address_increment(const uint8_t lock_address) {
	increment_address(lock_address);
}

static inline void set_address(const uint8_t address3, const uint8_t address2, const uint8_t address1) __attribute__ ((always_inline));
static inline void set_address(const uint8_t address3, const uint8_t address2, const uint8_t address1) {
	NOR_ADDR3_PORT = address3;
	NOR_ADDR2_PORT = address2;
	NOR_ADDR1_PORT = address1;
}

static inline void set_data(const uint8_t data2, const uint8_t data1) __attribute__ ((always_inline));
static inline void set_data(const uint8_t data2, const uint8_t data1) {
	NOR_DATA2_PORT = data2;
	NOR_DATA1_PORT = data1;
	_delay_ns(90);
	NOR_CONT_PORT &= ~NOR_CONT_WE; //LOW
	NOR_CONT_PORT |= NOR_CONT_WE; //HIGH
}

static inline uint8_t wait_for_ryby_short(void) __attribute__ ((always_inline));
static inline uint8_t wait_for_ryby_short(void) {
	//wait 200ns for RY/BY to become active
	_delay_ns(200);

	uint32_t cnt = 0x200000; //approx. 3secs
	while (cnt > 0) {
		if (NOR_CONT_PIN & NOR_CONT_RYBY) return 1;
		--cnt;
	}
	return 0;
}

static inline void wait_for_ryby_long(void) __attribute__ ((always_inline));
static inline void wait_for_ryby_long(void) {
	//wait 200ns for RY/BY to become active
	_delay_ns(200);

	while (1) {
		if (NOR_CONT_PIN & NOR_CONT_RYBY) break;
	}
}

void nor_reset(void) {
	NOR_CONT_PORT &= ~NOR_CONT_RESET; _delay_us(1);
	NOR_CONT_PORT |= NOR_CONT_RESET; _delay_us(1);
}

void nor_id(void) {
	NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	NOR_CONT_PORT &= ~NOR_CONT_CE; //LOW

	//two unlock cycles plus autoselect command
	set_address(0x00, 0x05, 0x55); set_data(0x00, 0xAA);
	set_address(0x00, 0x02, 0xAA); set_data(0x00, 0x55);
	set_address(0x00, 0x05, 0x55); set_data(0x00, 0x90);

	//manufacturer code
	set_address(0x00, 0x00, 0x00); nor_read(NOR_BSS_WORD, 0);

	//device code
	set_address(0x00, 0x00, 0x01); nor_read(NOR_BSS_WORD, 0);
	set_address(0x00, 0x00, 0x0E); nor_read(NOR_BSS_WORD, 0);
	set_address(0x00, 0x00, 0x0F); nor_read(NOR_BSS_WORD, 1);

	//reset required to exit autoselect command
	nor_reset();
}

void nor_erase_sector(void) {
	NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	NOR_CONT_PORT &= ~NOR_CONT_CE; //LOW

	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0xAA);
	set_address(_offset_2nddie, 0x02, 0xAA); set_data(0x00, 0x55);

	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0x80);
	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0xAA);

	set_address(_offset_2nddie, 0x02, 0xAA); set_data(0x00, 0x55);
	set_address(_address3, _address2, _address1); set_data(0x00, 0x30);

	wait_for_ryby_long();
}

void nor_erase_chip(void) {
	NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	NOR_CONT_PORT &= ~NOR_CONT_CE; //LOW

	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0xAA);
	set_address(_offset_2nddie, 0x02, 0xAA); set_data(0x00, 0x55);

	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0x80);
	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0xAA);

	set_address(_offset_2nddie, 0x02, 0xAA); set_data(0x00, 0x55);
	set_address(_offset_2nddie, 0x05, 0x55); set_data(0x00, 0x10);

	wait_for_ryby_long();
	nor_reset();
}

void nor_read(const uint32_t block_size, const uint8_t transmit) {
	NOR_DATA1_DDR = NOR_DATA2_DDR = 0x00; // set for input
	NOR_DATA1_PORT = NOR_DATA2_PORT = 0x00; //disable pull-ups

	if (block_size == NOR_BSS_WORD) {
		NOR_CONT_PORT &= ~NOR_CONT_OE; //LOW
		_delay_ns(90); //flash needs 90ns access time
		usbio_set_byte(NOR_DATA2_PIN, 0);
		usbio_set_byte(NOR_DATA1_PIN, transmit);
		NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	}
	else {
		uint8_t i;
		uint32_t addr = 0;

		/* Select the IN stream endpoint */
		Endpoint_SelectEndpoint(IN_EP);

		while (addr < block_size / 2) {
			/* Check if the current endpoint can be written to and that the next sample should be stored */
			while (!Endpoint_IsINReady()) USB_USBTask();

			for (i = 0; i < TX_BUFFER_SIZE / 2; ++i) {
				NOR_CONT_PORT &= ~NOR_CONT_OE; //LOW
				_delay_ns(90); //flash needs 90ns access time
				Endpoint_Write_8(NOR_DATA2_PIN);
				Endpoint_Write_8(NOR_DATA1_PIN);
				NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
				increment_address(1); ++addr;
			}

			Endpoint_ClearIN();
			//USB_USBTask();
		}
	}

	NOR_DATA1_DDR = NOR_DATA2_DDR = 0xFF;	// set for output
}

void nor_write_word(void) {
	NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	NOR_CONT_PORT &= ~(NOR_CONT_WE | NOR_CONT_CE); //LOW
	NOR_DATA2_PORT = usbio_get_byte();
	NOR_DATA1_PORT = usbio_get_byte();
	_delay_ns(90); //flash needs 90ns access time
	NOR_CONT_PORT |= NOR_CONT_WE; //HIGH
	if (_increment_address) increment_address(1);
}

void nor_write_block(const nor_prg_mode_t prg_mode) {
	uint32_t i;
	uint16_t k;
	uint8_t saddr1, saddr2, saddr3;
	uint8_t ryby_timeout = 0;

	NOR_CONT_PORT |= NOR_CONT_OE; //HIGH
	NOR_CONT_PORT &= ~NOR_CONT_CE; //LOW

	switch (prg_mode) {
	case NOR_PRG_MODE_WORD: //"single word program mode"
		/* Select the OUT stream endpoint */
		Endpoint_SelectEndpoint(OUT_EP);

		for (i = 0; i < NOR_BSS_8; i += RX_BUFFER_SIZE) {
			/* Check if the current endpoint can be read */
			while (!Endpoint_IsOUTReceived()) USB_USBTask();

			for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
				set_address(_offset_2nddie, 0x5, 0x55); set_data(0x0, 0xAA);
				set_address(_offset_2nddie, 0x2, 0xAA); set_data(0x0, 0x55);
				set_address(_offset_2nddie, 0x5, 0x55); set_data(0x0, 0xA0);
				set_address(_address3, _address2, _address1);
				set_data(Endpoint_Read_8(), Endpoint_Read_8());

				//increment address and wait for RY/BY
				increment_address(0);
				if (!wait_for_ryby_short()) {
					ryby_timeout = 1;
					++k;
					break;
				}
			}

			if (ryby_timeout) {
				for (;k < RX_BUFFER_SIZE / 2; ++k) {
					Endpoint_Read_8(); Endpoint_Read_8();
				}
				Endpoint_ClearOUT();
				i += RX_BUFFER_SIZE;
				break;
			}

			Endpoint_ClearOUT();
		}

		if (ryby_timeout) {
			for (; i < NOR_BSS_8; i += RX_BUFFER_SIZE) {
				/* Check if the current endpoint can be read */
				while (!Endpoint_IsOUTReceived()) USB_USBTask();

				for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
					Endpoint_Read_8(); Endpoint_Read_8();
				}

				Endpoint_ClearOUT();
			}

			usbio_set_byte(RES_TIMEOUT, 1); //if ry/by timeout, prepare to send FAIL!
			break;			//and exit case
		}

		usbio_set_byte(RES_SUCCESS, 1);
		break;

	case NOR_PRG_MODE_UBM: //"single word unlock bypass mode"
		/* Select the OUT stream endpoint */
		Endpoint_SelectEndpoint(OUT_EP);

		// enter unlock bypass mode
		set_address(_offset_2nddie, 0x5, 0x55); set_data(0x0, 0xAA);
		set_address(_offset_2nddie, 0x2, 0xAA); set_data(0x0, 0x55);
		set_address(_offset_2nddie, 0x5, 0x55); set_data(0x0, 0x20);
		set_address(_address3, _address2, _address1);

		for (i = 0; i < NOR_BSS_8; i += RX_BUFFER_SIZE) {
			/* Check if the current endpoint can be read */
			while (!Endpoint_IsOUTReceived()) USB_USBTask();

			for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
				set_data(0x0, 0xA0);
				set_data(Endpoint_Read_8(), Endpoint_Read_8());

				//increment address and wait for RY/BY
				increment_address(1);
				if (!wait_for_ryby_short()) {
					ryby_timeout = 1;
					++k;
					break;
				}
			}

			if (ryby_timeout) {
				for (;k < RX_BUFFER_SIZE / 2; ++k) {
					Endpoint_Read_8(); Endpoint_Read_8();
				}
				Endpoint_ClearOUT();
				i += RX_BUFFER_SIZE;
				break;
			}

			Endpoint_ClearOUT();
		}

		//exit unlock bypass mode
		set_data(0x0, 0x90);
		set_data(0x0, 0x0);

		if (ryby_timeout) {
			for (; i < NOR_BSS_8; i += RX_BUFFER_SIZE) {
				/* Check if the current endpoint can be read */
				while (!Endpoint_IsOUTReceived()) USB_USBTask();

				for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
					Endpoint_Read_8(); Endpoint_Read_8();
				}

				Endpoint_ClearOUT();
			}

			usbio_set_byte(RES_TIMEOUT, 1); //if ry/by timeout, prepare to send FAIL!
			break;			//and exit case
		}

		usbio_set_byte(RES_SUCCESS, 1); //ALL OK! prepare OK response
		break;

	case NOR_PRG_MODE_WBP: //"write buffer programming"
		/* Select the OUT stream endpoint */
		Endpoint_SelectEndpoint(OUT_EP);

		for (i = 0; i < NOR_BSS_32; i += RX_BUFFER_SIZE) {
			saddr1 = _address1; saddr2 = _address2; saddr3 = _address3;

			// enter write buffer programming mode
			set_address(0, 0x5, 0x55); set_data(0x0, 0xAA);
			set_address(0, 0x2, 0xAA); set_data(0x0, 0x55);
			set_address(saddr3, saddr2, saddr1); set_data(0x0, 0x25);
			set_address(saddr3, saddr2, saddr1); set_data(0x0, 0x1F);

			/* Check if the current endpoint can be read */
			while (!Endpoint_IsOUTReceived()) USB_USBTask();

			for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
				set_data(Endpoint_Read_8(), Endpoint_Read_8());
				increment_address(1);
			}

			Endpoint_ClearOUT();

			set_address(saddr3, saddr2, saddr1); set_data(0x0, 0x29);

			//wait for RY/BY
			if (!wait_for_ryby_short()) {
				ryby_timeout = 1;
				i += RX_BUFFER_SIZE;
				break;
			}
		}

		if (ryby_timeout) {
			for (; i < NOR_BSS_32; i += RX_BUFFER_SIZE) {
				/* Check if the current endpoint can be read */
				while (!Endpoint_IsOUTReceived()) USB_USBTask();

				for (k = 0; k < RX_BUFFER_SIZE / 2; ++k) {
					Endpoint_Read_8(); Endpoint_Read_8();
				}

				Endpoint_ClearOUT();
			}

			usbio_set_byte(RES_TIMEOUT, 1); //if ry/by timeout, prepare to send FAIL!
			break;			//and exit case
		}

		usbio_set_byte(RES_SUCCESS, 1); //ALL OK! prepare OK response
		break;

	default:
		break;
	}
}


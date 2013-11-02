/************************************************************************
 NANDORway.c (v1.0 beta) - Teensy++ 2.0 PS3/Xbox/Wii Flasher

 Copyright (C) 2013  judges@eEcho.com

 This code is licensed to you under the terms of the GNU GPL, version 2;
 see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*************************************************************************/

#define NANDORWAY_MAJOR_VERSION 1
#define NANDORWAY_MINOR_VERSION 0
#define NANDORWAY_BUILD_VERSION 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "usbio.h"
#include "delay_x.h"
#include "nor.h"
#include "nand.h"

enum {
	CMD_GETVERSION = 0,
	CMD_PING,
	CMD_BOOTLOADER,
	CMD_SPEEDTEST_READ,
	CMD_SPEEDTEST_WRITE,
	CMD_IO_LOCK,
	CMD_IO_RELEASE,
	CMD_NOR_ID,
	CMD_NOR_RESET,
	CMD_NOR_ERASE_SECTOR,
	CMD_NOR_ERASE_CHIP,
	CMD_NOR_ADDRESS_SET,
	CMD_NOR_ADDRESS_INCREMENT,
	CMD_NOR_ADDRESS_INCREMENT_ENABLE,
	CMD_NOR_ADDRESS_INCREMENT_DISABLE,
	CMD_NOR_2ND_DIE_ENABLE,
	CMD_NOR_2ND_DIE_DISABLE,
	CMD_NOR_READ_WORD,
	CMD_NOR_READ_BLOCK_4KB,
	CMD_NOR_READ_BLOCK_8KB,
	CMD_NOR_READ_BLOCK_64KB,
	CMD_NOR_READ_BLOCK_128KB,
	CMD_NOR_WRITE_WORD,
	CMD_NOR_WRITE_BLOCK_WORD,
	CMD_NOR_WRITE_BLOCK_UBM,
	CMD_NOR_WRITE_BLOCK_WBP,
	CMD_NAND_ID,
	CMD_NAND_ID_SET,
	CMD_NAND_BLOCK_SET,
	CMD_NAND_READ_BLOCK_ECC,
	CMD_NAND_READ_BLOCK_RAW,
} cmd_t;

//struct nand_driver_data nand0 = {
	//.info.t_adl = 100, .info.t_alh = 5, .info.t_als = 12, .info.t_rc = 25, .info.t_wh = 10, .info.t_wp = 12,
	//.gpio_ale = NAND_CONT_ALE, .gpio_ce = NAND_CONT_CE, .gpio_cle = NAND_CONT_CLE, .gpio_rb = NAND_CONT_RB, .gpio_re = NAND_CONT_RE, .gpio_we = NAND_CONT_WE, .gpio_wp = NAND_CONT_WP,
	//.gpio_cont0_ddr = &NAND0_CONT_DDR, .gpio_cont_pin = &NAND0_CONT_PIN, .gpio_cont_port = &NAND0_CONT_PORT,
	//.gpio_io_ddr = &NAND0_IO_DDR, .gpio_io_pin = &NAND0_IO_PIN, .gpio_io_port = &NAND0_IO_PORT
//};
struct nand_driver_data nand0 = {
	.info.t_adl = 100, .info.t_alh = 0, .info.t_als = 0, .info.t_rc = 0, .info.t_wh = 0, .info.t_wp = 0,
	.gpio_ale = NAND_CONT_ALE, .gpio_ce = NAND_CONT_CE, .gpio_cle = NAND_CONT_CLE, .gpio_rb = NAND_CONT_RB, .gpio_re = NAND_CONT_RE, .gpio_we = NAND_CONT_WE, .gpio_wp = NAND_CONT_WP,
	.gpio_cont_ddr = &NAND0_CONT_DDR, .gpio_cont_pin = &NAND0_CONT_PIN, .gpio_cont_port = &NAND0_CONT_PORT,
	.gpio_io_ddr = &NAND0_IO_DDR, .gpio_io_pin = &NAND0_IO_PIN, .gpio_io_port = &NAND0_IO_PORT
};
struct nand_driver_data nand1 = {
	.info.t_adl = 100, .info.t_alh = 0, .info.t_als = 0, .info.t_rc = 0, .info.t_wh = 0, .info.t_wp = 0,
	.gpio_ale = NAND_CONT_ALE, .gpio_ce = NAND_CONT_CE, .gpio_cle = NAND_CONT_CLE, .gpio_rb = NAND_CONT_RB, .gpio_re = NAND_CONT_RE, .gpio_we = NAND_CONT_WE, .gpio_wp = NAND_CONT_WP,
	.gpio_cont_ddr = &NAND1_CONT_DDR, .gpio_cont_pin = &NAND1_CONT_PIN, .gpio_cont_port = &NAND1_CONT_PORT,
	.gpio_io_ddr = &NAND1_IO_DDR, .gpio_io_pin = &NAND1_IO_PIN, .gpio_io_port = &NAND1_IO_PORT
};

void bootloader(void) {
	cli();
	// disable watchdog, if enabled
	// disable all peripherals
	UDCON = 1;
	USBCON = (1<<FRZCLK);  // disable USB
	UCSR1B = 0;
	_delay_ms(50);

	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK2 = 0; TIMSK3 = 0; UCSR1B = 0; TWCR = 0;
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;

	__asm volatile("jmp 0x1FC00");
}

// pure usb receive takes 16secs (1019KB/s) for 16MB
void speedtest_receive(void) {
	uint8_t k, byte;
	uint16_t i = 0;

	/* Select the OUT stream endpoint */
	Endpoint_SelectEndpoint(OUT_EP);

	while (i < NOR_BSS_32) {
		/* Check if the current endpoint can be read */
		while (!Endpoint_IsOUTReceived()) USB_USBTask();

		for (k = 0; k < RX_BUFFER_SIZE; ++k)
			byte = Endpoint_Read_8();

		Endpoint_ClearOUT();

		i += RX_BUFFER_SIZE;
	}
	usbio_set_byte('K', 1);
}

// pure usb transmit takes 15secs (1040KB/s) for 16MB
void speedtest_send(void) {
	uint8_t k;
	uint32_t i = 0;

	/* Select the IN stream endpoint */
	Endpoint_SelectEndpoint(IN_EP);

	while (i < NOR_BSS_128) {
		/* Check if the current endpoint can be written to and that the next sample should be stored */
		while (!Endpoint_IsINReady()) USB_USBTask();

		for (k = 0; k < TX_BUFFER_SIZE; ++k) {
			Endpoint_Write_8(0);
		}

		Endpoint_ClearIN();

		i += TX_BUFFER_SIZE;
	}
}


int32_t init_nand(struct nand_driver_data *nand) {
	int32_t retval;

	retval = nand_init(nand);
	if (retval) {
		//print_dbg("ERROR: failed to init NAND flash\r\n");
		return retval;
	}

	//block_status = malloc(nand->info.num_blocks);
	//if (!block_status) {
		////print_dbg("ERROR: could not allocate badblock table\r\n");
		//return -1;
	//}

	//nand->bad_table.block_status = block_status;

	//buffer = malloc(nand->info.page_size + nand->info.oob->size);
	//if (!buffer) {
		////print_dbg("ERROR: could not allocate buffer\r\n");
		//return -1;
	//}

	return 1;
}

void nand_initports(void) {
	*(nand0.gpio_cont_ddr) = 0xFF; 				// all control ports - output
	*(nand0.gpio_cont_ddr) &= ~NAND_CONT_RB;	// ready / busy - input
	*(nand0.gpio_cont_ddr) &= ~NAND_CONT_TRI;	// tri - input
	*(nand0.gpio_cont_port) = 0;				// all low
	*(nand0.gpio_cont_port) |= NAND_CONT_RB;	// enable pull-up for RB
	*(nand0.gpio_cont_port) |= NAND_CONT_WE;	// WE high
	*(nand0.gpio_cont_port) |= NAND_CONT_WP;	// WP high
	*(nand0.gpio_io_ddr) = 0x00;				// all i/o ports - input
	*(nand0.gpio_io_port) = 0x00;				// disable pull-ups

	*(nand1.gpio_cont_ddr) = 0xFF; 				// all control ports - output
	*(nand1.gpio_cont_ddr) &= ~NAND_CONT_RB;	// ready / busy - input
	*(nand1.gpio_cont_ddr) &= ~NAND_CONT_TRI;	// tri - input
	*(nand1.gpio_cont_port) = 0;				// all low
	*(nand1.gpio_cont_port) |= NAND_CONT_RB;	// enable pull-up for RB
	*(nand1.gpio_cont_port) |= NAND_CONT_WE;	// WE high
	*(nand1.gpio_cont_port) |= NAND_CONT_WP;	// WP high
	*(nand1.gpio_io_ddr) = 0x00;				// all i/o ports - input
	*(nand1.gpio_io_port) = 0x00;				// disable pull-ups

	//*(nand0.gpio_cont_ddr) = 0xFF; 				// all control ports - input
	//*(nand0.gpio_cont_port) = 0xFF;				// disable pull-ups
	//*(nand0.gpio_io_ddr) = 0xFF;				// all i/o ports - input
	//*(nand0.gpio_io_port) = 0xFF;				// disable pull-ups
//
	//*(nand1.gpio_cont_ddr) = 0xFF; 				// all control ports - input
	//*(nand1.gpio_cont_port) = 0xFF;				// disable pull-ups
	//*(nand1.gpio_io_ddr) = 0xFF;				// all i/o ports - input
	//*(nand1.gpio_io_port) = 0xFF;				// disable pull-ups
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void) {
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Set clock prescaler to 8 Mhz */
	clock_prescale_set(clock_div_2);

	/* TODO - disable JTAG to allow corresponding pins to be used - PF4, PF5, PF6, PF7 */
	#if ((defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
	defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
	defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__) ||  \
	defined(__AVR_ATmega32U6__)))
	// note the JTD bit must be written twice within 4 clock cycles to disable JTAG
	// you must also set the IVSEL bit at the same time, which requires IVCE to be set first
	// port pull-up resistors are enabled - PUD(Pull Up Disable) = 0
	MCUCR = (1 << JTD) | (1 << IVCE) | (0 << PUD);
	MCUCR = (1 << JTD) | (0 << IVSEL) | (0 << IVCE) | (0 << PUD);
	#endif

	//PRR0 = 0xFF;
	//PRR1 = 0x7F;
	//ACSR = 0x80;

	//set all i/o lines to input
	nor_releaseports();

	USB_Init();
}


//uint8_t buffer[2112];
//uint8_t block_status[1024];

int main(void)
{
	SetupHardware();
	sei();

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	while (USB_DeviceState != DEVICE_STATE_Configured)  /* wait */
		USB_USBTask();

	//configure all i/o lines (and set tristate=low)
	//nor_initports();
	nand_initports();

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	usbio_initbuffers();

	int16_t command = -1;
	uint8_t nand_id = 0;
	uint16_t nand_block = 0;

	while (1) {
		USB_USBTask();
		while (USB_DeviceState == DEVICE_STATE_Configured) { // is user still connected?
			command = usbio_get_byte();
			if (command == -1) continue;

			switch (command) {
			case CMD_GETVERSION:
				usbio_set_byte(NANDORWAY_MAJOR_VERSION, 0);
				usbio_set_byte(NANDORWAY_MINOR_VERSION, 0);
				usbio_set_byte(NANDORWAY_BUILD_VERSION, 1);
				break;
			case CMD_PING:
				usbio_set_byte(0x42, 0);
				usbio_set_byte(0xbd, 1);
				break;
			case CMD_BOOTLOADER:
				bootloader();
				break;
			case CMD_SPEEDTEST_READ:
				speedtest_send();
				break;
			case CMD_SPEEDTEST_WRITE:
				speedtest_receive();
				break;
			case CMD_IO_LOCK:
				nor_initports();
				break;
			case CMD_IO_RELEASE:
				nor_releaseports();
				break;
			case CMD_NOR_ID:
				nor_id();
				break;
			case CMD_NOR_RESET:
				nor_reset();
				break;
			case CMD_NOR_ERASE_SECTOR:
				nor_erase_sector();
				break;
			case CMD_NOR_ERASE_CHIP:
				nor_erase_chip();
				break;
			case CMD_NOR_ADDRESS_SET:
				nor_address_set(usbio_get_byte(), usbio_get_byte(), usbio_get_byte());
				break;
			case CMD_NOR_ADDRESS_INCREMENT:
				nor_address_increment(1);
				break;
			case CMD_NOR_ADDRESS_INCREMENT_ENABLE:
				nor_address_increment_set(1);
				break;
			case CMD_NOR_ADDRESS_INCREMENT_DISABLE:
				nor_address_increment_set(0);
				break;
			case CMD_NOR_2ND_DIE_ENABLE:
				nor_2nd_die_offset(0x40); //A22=HIGH for Samsung K8Q2815
				break;
			case CMD_NOR_2ND_DIE_DISABLE:
				nor_2nd_die_offset(0x00); //A22=LOW for Samsung K8Q2815
				break;
			case CMD_NOR_READ_WORD:
				nor_read(NOR_BSS_WORD, 1);
				break;
			case CMD_NOR_READ_BLOCK_4KB:
				nor_read(NOR_BSS_4, 1);
				break;
			case CMD_NOR_READ_BLOCK_8KB:
				nor_read(NOR_BSS_8, 1);
				break;
			case CMD_NOR_READ_BLOCK_64KB:
				nor_read(NOR_BSS_64, 1);
				break;
			case CMD_NOR_READ_BLOCK_128KB:
				nor_read(NOR_BSS_128, 1);
				break;
			case CMD_NOR_WRITE_WORD:
				nor_write_word();
				break;
			case CMD_NOR_WRITE_BLOCK_WORD:
				nor_write_block(NOR_PRG_MODE_WORD);
				break;
			case CMD_NOR_WRITE_BLOCK_UBM:
				nor_write_block(NOR_PRG_MODE_UBM);
				break;
			case CMD_NOR_WRITE_BLOCK_WBP:
				nor_write_block(NOR_PRG_MODE_WBP);
				break;
			case CMD_NAND_ID:
				switch (nand_id) {
				case 0:
					if (init_nand(&nand0) == 1) {
						//24 bytes
						usbio_set_byte(nand0.info.maker_code, 0);
						usbio_set_byte(nand0.info.device_code, 0);
						usbio_set_byte((nand0.info.page_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand0.info.page_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand0.info.page_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand0.info.page_size & 0xFF, 0);
						usbio_set_byte(nand0.info.oob->size, 0);
						usbio_set_byte((nand0.info.block_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand0.info.block_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand0.info.block_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand0.info.block_size & 0xFF, 0);
						usbio_set_byte((nand0.info.num_blocks >> 24) & 0xFF, 0);
						usbio_set_byte((nand0.info.num_blocks >> 16) & 0xFF, 0);
						usbio_set_byte((nand0.info.num_blocks >> 8) & 0xFF, 0);
						usbio_set_byte(nand0.info.num_blocks & 0xFF, 0);
						usbio_set_byte(nand0.info.num_planes, 0);
						usbio_set_byte((nand0.info.plane_size >> 56) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 48) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 40) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 32) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand0.info.plane_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand0.info.plane_size & 0xFF, 1);
					}
					else {
						for (uint8_t i = 0; i < 23; ++i) {
							usbio_set_byte(0xFF, 0);
						}
						usbio_set_byte(0xFF, 1);
					}
					break;
				case 1:
					if (init_nand(&nand1) == 1) {
						//24 bytes
						usbio_set_byte(nand1.info.maker_code, 0);
						usbio_set_byte(nand1.info.device_code, 0);
						usbio_set_byte((nand1.info.page_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand1.info.page_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand1.info.page_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand1.info.page_size & 0xFF, 0);
						usbio_set_byte(nand1.info.oob->size, 0);
						usbio_set_byte((nand1.info.block_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand1.info.block_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand1.info.block_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand1.info.block_size & 0xFF, 0);
						usbio_set_byte((nand1.info.num_blocks >> 24) & 0xFF, 0);
						usbio_set_byte((nand1.info.num_blocks >> 16) & 0xFF, 0);
						usbio_set_byte((nand1.info.num_blocks >> 8) & 0xFF, 0);
						usbio_set_byte(nand1.info.num_blocks & 0xFF, 0);
						usbio_set_byte(nand1.info.num_planes, 0);
						usbio_set_byte((nand1.info.plane_size >> 56) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 48) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 40) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 32) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 24) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 16) & 0xFF, 0);
						usbio_set_byte((nand1.info.plane_size >> 8) & 0xFF, 0);
						usbio_set_byte(nand1.info.plane_size & 0xFF, 1);
					}
					else {
						for (uint8_t i = 0; i < 23; ++i) {
							usbio_set_byte(0xFF, 0);
						}
						usbio_set_byte(0xFF, 1);
					}
					break;
				default:
					break;
				}
				break;
			case CMD_NAND_ID_SET:
				nand_id = usbio_get_byte();
				break;
			case CMD_NAND_BLOCK_SET:
				nand_block = (usbio_get_byte() << 8) | usbio_get_byte();
				break;
			case CMD_NAND_READ_BLOCK_RAW:
				switch (nand_id) {
				case 0:
					nand_read_block_raw(&nand0, nand_block);
					break;
				case 1:
					nand_read_block_raw(&nand1, nand_block);
					break;
				default:
					break;
				}


//				for (uint32_t offset = 0; offset < (nand0.info.block_size + (nand0.info.pages_per_block * nand0.info.oob->size)); offset += nand0.info.page_size + nand0.info.oob->size) {

					/*for (uint16_t buffer_ix = 0; buffer_ix < nand0.info.page_size + nand0.info.oob->size; buffer_ix += TX_BUFFER_SIZE) {
						while (!Endpoint_IsINReady())
							USB_USBTask();

						for (uint8_t i = 0; i < TX_BUFFER_SIZE; ++i) {
							Endpoint_Write_8(buffer[buffer_ix + i]);
						}

						Endpoint_ClearIN();
						//USB_USBTask();
					}*/
//				}


/*				for (uint32_t offset = 0; offset < nand0.info.block_size; offset += nand0.info.page_size) {
					nand_read_raw(&nand0, nand_block, offset, buffer, nand0.info.page_size);

					for (uint16_t buffer_ix = 0; buffer_ix < nand0.info.page_size; buffer_ix += TX_BUFFER_SIZE) {
						while (!Endpoint_IsINReady()) USB_USBTask();

						for (uint8_t i = 0; i < TX_BUFFER_SIZE; ++i) {
							Endpoint_Write_8(buffer[buffer_ix + i]);
						}

						Endpoint_ClearIN();
						//USB_USBTask();
					}
				}
*/
				break;
			default:
				break;
			}
		}
	}
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured.
 */
void EVENT_USB_Device_ConfigurationChanged(void) {
	Endpoint_ConfigureEndpoint(IN_EP_ADDR, EP_TYPE_BULK, IN_EP_SIZE, 2);
	Endpoint_ConfigureEndpoint(OUT_EP_ADDR, EP_TYPE_BULK,	OUT_EP_SIZE, 2);
}

/** Event handler for the USB_Connect event.
 */
/*
void EVENT_USB_Device_Connect(void) {
//TODO - handle device connect
}
*/

/** Event handler for the USB_Disconnect event.
 */
/*
void EVENT_USB_Device_Disconnect(void) {
//TODO - handle device disconnect
}
*/

/** Event handler for the library USB Control Request reception event. */
/*
void EVENT_USB_Device_ControlRequest(void) {
	// Process specific control requests
	switch (USB_ControlRequest.bRequest)
	{
		default: break;
	}
}
*/

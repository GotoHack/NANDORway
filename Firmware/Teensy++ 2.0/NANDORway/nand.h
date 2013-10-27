/* This header file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief NAND flash driver for AVR32 with local bus support or EBI.
 *
 * This file contains the driver for NAND flash driver.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a CPU local bus can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/* Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NAND_H
#define _NAND_H

#include <avr/io.h>

#define NAND0_IO_PORT 	PORTF
#define NAND0_IO_PIN	PINF
#define NAND0_IO_DDR	DDRF

#define NAND0_CONT_PORT PORTB
#define NAND0_CONT_PIN	PINB
#define NAND0_CONT_DDR	DDRB

#define NAND1_IO_PORT 	PORTC
#define NAND1_IO_PIN	PINC
#define NAND1_IO_DDR	DDRC

#define NAND1_CONT_PORT PORTD
#define NAND1_CONT_PIN	PIND
#define NAND1_CONT_DDR	DDRD

#define NAND_CONT_CE	0b00000001	// 0: Chip Enable
#define NAND_CONT_RE	0b00000010	// 1: Read Enable
#define NAND_CONT_CLE	0b00000100	// 2: Command Latch Enable
#define NAND_CONT_ALE	0b00001000	// 3: Address Latch Enable
#define NAND_CONT_WP	0b00010000	// 4: Write Protect
#define NAND_CONT_WE	0b00100000	// 5: Write Enable
#define NAND_CONT_RB	0b01000000	// 6: Ready/Busy (ready - high, busy - low)
#define NAND_CONT_TRI	0b10000000	// 7: Tristate (ready - high, busy - low)

/*! \brief FCPU must be configured to match the system configuration. */
//#define FCPU				8000000

/*! \brief Interface the flash using GPIO and CPU local bus. */
#define	NAND_BUS_TYPE_GPIO		1

/*! \brief Do not use error correction code when interfacing the flash. */
#define NAND_ECC_NONE			1
/*! \brief Use software error correction code when interfacing the flash. */
#define NAND_ECC_SW			2

/*! \brief Define to match the interface used. */
#define NAND_BUS_TYPE			NAND_BUS_TYPE_GPIO
/*! \brief Define to wanted ECC type. */
#define NAND_ECC_TYPE			NAND_ECC_SW

/*! \brief Define to enable marking bad blocks permanently in the NAND flash.
 *         This will write the bad block byte in the spare area for blocks
 *         where commands fail.
 */
/*#define NAND_MARK_BLOCKS_BAD		1*/
/*! \brief Define to enable the function to permanently marked bad blocks in
 *         the NAND flash. This will erase a block, even if marked bad.
 */
/*#define NAND_MARK_BLOCKS_OK		1*/

/*! \brief Define to enable always active chip enable (CE). */
#define NAND_CE_ALWAYS_ACTIVE	1

/*! \brief Nanoseconds per CPU tick. */
//#define NS_PER_TICK					(1000000 / (FCPU / 1000))

/*! \brief NAND flash driver commands. */
enum {
	NAND_CMD_READ	= 1,
	NAND_CMD_RANDOM_READ,
	NAND_CMD_ID,
	NAND_CMD_RESET,
	NAND_CMD_PROGRAM,
	NAND_CMD_RANDOM_PROGRAM,
	NAND_CMD_PROGRAM_COMPLETE,
	NAND_CMD_ERASE,
	NAND_CMD_STATUS,
} nand_cmd_t;

/*! \brief NAND flash driver return values. */
enum {
	SUCCESS = 0,
	/*! \brief I/O error return value. */
	EIO,
	/*! \brief Busy error return value. */
	EBUSY,
	/*! \brief Invalid argument error return value. */
	EINVAL,
	/*! \brief Timeout error return value. */
	ETIMEDOUT,
	/*! \brief ECC error return value. */
	ENANDECC_BADECC,
	/*! \brief Correctable ECC error return value. */
	ENANDECC_CORRECTABLE,
	/*! \brief Uncorrectable ECC error return value. */
	ENANDECC_UNCORRECTABLE,
} nand_return_values_t;

/*! \brief Struct for defining the spare area (out of bounce) in a page. */
struct nand_out_of_bounce_layout {
	/*! \brief Spare are total size. */
	uint8_t	size;
	/*! \brief Number of bytes for ECC data. */
	uint8_t	eccbytes;
	/*! \brief Array of offsets where ECC data can be stored. */
	uint8_t	eccpos[24];
};

/*! \brief NAND flash read page command start. */
#define NAND_COMMAND_READ1				0x00
/*! \brief NAND flash read page command end. */
#define NAND_COMMAND_READ2				0x30
/*! \brief NAND flash read ID command. */
#define NAND_COMMAND_READID				0x90
/*! \brief NAND flash reset command. */
#define NAND_COMMAND_RESET				0xFF
/*! \brief NAND flash program page command start. */
#define NAND_COMMAND_PAGEPROG1				0x80
/*! \brief NAND flash program page command end. */
#define NAND_COMMAND_PAGEPROG2				0x10
/*! \brief NAND flash erase block command start. */
#define NAND_COMMAND_ERASE1				0x60
/*! \brief NAND flash erase block command end. */
#define NAND_COMMAND_ERASE2				0xD0
/*! \brief NAND flash read status command. */
#define NAND_COMMAND_STATUS				0x70
/*! \brief NAND flash random program page command start. */
#define NAND_COMMAND_RANDOM_PAGEPROG			0x85
/*! \brief NAND flash random read page command start. */
#define NAND_COMMAND_RANDOM_READ1			0x05
/*! \brief NAND flash random read page command end. */
#define NAND_COMMAND_RANDOM_READ2			0xE0

/*! \brief Offset in spare area where bad block information is located for large page devices. */
#define NAND_LARGE_BAD_BLOCK_POSITION			0
/*! \brief Offset in spare area where bad block information is located for small page devices. */
#define NAND_SMALL_BAD_BLOCK_POSITION			5

/*! \brief NAND flash block ok. */
#define NAND_BLOCK_OK				0
/*! \brief NAND flash block bad and must not be used. */
#define NAND_BLOCK_BAD				1

/*! \brief NAND flash information about maker, device, size and timing.
*/
struct nand_info {
	/*! \brief Out of bounce layout information. */
	struct nand_out_of_bounce_layout *oob;

	/*! \brief Maker code. */
	uint8_t	maker_code;
	/*! \brief Device code. */
	uint8_t	device_code;

	/*! \brief Page size in bytes. */
	uint32_t	page_size;
	/*! \brief Number of positions to shift when converting an offset in
	 *         a block to page. Used when calculating NAND flash address.
	 */ 
	uint32_t	page_shift;
	/*! \brief Number of pages per block. */
	uint32_t	pages_per_block;
	/*! \brief Block size in bytes. */
	uint32_t	block_size;
	/*! \brief Number of positions to shift when converting block number
	 *         to NAND flash address.
	 */ 
	uint32_t	block_shift;
	/*! \brief Number of blocks. */
	uint32_t	num_blocks;
	/*! \brief NAND flash I/O bus width in bits. */
	uint32_t	bus_width;

	/*! \brief NAND flash number of planes. */
	uint8_t	num_planes;
	/*! \brief NAND flash plane size. */
	uint64_t 	plane_size;

	/*! \brief Offset to bad block position in page spare area. */
	uint8_t	badblock_offset;

	/*! \brief NAND flash WE low pulse time. */
	uint32_t	t_wp;
	/*! \brief NAND flash address to data loading time. */
	uint32_t	t_adl;
	/*! \brief NAND flash ALE hold time. */
	uint32_t	t_alh;
	/*! \brief NAND flash ALE setup timing. */
	uint32_t	t_als;
	/*! \brief NAND flash WE high hold time. */
	uint32_t	t_wh;
	/*! \brief NAND flash read cycle time. */
	uint32_t	t_rc;
};

/*! \brief Array of status on all blocks, defining which are bad.
*/
struct nand_bad_table {
	uint8_t				*block_status;
};

/*! \brief NAND flash driver data.
*/
struct nand_driver_data {
	struct nand_info		info;
	struct nand_bad_table		bad_table;

	/*! \brief GPIO line connected to NAND CE signal. */
	uint8_t				gpio_ce;
	/*! \brief GPIO line connected to NAND R/B signal. */
	uint8_t				gpio_rb;
	/*! \brief GPIO line connected to NAND RE signal. */
	uint8_t				gpio_re;
	/*! \brief GPIO line connected to NAND WE signal. */
	uint8_t				gpio_we;

	/*! \brief GPIO line connected to NAND WP signal, not mandatory. */
	uint8_t				gpio_wp;

	/*! \brief GPIO line connected to NAND ALE signal. */
	uint8_t				gpio_ale;
	/*! \brief GPIO line connected to NAND CLE signal. */
	uint8_t				gpio_cle;

	/*! \brief GPIO port number on CPU local bus used for I/O access. */
	volatile uint8_t *gpio_io_port;
	volatile uint8_t *gpio_io_pin;
	volatile uint8_t *gpio_io_ddr;
	
	volatile uint8_t *gpio_cont_port;
	volatile uint8_t *gpio_cont_pin;
	volatile uint8_t *gpio_cont_ddr;

	/*! \brief GPIO port mask on CPU local bus used for I/O access. */
	//uint64_t			gpio_io_mask;
	/*! \brief GPIO port address on CPU local bus used for I/O access. */
	//uint64_t			*gpio_io_address;
	/*! \brief GPIO port offset to first GPIO line used for I/O access. */
	//uint64_t			gpio_io_offset;
	/*! \brief GPIO port size used for I/O access, i.e. number of bits. */
	//uint64_t			gpio_io_size;
};

int32_t nand_init(struct nand_driver_data *nfd);

int32_t nand_create_badblocks_table(struct nand_driver_data *nfd);

int32_t nand_erase(struct nand_driver_data *nfd, const uint64_t block);
int32_t nand_read(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count);
int32_t nand_read_raw(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count);
int32_t nand_read_block_raw(struct nand_driver_data *nfd, const uint64_t block);
int32_t nand_write(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count);
int32_t nand_write_raw(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count);

#endif /* _NAND_H */

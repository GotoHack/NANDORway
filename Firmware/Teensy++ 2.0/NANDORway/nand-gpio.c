/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief NAND flash GPIO driver for AVR32 with local bus support.
 *
 * This file contains the driver for NAND flash GPIO driver.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a CPU local bus can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */
//#include "gpio.h"
#include <avr/io.h>
#include "delay_x.h"
#include "ecc.h"
//#include "compiler.h"
#include "clz_ctz.h"
#include "nand.h"
#include "nand-gpio.h"
#include "usbio.h"

#if NAND_ECC_TYPE == NAND_ECC_SW
#include "ecc-sw.h"
#endif

/*! \brief Spare area description for pages with 16 bytes OOB. */
static struct nand_out_of_bounce_layout nand_oob_16 = {
	.size		= 16,
	.eccbytes	= 6,
	.eccpos		= {
		0, 1, 2, 3, 6, 7,
	},
};

/*! \brief Spare area description for pages with 64 bytes OOB. */
static struct nand_out_of_bounce_layout nand_oob_64 = {
	.size		= 64,
	.eccbytes	= 24,
	.eccpos		= {
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63,
	},
};

static int32_t nand_gpio_command_failed(struct nand_driver_data *nfd);

static void nand_gpio_write_addr(struct nand_driver_data *nfd,
		const uint8_t addr);

static int32_t nand_gpio_reset(struct nand_driver_data *nfd);

static int32_t nand_gpio_read_id(struct nand_driver_data *nfd);

static void nand_gpio_block_markok(struct nand_driver_data *nfd,
		const uint64_t block);
#ifdef NAND_MARK_BLOCKS_OK
static int32_t nand_gpio_block_setok(struct nand_driver_data *nfd,
		const uint64_t block);
#endif
static void nand_gpio_block_markbad(struct nand_driver_data *nfd,
		const uint64_t block);
#ifdef NAND_MARK_BLOCKS_BAD
static int32_t nand_gpio_block_setbad(struct nand_driver_data *nfd,
		const uint64_t block);
#endif
static int32_t nand_gpio_block_checkbad(struct nand_driver_data *nfd,
		const uint64_t block);
static int32_t nand_gpio_block_isbad(struct nand_driver_data *nfd,
		const uint64_t block);

#if NAND_ECC_TYPE == NAND_ECC_SW
static int32_t nand_gpio_read_oob_ecc(struct nand_driver_data *nfd,
		uint64_t *ecc, const uint64_t block,
		const uint64_t offset);
static int32_t nand_gpio_write_oob_ecc(struct nand_driver_data *nfd,
		const uint64_t ecc, const uint64_t block,
		const uint64_t offset);
#endif

/*! \brief Delay function for busy waiting when timing the NAND flash signals.
 *
 *  This internal function is used to generate correct timing when sending
 *  commands, addresses and data to and from the NAND flash device.
 *
 *  \param ns Number of nanoseconds to delay.
 */
/*
static inline void ndelay(const uint64_t ns)
{
	cpu_delay_cy(ns / NS_PER_TICK + 1);
}
*/

/*! \brief Write an value to the GPIO local bus registers.
 *
 *  This internal function writes a value to the GPIO local bus registers in
 *  the AVR32 device. This is used to configure the output driver enable
 *  register and the output value registers.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param offset Offset from the base GPIO local bus address to write too.
 *  \param data Data to write to the offset address on the GPIO local bus.
 */
static inline void _nand_gpio_write_io(struct nand_driver_data *nfd, const uint8_t data)
{
	*(nfd->gpio_io_ddr) = 0xFF;
	*(nfd->gpio_io_port) = data;
}

static inline void gpio_enable_pin_output(volatile uint8_t *gpio_io_ddr, const uint8_t pin)
{
	*gpio_io_ddr |= pin;
}

static inline void gpio_enable_pin_input(volatile uint8_t *gpio_io_ddr, const uint8_t pin)
{
	*gpio_io_ddr &= ~pin;
}

static inline void gpio_enable_pin_pull_up(volatile uint8_t *gpio_io_port, const uint8_t pin)
{
	*gpio_io_port |= pin;
}

static inline void gpio_disable_pin_pull_up(volatile uint8_t *gpio_io_port, const uint8_t pin)
{
	*gpio_io_port &= ~pin;
}

static inline void gpio_clr_gpio_pin(volatile uint8_t *gpio_io_port, const uint8_t pin)
{
	*gpio_io_port &= ~pin;
}

static inline void gpio_set_gpio_pin(volatile uint8_t *gpio_io_port, const uint8_t pin)
{
	*gpio_io_port |= pin;
}

static inline uint8_t gpio_get_pin_value(volatile uint8_t *gpio_io_pin, const uint8_t pin)
{
	return *gpio_io_pin & pin;
}

/*! \brief Write an I/O value to the I/O port on the NAND flash.
 *
 *  This internal function writes a value to the I/O port on the NAND flash.
 *  It is mandatory to send appropriate NAND flash commands before trying to
 *  write any values.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \param data Data to write to the I/O port on the NAND flash.
 */
static inline void
nand_gpio_write_io(struct nand_driver_data *nfd, const uint8_t data)
{
	_nand_gpio_write_io(nfd, data);

	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);
	_delay_ns(t_wp);

	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);
	_delay_ns(t_wh);
}

/*! \brief Read an I/O value from the I/O port on the NAND flash.
 *
 *  This internal function reads a value from the I/O port on the NAND flash.
 *  It is mandatory to send appropriate NAND flash commands before trying to
 *  read out any values.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return I/O value read from the device.
 */
static inline uint8_t
nand_gpio_read_io(const struct nand_driver_data *nfd)
{
	static uint8_t byte;
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_re);
	_delay_ns(t_rc);
	
	byte = *(nfd->gpio_io_pin);

	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_re);
	
	return byte;
}

/*! \brief Wait for NAND flash R/B signal to go ready.
 *
 *  This function will wait for the R/B signal to go to the ready state. It
 *  will do a polled wait with the possibility for a timeout.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_wait_ready(struct nand_driver_data *nfd)
{
	/* Should be done within 3 milliseconds for all commands. */
	volatile uint64_t timeout = 0x200000; //approx. 3secs

	while (timeout > 0) {
		if (gpio_get_pin_value(nfd->gpio_cont_pin, nfd->gpio_rb)) return 0;
		--timeout;
	}

	return -ETIMEDOUT;
}

/*! \brief Write a command to the NAND flash device.
 *
 *  This function will write a command to the NAND flash device, it is used
 *  by the other functions internally in the NAND GPIO driver.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param command Command to write to the NAND flash device.
 */
static void _nand_gpio_write_cmd(struct nand_driver_data *nfd,
		const uint8_t command)
{
	_nand_gpio_write_io(nfd, command);
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_cle);
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);

	//_delay_ns(t_wp);

	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);

	//_delay_ns(t_alh);

	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_cle);
}

/*! \brief Write a command to the NAND flash device.
 *
 *  This function will write a command to the NAND flash device, it is used
 *  by the other functions internally in the NAND GPIO driver.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param command Command to write to the NAND flash device.
 *  \param addr Address part of the command if appropriate.
 *  \param offset Offset part of the address if appropriate.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_write_cmd(struct nand_driver_data *nfd,
		const uint8_t command, const uint64_t addr,
		const uint64_t offset)
{
	int32_t retval = 0;

	switch (command) {
	case NAND_CMD_RESET:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_RESET);
		retval = nand_gpio_wait_ready(nfd);
		return retval;
	case NAND_CMD_ID:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_READID);
		gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
		nand_gpio_write_addr(nfd, 0);
		gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
		return retval;
	case NAND_CMD_STATUS:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_STATUS);
		return retval;
	case NAND_CMD_READ:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_READ1);
		break;
	case NAND_CMD_RANDOM_READ:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_RANDOM_READ1);
		break;
	case NAND_CMD_PROGRAM:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_PAGEPROG1);
		break;
	case NAND_CMD_RANDOM_PROGRAM:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_RANDOM_PAGEPROG);
		break;
	case NAND_CMD_PROGRAM_COMPLETE:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_PAGEPROG2);
		retval = nand_gpio_wait_ready(nfd);
		return retval;
	case NAND_CMD_ERASE:
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_ERASE1);
		gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
		nand_gpio_write_addr(nfd, (addr)       & 0xff);
		nand_gpio_write_addr(nfd, (addr >> 8)  & 0xff);
		nand_gpio_write_addr(nfd, (addr >> 16) & 0xff);
		gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_ERASE2);
		retval = nand_gpio_wait_ready(nfd);
		return retval;
	default:
		return -EINVAL;
	}

	/* From here the command is either
	 * NAND_CMD_READ, NAND_CMD_RANDOM_READ or NAND_CMD_PROGRAM.
	 */
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
	nand_gpio_write_addr(nfd, (offset)      & 0xff);
	nand_gpio_write_addr(nfd, (offset >> 8) & 0xff);
	if (command == NAND_CMD_READ || command == NAND_CMD_PROGRAM) {
		nand_gpio_write_addr(nfd, (addr)       & 0xff);
		nand_gpio_write_addr(nfd, (addr >> 8)  & 0xff);
		nand_gpio_write_addr(nfd, (addr >> 16) & 0xff);
	}
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);

	if (command == NAND_CMD_READ) {
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_READ2);
	}
	else if (command == NAND_CMD_RANDOM_READ) {
		_nand_gpio_write_cmd(nfd, NAND_COMMAND_RANDOM_READ2);
	} else {
		return retval;
	}

	return nand_gpio_wait_ready(nfd);
}

/*! \brief Write an address to the NAND flash device.
 *
 *  This function will write an address to the NAND flash device, it is used
 *  by the other functions internally in the NAND GPIO driver.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param addr 8 bit part of an address to write to the NAND flash device.
 */
static void nand_gpio_write_addr(struct nand_driver_data *nfd,
		const uint8_t addr)
{
	_nand_gpio_write_io(nfd, addr);

	//gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);

	//_delay_ns(t_als);

	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);

	//_delay_ns(t_wh);

	//gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);
}

/*! \brief Configure the NAND GPIO driver.
 *
 *  This function will configure the GPIO lines to the NAND flash device,
 *  and call the \ref nand_gpio_reset and \ref nand_gpio_read_id functions.
 *
 *  After this function has been called the user must set up the
 *  block_status array in the nand_driver_data struct and call the
 *  \ref nand_create_badblocks_table function.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
int32_t nand_gpio_init(struct nand_driver_data *nfd)
{
	//int32_t i;
	int32_t retval;

	/* Configure GPIO for NAND device */
	gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_ce);
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
	
	gpio_enable_pin_input(nfd->gpio_cont_ddr, nfd->gpio_rb);
	gpio_enable_pin_pull_up(nfd->gpio_cont_port, nfd->gpio_rb);
	gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_re);
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_re);
	gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_we);
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_we);

	if (nfd->gpio_wp >= 0) {
		gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_wp);
		gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_wp);
		//gpio_enable_pin_input(nfd->gpio_cont_ddr, nfd->gpio_wp);
		//gpio_disable_pin_pull_up(nfd->gpio_cont_port, nfd->gpio_wp);
	}

	gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_ale);
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ale);

	gpio_enable_pin_output(nfd->gpio_cont_ddr, nfd->gpio_cle);
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_cle);

	/* Enable GPIO for NAND flash I/O port. */
	//gpio_enable_pin_output(nfd->gpio_io_ddr, 0xFF);
	//gpio_clr_gpio_pin(nfd->gpio_io_port, 0xFF);
	
	//for (i = 0; i < nfd->gpio_io_size; i++) {
		//gpio_enable_gpio_pin((32 * nfd->gpio_io_port) +
				//nfd->gpio_io_offset + i);
		//gpio_disable_pin_pull_up((32 * nfd->gpio_io_port) +
				//nfd->gpio_io_offset + i);
	//}

	retval = nand_gpio_reset(nfd);
	if (retval) {
		return retval;
	}

	retval = nand_gpio_read_id(nfd);
	if (retval) {
		return retval;
	}

	return 0;
}

/*! \brief Reset the NAND flash device.
 *
 *  This function resets the NAND flash device by sending the reset command.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_reset(struct nand_driver_data *nfd)
{
#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	if (nand_gpio_write_cmd(nfd, NAND_CMD_RESET, 0, 0)) {
		return -EBUSY;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return 0;
}


/*! \brief Read ID from NAND flash.
 *
 *  This function reads the ID from the NAND flash device. These ID bytes are
 *  used to fill in information about the flash producer, model, page size,
 *  block size, plane size, etc.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_read_id(struct nand_driver_data *nfd)
{
	uint32_t  spare_size;
	uint8_t maker_code;
	uint8_t device_code;
	uint8_t chip_data;
	uint8_t size_data;
	uint8_t plane_data;
	uint8_t plane_size;

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	nand_gpio_write_cmd(nfd, NAND_CMD_ID, 0, 0);

	/* Set GPIO I/O port in input mode. */
	//gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
	//gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);

	/* Set GPIO I/O port in input mode. */
	gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
	gpio_enable_pin_pull_up(nfd->gpio_io_port, 0xFF);

	maker_code = nand_gpio_read_io(nfd);
	device_code = nand_gpio_read_io(nfd);
	chip_data = nand_gpio_read_io(nfd);
	size_data = nand_gpio_read_io(nfd);
	plane_data = nand_gpio_read_io(nfd);

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	/* Fill the NAND structure parameters */
	nfd->info.maker_code = maker_code;
	nfd->info.device_code = device_code;
	nfd->info.page_size  = 0x400 << (size_data & 0x03);
	nfd->info.page_shift = ctz(nfd->info.page_size);
	nfd->info.block_size = (64UL * 1024UL) << ((size_data & 0x30) >> 4);

	nfd->info.num_planes = (1 << ((plane_data >> 2) & 0x03));

	plane_size = (plane_data >> 4) & 0x07;

	/* Store the plane size in bytes. */
	switch (plane_size) {
		case 0x0:
			nfd->info.plane_size = 1UL << 23;
			break;
		case 0x1:
			nfd->info.plane_size = 1UL << 24;
			break;
		case 0x2:
			nfd->info.plane_size = 1UL << 25;
			break;
		case 0x3:
			nfd->info.plane_size = 1UL << 26;
			break;
		case 0x4:
			nfd->info.plane_size = 1UL << 27;
			break;
		case 0x5:
			nfd->info.plane_size = 1UL << 28;
			break;
		case 0x6:
			nfd->info.plane_size = 1UL << 29;
			break;
		case 0x7:
			nfd->info.plane_size = 1UL << 30;
			break;
		default:
			return -EINVAL;
	}

	if ((size_data & 0x40) == 0) {
		nfd->info.bus_width = 8;
	} else {
		nfd->info.bus_width = 16;
	}

	nfd->info.num_blocks = nfd->info.num_planes * nfd->info.plane_size /
		nfd->info.block_size;
	nfd->info.pages_per_block = nfd->info.block_size / nfd->info.page_size;
	nfd->info.block_shift = ctz(nfd->info.pages_per_block);

	if (nfd->info.page_size >= 512) {
		nfd->info.badblock_offset = NAND_LARGE_BAD_BLOCK_POSITION;
	} else {
		nfd->info.badblock_offset = NAND_SMALL_BAD_BLOCK_POSITION;
	}

	spare_size = (8 << ((size_data & 0x04) >> 2)) *
			(nfd->info.page_size / 512);

	switch (spare_size) {
	case 16:
		nfd->info.oob = &nand_oob_16;
		break;
	case 64:
		nfd->info.oob = &nand_oob_64;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*! \brief Creates a bad blocks table.
 *
 *  This function creates a bad block table of the entire flash. It is vital
 *  that the nand_driver_data struct is proper set up before calling
 *  this function because it will assume the rather large block_status array
 *  is initialized.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
int32_t nand_gpio_create_badblocks_table(struct nand_driver_data *nfd)
{
	int32_t i;
	int32_t retval;
	uint64_t num_bad_blocks = 0;

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	for (i = 0; i < nfd->info.num_blocks; i++) {
		retval = nand_gpio_block_checkbad(nfd, i);
		if (retval != NAND_BLOCK_OK) {
			++num_bad_blocks;
			nand_gpio_block_markbad(nfd, i);
		}
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return num_bad_blocks;
}

/*! \brief Checks if a block in the bad blocks table is bad.
 *
 *  This function will check if a block in the bad blocks table is bad.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to check, starting at position 0.
 *
 *  \return 0 on ok block, an error number elsewise.
 */
static int32_t nand_gpio_block_isbad(struct nand_driver_data *nfd,
		const uint64_t block)
{
	if(nfd->bad_table.block_status[block] == NAND_BLOCK_BAD) {
		return -EIO;
	}

	return 0;
}

static void nand_gpio_block_markbad(struct nand_driver_data *nfd,
		const uint64_t block)
{
	nfd->bad_table.block_status[block] = NAND_BLOCK_BAD;
}

/*! \brief Marks a block in the bad blocks table bad.
 *
 *  This function will set a block to be bad in the bad blocks table.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to mark bad, starting at position 0.
 */
static void nand_gpio_block_markok(struct nand_driver_data *nfd,
		const uint64_t block)
{
	nfd->bad_table.block_status[block] = NAND_BLOCK_OK;
}

/*! \brief Checks if a block in NAND flash is bad.
 *
 *  This function will read the bad block byte in the NAND flash and check
 *  if the block is bad. This function is used by the
 *  \ref nand_gpio_create_badblocks_table.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to check, starting at position 0.
 *
 *  \return 0 on ok block, an error number elsewise.
 */
static int32_t nand_gpio_block_checkbad(struct nand_driver_data *nfd,
		const uint64_t block)
{
	int32_t retval;
	uint8_t tmp_read;
	uint64_t offset = nfd->info.page_size + nfd->info.badblock_offset;
	uint64_t addr = block << nfd->info.block_shift;

	if (nfd->info.bus_width == 16) {
		offset &= 0xfe;
	}

	/*
	 * Read first page into NAND flash cache. Byte offset in page
	 * (2 bytes) and page + block address (3 bytes).
	 *
	 * Jump to the bad block information location in this block and
	 * read a byte, should be 0xff.
	 */
	retval = nand_gpio_write_cmd(nfd, NAND_CMD_READ, addr, offset);
	if (retval) {
		return retval;
	}

	/* Set GPIO I/O port in input mode. */
	gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
	gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);

	tmp_read = nand_gpio_read_io(nfd);
	if (!(tmp_read & 0xff)) {
		nand_gpio_block_markbad(nfd, block);
		return NAND_BLOCK_BAD;
	} else {
		nand_gpio_block_markok(nfd, block);
	}

	/* Check second page as well if first page seems to be OK. */
	if (!nand_gpio_block_isbad(nfd, block)) {
		retval = nand_gpio_write_cmd(nfd, NAND_CMD_READ,
				addr + 1, offset);
		if (retval) {
			return retval;
		}

		/* Set GPIO I/O port in input mode. */
		gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
		gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);

		tmp_read = nand_gpio_read_io(nfd);
		if (!(tmp_read & 0xff)) {
			nand_gpio_block_markbad(nfd, block);
			return NAND_BLOCK_BAD;
		} else {
			nand_gpio_block_markok(nfd, block);
		}
	}

	return 0;
}

#ifdef NAND_MARK_BLOCKS_BAD
/*! \brief Set block in NAND flash to be bad.
 *
 *  This function will set a block in the NAND flash to be bad, this is used
 *  to mark bad blocks permanently in the NAND flash. Use with care.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to set ok, starting at position 0.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_block_setbad(struct nand_driver_data *nfd,
		const uint64_t block)
{
	int32_t retval;
	uint64_t addr = block << nfd->info.block_shift;
	uint64_t offset = nfd->info.page_size + nfd->info.badblock_offset;

	if (nfd->info.bus_width == 16) {
		offset &= 0xfe;
	}

	nand_gpio_block_markbad(nfd, block);

	retval = nand_gpio_erase(nfd, block);
	if (retval) {
		return retval;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM, addr, offset);
	if (retval) {
		return retval;
	}

	_delay_ns(t_adl);

	nand_gpio_write_io(nfd, 0);

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM_COMPLETE, 0, 0);
	if (retval) {
		return retval;
	}

	/* Check the write sequence operation. */
	if (nand_gpio_command_failed(nfd)) {
		return -EIO;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return 0;
}

#ifdef NAND_MARK_BLOCKS_OK
/*! \brief Set block in NAND flash to be ok.
 *
 *  This function will set a block in the NAND flash to be ok, this is used
 *  if a block has been faulty marked as a bad block. Use with care.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to set ok, starting at position 0.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_block_setok(struct nand_driver_data *nfd,
		const uint64_t block)
{
	int32_t retval = nand_gpio_erase(nfd, block);
	if (retval) {
		return retval;
	}

	return nand_gpio_block_checkbad(nfd, block);
}
#endif
#endif

#if NAND_ECC_TYPE == NAND_ECC_SW
/*! \brief Read ECC data from the spare area of the NAND flash.
 *
 *  This function reads ECC data from the spare area of the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param ecc Pointer to variable to put the read ECC data.
 *  \param block Block number for the related data, starting at position 0.
 *  \param offset Offset into the block for the related data.
 *
 *  \return Number of bytes read, an error number elsewise.
 */
int32_t nand_gpio_read_oob_ecc(struct nand_driver_data *nfd, uint64_t *ecc,
		const uint64_t block, const uint64_t offset)
{
	int32_t i;
	int32_t retval;
	int32_t ecc_offset;
	int32_t ecc_page_chunk;
	uint64_t col_addr;
	uint64_t row_addr;

	/* Known bad block? */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	/* Offset must be in ecc chunk size order. */
	if (offset % NAND_ECC_CHUNK_SIZE || block >= nfd->info.num_blocks
			|| offset >= nfd->info.block_size) {
		return -EINVAL;
	}

	/* Get position about where ECC for current chunk is. */
	ecc_page_chunk = (offset - ((offset / nfd->info.page_size)
			* nfd->info.page_size)) / NAND_ECC_CHUNK_SIZE;
	ecc_page_chunk *= NAND_ECC_BYTES_PER_CHUNK;
	ecc_offset = nfd->info.oob->eccpos[ecc_page_chunk];

	/* Initial col_addr must be page size + jump into ECC offset. */
	col_addr = nfd->info.page_size + ecc_offset;
	row_addr = (block << nfd->info.block_shift) +
			(offset >> nfd->info.page_shift);

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_READ, row_addr, col_addr);
	if (retval) {
		return retval;
	}

	*ecc = 0;

	/* Fill the Page Buffer with main area data. */
	for (i = 0; i < NAND_ECC_BYTES_PER_CHUNK; ) {
		/* Set GPIO I/O port in input mode. */
		gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
		gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);

		if (nfd->info.bus_width == 8) {
			*ecc |= nand_gpio_read_io(nfd) << (i * 8);
			i++;
		} else {
			*ecc |= nand_gpio_read_io(nfd) << (i * 16);
			i += 2;
		}

		/* Do we need to move the position in the read buffer? */
		if ((ecc_offset + (nfd->info.bus_width / 8))
				!= nfd->info.oob->eccpos[ecc_page_chunk
				               + (nfd->info.bus_width / 8)]) {
			/* Move read pointer. */
			col_addr = nfd->info.page_size
					+ nfd->info.oob->
						eccpos[ecc_page_chunk + i];
			retval = nand_gpio_write_cmd(nfd, NAND_CMD_RANDOM_READ,
					0, col_addr);
			if (retval) {
				return retval;
			}
		}

		ecc_page_chunk += nfd->info.bus_width / 8;
		ecc_offset = nfd->info.oob->eccpos[ecc_page_chunk];
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return nfd->info.oob->eccbytes;
}

/*! \brief Write ECC data into the spare area of the NAND flash.
 *
 *  This function writes ECC data into the spare area of the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param ecc ECC data to write into the spare area.
 *  \param block Block number for the related data, starting at position 0.
 *  \param offset Offset into the block for the related data.
 *
 *  \return Number of bytes written, an error number elsewise.
 */
int32_t nand_gpio_write_oob_ecc(struct nand_driver_data *nfd,
		const uint64_t ecc, const uint64_t block,
		const uint64_t offset)
{
	int32_t i;
	int32_t retval;
	int32_t ecc_offset;
	int32_t ecc_page_chunk;
	uint64_t col_addr;
	uint64_t row_addr;

	/* Known bad block? */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	/* Offset must be in ecc chunk size order. */
	if (offset % NAND_ECC_CHUNK_SIZE || block >= nfd->info.num_blocks
			|| offset >= nfd->info.block_size) {
		return -EINVAL;
	}

	/* Get position about where ECC for current chunk is. */
	ecc_page_chunk = (offset - ((offset / nfd->info.page_size)
			* nfd->info.page_size)) / NAND_ECC_CHUNK_SIZE;
	ecc_page_chunk *= NAND_ECC_BYTES_PER_CHUNK;
	ecc_offset = nfd->info.oob->eccpos[ecc_page_chunk];

	/* Initial col_addr must be page size + jump into ECC offset. */
	col_addr = nfd->info.page_size + ecc_offset;
	row_addr = (block << nfd->info.block_shift) +
			(offset >> nfd->info.page_shift);

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM, row_addr, col_addr);
	if (retval) {
		return retval;
	}

	_delay_ns(t_adl);

	/* Fill the Page Buffer with main area data. */
	for (i = 0; i < NAND_ECC_BYTES_PER_CHUNK; ) {
		if (nfd->info.bus_width == 8) {
			nand_gpio_write_io(nfd, (ecc >> (i * 8)) & 0xff);
			i++;
		} else {
			nand_gpio_write_io(nfd, (ecc >> (i * 16)) & 0xffff);
			i += 2;
		}

		/* Do we need to move the position in the read buffer? */
		if ((ecc_offset + (nfd->info.bus_width / 8))
				!= nfd->info.oob->eccpos[ecc_page_chunk
				               + (nfd->info.bus_width / 8)]) {
			/* Move read pointer. */
			col_addr = nfd->info.page_size + nfd->info.oob->
					eccpos[ecc_page_chunk + i];
			retval = nand_gpio_write_cmd(nfd, NAND_CMD_RANDOM_PROGRAM,
					0, col_addr);
			if (retval) {
				return retval;
			}
		}

		ecc_page_chunk += (nfd->info.bus_width / 8);
		ecc_offset = nfd->info.oob->eccpos[ecc_page_chunk];
	}

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM_COMPLETE, 0, 0);
	if (retval) {
		return retval;
	}

	/* Check the write sequence operation. */
	if (nand_gpio_command_failed(nfd)) {
#ifdef NAND_MARK_BLOCKS_BAD
		nand_gpio_block_setbad(nfd, block);
#else
		nand_gpio_block_markbad(nfd, block);
#endif
		return -EIO;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return nfd->info.oob->eccbytes;
}
#endif

/*! \brief Erase a block in the NAND flash.
 *
 *  This function erases the contents in a given block in the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to erase, starting at position 0.
 *
 *  \return 0 on success, an error number elsewise.
 */
int32_t nand_gpio_erase(struct nand_driver_data *nfd, const uint64_t block)
{
	int32_t retval;
	uint64_t addr;

	if (block >= nfd->info.num_blocks) {
		return -EINVAL;
	}

	/* Do NOT erase bad blocks. */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	addr = block << nfd->info.block_shift;

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_ERASE, addr, 0);
	if (retval) {
		return retval;
	}

	if (nand_gpio_command_failed(nfd)) {
#ifdef NAND_MARK_BLOCKS_BAD
		nand_gpio_block_setbad(nfd, block);
#else
		nand_gpio_block_markbad(nfd, block);
#endif
		return -EIO;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return 0;
}

/*! \brief Read data from the NAND flash.
 *
 *  This function reads data from the NAND flash at a given block and offset
 *  into a buffer.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to read from, starting at position 0.
 *  \param offset Offset into the block to read from, starting at position 0.
 *  \param buf Buffer for storing the read data.
 *  \param count Number of bytes to read.
 *
 *  \return Number of bytes read, an error number elsewise.
 */
int32_t nand_gpio_read(struct nand_driver_data *nfd,
		const uint64_t block, const uint64_t offset,
		uint8_t *buf, const uint16_t count)
{
	int32_t i;
	int32_t retval;
	uint64_t col_addr;
	uint64_t row_addr;
	//uint8_t *buf_char = buf;
	//uint16_t *buf_short = (uint16_t *)buf;
	//uint64_t end = offset - ((offset / nfd->info.page_size)
				//* nfd->info.page_size) + count;

	/* Known bad block? */
	//if(nand_gpio_block_isbad(nfd, block)) {
		//return -EIO;
	//}

	//if (end > nfd->info.page_size || block >= nfd->info.num_blocks
			//|| offset >= nfd->info.block_size) {
		//return -EINVAL;
	//}

	col_addr = offset % (nfd->info.page_size + nfd->info.oob->size);
	row_addr = (block << nfd->info.block_shift) +
		(offset >> nfd->info.page_shift);

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_READ, row_addr, col_addr);
	if (retval) {
		return retval;
	}

	/* Set GPIO I/O port in input mode. */
	gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
	gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);
	
	/* Fill the Page Buffer with main area data. */
	/*for (i = 0; i < count; ) {
		if (nfd->info.bus_width == 8) {
			*buf_char = nand_gpio_read_io(nfd);
			buf_char++;
			i++;
		} else {
			*buf_short = nand_gpio_read_io(nfd);
			buf_short++;
			i += 2;
		}
	}*/
	for (i = 0; i < count; i += TX_BUFFER_SIZE) {
		/* Check if the current endpoint can be written to and that the next sample should be stored */
		while (!Endpoint_IsINReady()) USB_USBTask();

		for (uint8_t tx = 0; tx < TX_BUFFER_SIZE; ++tx) {
			Endpoint_Write_8(nand_gpio_read_io(nfd));
		}

		Endpoint_ClearIN();
		//USB_USBTask();
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return count;
}

int32_t nand_gpio_read_block(struct nand_driver_data *nfd,
		const uint64_t block)
{
	//uint64_t col_addr;
	uint32_t row_addr;
	//uint8_t *buf_char = buf;
	//uint16_t *buf_short = (uint16_t *)buf;
	//uint64_t end = offset - ((offset / nfd->info.page_size)
				//* nfd->info.page_size) + count;

	/* Known bad block? */
	//if(nand_gpio_block_isbad(nfd, block)) {
		//return -EIO;
	//}

	//if (end > nfd->info.page_size || block >= nfd->info.num_blocks
			//|| offset >= nfd->info.block_size) {
		//return -EINVAL;
	//}

	uint8_t tx;
	uint16_t i;
	uint16_t page_size = nfd->info.page_size + nfd->info.oob->size;
	uint8_t error;
	//uint8_t sreg;
	
	//col_addr = offset % (nfd->info.page_size + nfd->info.oob->size);
	//row_addr = (block << nfd->info.block_shift) +
		//(offset >> nfd->info.page_shift);

	/* Select the IN stream endpoint */
	Endpoint_SelectEndpoint(IN_EP);

	/* Save global interrupt flag and disable interrupts */
	//sreg = SREG;
	//cli();

	for (uint16_t page = 0; page < nfd->info.pages_per_block; ++page) {
#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

		row_addr = (block << nfd->info.block_shift) + page;
		
		//_delay_us(10000); //4000
		error = 0;
		if (nand_gpio_write_cmd(nfd, NAND_CMD_READ, row_addr, 0) == -ETIMEDOUT) error = 1;

		/* Set GPIO I/O port in input mode. */
		gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
		gpio_enable_pin_pull_up(nfd->gpio_io_port, 0xFF);
		//gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
		//gpio_disable_pin_pull_up(nfd->gpio_io_port, 0xFF);
	
		//_delay_us(30);
		for (i = 0; i < page_size; i += TX_BUFFER_SIZE) {
			/* Check if the current endpoint can be written to and that the next sample should be stored */
			while (!Endpoint_IsINReady()) USB_USBTask();

			for (tx = 0; tx < TX_BUFFER_SIZE; ++tx) {
				if (error)
					Endpoint_Write_8(1);
				else
					Endpoint_Write_8(nand_gpio_read_io(nfd));
			}

			Endpoint_ClearIN();
		}
		
#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif
	}
	
	//SREG = sreg;

	return 0;
}

/*! \brief Write data to the NAND flash.
 *
 *  This function writes the content of a buffer to a given block and offset
 *  in the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to write to, starting at position 0.
 *  \param offset Offset into the block to write to, starting at position 0.
 *  \param buf Buffer with data to write.
 *  \param count Number of bytes to write.
 *
 *  \return Number of bytes written, an error number elsewise.
 */
int32_t nand_gpio_write(struct nand_driver_data *nfd,
		const uint64_t block, const uint64_t offset,
		const uint8_t *buf, const uint16_t count)
{
	int32_t i;
	int32_t retval;
	uint64_t col_addr;
	uint64_t row_addr;
	uint8_t *buf_char = (uint8_t *)buf;
	uint16_t *buf_short = (uint16_t *)buf;
	uint64_t end = offset - ((offset / nfd->info.page_size)
				* nfd->info.page_size) + count;

	/* Known bad block? */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	if (end > nfd->info.page_size || block >= nfd->info.num_blocks
			|| offset >= nfd->info.block_size) {
		return -EINVAL;
	}

	col_addr = offset % nfd->info.page_size;
	row_addr = (block << nfd->info.block_shift) +
		(offset >> nfd->info.page_shift);

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_clr_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM, row_addr, col_addr);

	_delay_ns(t_adl);

	/* GPIO I/O port is in output mode after previous function calls. */
	for (i = 0; i < count; ) {
		if (nfd->info.bus_width == 8) {
			nand_gpio_write_io(nfd, *buf_char++);
			i++;
		} else {
			nand_gpio_write_io(nfd, *buf_short++);
			i += 2;
		}
	}

	retval = nand_gpio_write_cmd(nfd, NAND_CMD_PROGRAM_COMPLETE, 0, 0);
	if (retval) {
		return retval;
	}

	/* Check the write sequence operation. */
	if (nand_gpio_command_failed(nfd)) {
#ifdef NAND_MARK_BLOCKS_BAD
		nand_gpio_block_setbad(nfd, block);
#else
		nand_gpio_block_markbad(nfd, block);
#endif
		return -EIO;
	}

#ifndef NAND_CE_ALWAYS_ACTIVE
	gpio_set_gpio_pin(nfd->gpio_cont_port, nfd->gpio_ce);
#endif

	return count;
}

#if NAND_ECC_TYPE == NAND_ECC_SW
/*! \brief Read data from the NAND flash with ECC.
 *
 *  This function reads data from the NAND flash at a given block and offset
 *  into a buffer.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to read from, starting at position 0.
 *  \param offset Offset into the block to read from, starting at position 0.
 *                Must be a size order of \ref NAND_ECC_CHUNK_SIZE.
 *  \param buf Buffer for storing the read data.
 *  \param count Number of bytes to read.
 *
 *  \return Number of bytes read, an error number elsewise.
 */
int32_t nand_gpio_read_ecc(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count)
{
	int32_t i;
	int32_t retval;
	uint64_t ecc_oob;
	uint64_t ecc_calc;
	uint64_t error_offset;
	uint8_t *buf_p = buf;
	uint8_t corrected_value;
	uint64_t end = offset - ((offset / nfd->info.page_size)
				* nfd->info.page_size) + count;

	/* Known bad block? */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	/* When doing reads with ECC we must handle it in chunks. */
	if (count % NAND_ECC_CHUNK_SIZE || end > nfd->info.page_size
			|| block >= nfd->info.num_blocks
			|| offset >= nfd->info.block_size) {
		return -EINVAL;
	}

	/* Fetch the data from the NAND flash device. */
	retval = nand_gpio_read(nfd, block, offset, buf, count);
	if (retval < 0) {
		return retval;
	}

	/* Calculate ECC in chunks and compare with flash. */
	for (i = 0; i < count / NAND_ECC_CHUNK_SIZE; i++) {
		ecc_make_block_256b(&ecc_calc, buf_p);
		nand_gpio_read_oob_ecc(nfd, &ecc_oob, block,
				(offset + (i * NAND_ECC_CHUNK_SIZE)));

		retval = ecc_compare_block_256b(&ecc_calc, &ecc_oob, buf_p,
				&error_offset, &corrected_value);
		if (retval == -ENANDECC_CORRECTABLE) {
			buf[(i * NAND_ECC_CHUNK_SIZE) + error_offset] =
				corrected_value;
		}
		else if (retval == -ENANDECC_UNCORRECTABLE) {
#ifdef NAND_MARK_BLOCKS_BAD
			nand_gpio_block_setbad(nfd, block);
#else
			nand_gpio_block_markbad(nfd, block);
#endif
			return retval;
		}
		else if (retval) {
			return retval;
		}

		buf_p += NAND_ECC_CHUNK_SIZE;
	}

	return count;
}

/*! \brief Write data to the NAND flash with ECC.
 *
 *  This function writes the content of a buffer to a given block and offset
 *  in the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *  \param block Block number to write to, starting at position 0.
 *  \param offset Offset into the block to write to, starting at position 0.
 *                Must be a size order of \ref NAND_ECC_CHUNK_SIZE.
 *  \param buf Buffer with data to write.
 *  \param count Number of bytes to write.
 *
 *  \return Number of bytes written, an error number elsewise.
 */
int32_t nand_gpio_write_ecc(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count)
{
	int32_t i;
	int32_t retval;
	uint64_t ecc = 0;
	uint8_t *buf_p = (uint8_t *)buf;
	uint64_t end = offset - ((offset / nfd->info.page_size)
				* nfd->info.page_size) + count;

	/* Known bad block? */
	if(nand_gpio_block_isbad(nfd, block)) {
		return -EIO;
	}

	/* When doing writes with ECC we must handle it in chunks. */
	if (count % NAND_ECC_CHUNK_SIZE || end > nfd->info.page_size
			|| block >= nfd->info.num_blocks
			|| offset >= nfd->info.block_size) {
		return -EINVAL;
	}

	/* Write the data to flash, ECC is added afterwards. */
	retval = nand_gpio_write(nfd, block, offset, buf, count);
	if (retval < 0) {
		return retval;
	}

	/* Calculate ECC in chunks and write to spare area. */
	for (i = 0; i < (count / NAND_ECC_CHUNK_SIZE); i++) {
		ecc_make_block_256b(&ecc, buf_p);

		retval = nand_gpio_write_oob_ecc(nfd, ecc, block,
				offset + (i * NAND_ECC_CHUNK_SIZE));
		if (retval < 0) {
			return retval;
		}

		buf_p += NAND_ECC_CHUNK_SIZE;
	}

	return count;
}
#endif

/*! \brief Check if last given command completed successfully.
 *
 *  This function reads the status from the NAND flash and checks if the
 *  last given command completed successfully.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND GPIO driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
static int32_t nand_gpio_command_failed(struct nand_driver_data *nfd)
{
	uint8_t cmd_status;

	nand_gpio_write_cmd(nfd, NAND_CMD_STATUS, 0, 0);

	/* Set GPIO I/O port in input mode. */
	gpio_enable_pin_input(nfd->gpio_io_ddr, 0xFF);
	gpio_enable_pin_pull_up(nfd->gpio_io_port, 0xFF);

	cmd_status = nand_gpio_read_io(nfd);

	if (!(cmd_status & 0x40)) {
		return -EBUSY;
	}
	else if ((cmd_status & 0x40) && (cmd_status & 0x01)) {
		return -EIO;
	}

	return 0;
}

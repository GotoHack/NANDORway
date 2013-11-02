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

#include "NANDORway.h"

#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
#include "nand-gpio.h"
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif

/*! \brief Configure the NAND driver.
 *
 *  This function will initialize and configure the NAND flash device.
 *
 *  After this function has been called the user must set up the
 *  block_status array in the nand_driver_data struct and call the
 *  \ref nand_create_badblocks_table function.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *
 *  \return 0 on success, an error number elsewise.
 */
int32_t nand_init(struct nand_driver_data *nfd)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_init(nfd);
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
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
int32_t nand_create_badblocks_table(struct nand_driver_data *nfd)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_create_badblocks_table(nfd);
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

/*! \brief Erase a block in the NAND flash.
 *
 *  This function erases the contents in a given block in the NAND flash.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *  \param block Block number to erase, starting at position 0.
 *
 *  \return 0 on success, an error number elsewise.
 */
int32_t nand_erase(struct nand_driver_data *nfd, const uint64_t block)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_erase(nfd, block);
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

/*! \brief Read data from the NAND flash.
 *
 *  This function reads data from the NAND flash at a given block and offset
 *  into a buffer. The read will be done with or without ECC depending on the
 *  configuration of the NAND driver \ref NAND_ECC_TYPE.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *  \param block Block number to read from, starting at position 0.
 *  \param offset Offset into the block to read from, starting at position 0.
 *  \param buf Buffer for storing the read data.
 *  \param count Number of bytes to read.
 *
 *  \return Number of bytes read, an error number elsewise.
 */
int32_t nand_read(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
#if NAND_ECC_TYPE == NAND_ECC_NONE
	return nand_gpio_read(nfd, block, offset, buf, count);
#elif NAND_ECC_TYPE == NAND_ECC_SW
	return nand_gpio_read_ecc(nfd, block, offset, buf, count);
#else
#error NAND_ECC_TYPE must be defined, see nand.h header file.
#endif
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

/*! \brief Read data from the NAND flash.
 *
 *  This function reads data from the NAND flash at a given block and offset
 *  into a buffer. The read will be done raw, without any ECC, even if ECC
 *  is enabled in the NAND driver.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *  \param block Block number to read from, starting at position 0.
 *  \param offset Offset into the block to read from, starting at position 0.
 *  \param buf Buffer for storing the read data.
 *  \param count Number of bytes to read.
 *
 *  \return Number of bytes read, an error number elsewise.
 */
int32_t nand_read_raw(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_read(nfd, block, offset, buf, count);
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

int32_t nand_read_block_raw(struct nand_driver_data *nfd, const uint64_t block)
{
	#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_read_block(nfd, block);
	#else
	#error NAND_BUS_TYPE must be defined, see nand.h header file.
	#endif
}

/*! \brief Write data to the NAND flash.
 *
 *  This function writes the content of a buffer to a given block and offset
 *  in the NAND flash. The write will be done with or without ECC depending on
 *  the configuration of the NAND driver \ref NAND_ECC_TYPE.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *  \param block Block number to write to, starting at position 0.
 *  \param offset Offset into the block to write to, starting at position 0.
 *  \param buf Buffer with data to write.
 *  \param count Number of bytes to write.
 *
 *  \return Number of bytes written, an error number elsewise.
 */
int32_t nand_write(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
#if NAND_ECC_TYPE == NAND_ECC_NONE
	return nand_gpio_write(nfd, block, offset, buf, count);
#elif NAND_ECC_TYPE == NAND_ECC_SW
	return nand_gpio_write_ecc(nfd, block, offset, buf, count);
#else
#error NAND_ECC_TYPE must be defined, see nand.h header file.
#endif
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

/*! \brief Write data to the NAND flash.
 *
 *  This function writes the content of a buffer to a given block and offset
 *  in the NAND flash. The write will be done raw, without any ECC, even if
 *  ECC is enabled in the NAND driver.
 *
 *  \param nfd Pointer to the nand_driver_data struct which
 *             holds all vital data about the NAND driver.
 *  \param block Block number to write to, starting at position 0.
 *  \param offset Offset into the block to write to, starting at position 0.
 *  \param buf Buffer with data to write.
 *  \param count Number of bytes to write.
 *
 *  \return Number of bytes written, an error number elsewise.
 */
int32_t nand_write_raw(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count)
{
#if NAND_BUS_TYPE == NAND_BUS_TYPE_GPIO
	return nand_gpio_write(nfd, block, offset, buf, count);
#else
#error NAND_BUS_TYPE must be defined, see nand.h header file.
#endif
}

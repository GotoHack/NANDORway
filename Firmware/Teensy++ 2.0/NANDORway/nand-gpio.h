/* This header file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

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

#ifndef _NAND_GPIO_H
#define _NAND_GPIO_H

#include "nand.h"

/*! \brief NAND flash WE low pulse time. */
#define t_wp	100
/*! \brief NAND flash address to data loading time. */
#define t_adl	100
/*! \brief NAND flash ALE hold time. */
#define t_alh	100
/*! \brief NAND flash ALE setup timing. */
#define t_als	100
/*! \brief NAND flash WE high hold time. */
#define t_wh	100
/*! \brief NAND flash read cycle time. */
#define t_rc	100

int32_t nand_gpio_init(struct nand_driver_data *nfd);

int32_t nand_gpio_create_badblocks_table(struct nand_driver_data *nfd);

int32_t nand_gpio_erase(struct nand_driver_data *nfd, const uint64_t block);

int32_t nand_gpio_read(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count);

int32_t nand_gpio_read_block(struct nand_driver_data *nfd,
		const uint64_t block);

int32_t nand_gpio_write(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count);

#if NAND_ECC_TYPE == NAND_ECC_SW
int32_t nand_gpio_read_ecc(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, uint8_t *buf,
		const uint16_t count);

int32_t nand_gpio_write_ecc(struct nand_driver_data *nfd, const uint64_t block,
		const uint64_t offset, const uint8_t *buf,
		const uint16_t count);
#endif

#endif /* _NAND_GPIO_H */

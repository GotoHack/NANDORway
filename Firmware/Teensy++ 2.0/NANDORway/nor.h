/** \file
 *
 *  Header file for nor.c.
 */
 
#ifndef _NOR_H_
#define _NOR_H_

/* Includes: */
// Define block/sector size for reading/writing
#define NOR_BSS_4		0x01000	//2Kwords = 4KB
#define NOR_BSS_8		0x02000	//4Kwords = 8KB
#define NOR_BSS_32		0x08000	//16Kwords = 32KB
#define NOR_BSS_64		0x10000	//32Kwords = 64KB
#define NOR_BSS_128		0x20000	//64Kwords = 128KB
#define NOR_BSS_WORD	0x00002	//word = 2Bytes

typedef enum {
	NOR_PRG_MODE_WORD = 0,
	NOR_PRG_MODE_UBM,
	NOR_PRG_MODE_WBP,
} nor_prg_mode_t;

/* Function Prototypes: */
void nor_initports(void);
void nor_releaseports(void);
void nor_reset(void);
void nor_id(void);
void nor_erase_sector(void);
void nor_erase_chip(void);
void nor_read(const uint32_t block_size, const uint8_t transmit);
void nor_write_word(void);
void nor_write_block(const nor_prg_mode_t prg_mode);
void nor_address_set(const uint8_t address3, const uint8_t address2, const uint8_t address1);
void nor_address_increment(const uint8_t lock_address);
void nor_address_increment_set(const uint8_t enable);
void nor_2nd_die_offset(const uint8_t offset);
		
#endif //_NOR_H_

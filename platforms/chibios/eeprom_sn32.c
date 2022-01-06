/*
 * This software is experimental and a work in progress.
 * Under no circumstances should these files be used in relation to any critical system(s).
 * Use of these files is at your own risk.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This files are free to use from http://engsta.com/stm32-flash-memory-eeprom-emulator/ by
 * Artur F.
 *
 * Modifications for QMK and STM32F303 by Yiancar
 * Adapted for SONIX chips by dexter93 and gloryhzw
 */

#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "Flash.h"
#include "eeprom_sn32.h"

/*****************************************************************************
 * Allows to use the internal flash to store non volatile data. To initialize
 * the functionality use the EEPROM_Init() function. Be sure that by reprogramming
 * of the controller just affected pages will be deleted. In other case the non
 * volatile data will be lost.
 ******************************************************************************/
#include "eeprom_sn32_defs.h"
#if !defined(FEE_PAGE_SIZE) || !defined(FEE_TOTAL_PAGES) || !defined(FEE_DENSITY_PAGES) || !defined(FEE_MCU_FLASH_SIZE) || !defined(FEE_PAGE_BASE_ADDRESS)
#    error "not implemented."
#endif

#define FEE_DENSITY_BYTES (FEE_PAGE_SIZE * FEE_DENSITY_PAGES - 1)
#define FEE_LAST_PAGE_ADDRESS (FEE_PAGE_BASE_ADDRESS + (FEE_PAGE_SIZE * FEE_DENSITY_PAGES))
/* Flash word value after erase */
#define FEE_EMPTY_WORD ((uint16_t)0xFFFF)

#define FEE_ADDR_OFFSET(Address) (Address)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Functions -----------------------------------------------------------------*/
/*
 * Debug print utils
 */

#if defined(DEBUG_EEPROM_OUTPUT)

#    define debug_eeprom debug_enable
#    define eeprom_println(s) println(s)
#    define eeprom_printf(fmt, ...) xprintf(fmt, ##__VA_ARGS__);

#else /* NO_DEBUG */

#    define debug_eeprom false
#    define eeprom_println(s)
#    define eeprom_printf(fmt, ...)

#endif /* NO_DEBUG */

/*****************************************************************************
 *  Delete Flash Space used for user Data, deletes the whole space between
 *  RW_PAGE_BASE_ADDRESS and the last uC Flash Page
 ******************************************************************************/
uint16_t EEPROM_Init(void) {

    // Clear Flags
    // SN_FLASH->STATUS &= ~FLASH_PGERR
    if (debug_eeprom) {
        println("EEPROM_Init");
    }

    return FEE_DENSITY_BYTES;
}

/*****************************************************************************
 *  Erase the whole reserved Flash Space used for user Data
 ******************************************************************************/
void EEPROM_Erase(void) {
    int page_num = 0;

    // delete all pages from specified start page to the last page
    do {
        eeprom_println("EEPROM_Erase");
        FLASH_EraseSector(FEE_PAGE_BASE_ADDRESS + (page_num * FEE_PAGE_SIZE));
        page_num++;
    } while (page_num < FEE_DENSITY_PAGES);
}

/*****************************************************************************
 *  Writes data to flash on specified address.
 *******************************************************************************/
uint8_t EEPROM_WriteDataByte(uint16_t Address, uint8_t DataByte) {
    FLASH_Status FlashStatus = FLASH_OKAY;

    /* if the address is out-of-bounds, do nothing */
    if (Address > FEE_DENSITY_BYTES) {
        eeprom_printf("EEPROM_WriteDataByte(0x%04x, 0x%02x) [BAD ADDRESS]\n", Address, DataByte);
        return FLASH_FAIL;
    }

    /* if the value is the same, don't bother writing it */
    if (DataByte == *(__IO uint8_t *)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address))) {
        eeprom_printf("EEPROM_WriteDataByte(0x%04x, 0x%02x) [SKIP SAME]\n", Address, DataByte);
        return FLASH_OKAY;
    }
 
    uint32_t addr = FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address);

    // update the data byte aligned in a 32-bit dword
    uint32_t value = *((uint32_t*)(addr & 0xFFFFFFFC));
    uint8_t* v8 = (uint8_t*)&value;
    v8[addr & 3] = DataByte;

    // program the 32-bit dword
    eeprom_printf("FLASH_ProgramDWord(0x%08x, 0x%04x) [DIRECT]\n", Address, value);
    FlashStatus = FLASH_ProgramDWord(addr & 0xFFFFFFFC, value);

    return FlashStatus;
}

uint8_t EEPROM_WriteDataWord(uint16_t Address, uint16_t DataWord) {
    /* if the address is out-of-bounds, do nothing */
    if (Address > FEE_DENSITY_BYTES) {
        eeprom_printf("EEPROM_WriteDataWord(0x%04x, 0x%04x) [BAD ADDRESS]\n", Address, DataWord);
        return FLASH_FAIL;
    }

    /* Check for word alignment */
    FLASH_Status final_status = FLASH_OKAY;
    if (Address % 2) {
        final_status        = EEPROM_WriteDataByte(Address, DataWord);
        FLASH_Status status = EEPROM_WriteDataByte(Address + 1, DataWord >> 8);
        if (status != FLASH_OKAY) final_status = status;
        if (final_status != 0 && final_status != FLASH_OKAY) {
            eeprom_printf("EEPROM_WriteDataWord [STATUS == %d]\n", final_status);
        }
        return final_status;
    }

    /* if the value is the same, don't bother writing it */
    uint8_t storedData = EEPROM_ReadDataByte(Address);
    uint16_t oldValue = *(uint16_t *)(&storedData);
    if (oldValue == DataWord) {
        eeprom_printf("EEPROM_WriteDataWord(0x%04x, 0x%04x) [SKIP SAME]\n", Address, DataWord);
        return 0;
    }

    if (final_status != 0 && final_status != FLASH_OKAY) {
        eeprom_printf("EEPROM_WriteDataWord [STATUS == %d]\n", final_status);
    }

    return final_status;
}

/*****************************************************************************
 *  Read data from a specified address.
 *******************************************************************************/
uint8_t EEPROM_ReadDataByte(uint16_t Address) {
    uint8_t DataByte = 0xFF;

    if (Address <= FEE_DENSITY_BYTES) {
        // Get Byte from specified address
        DataByte = (*(__IO uint8_t *)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address)));
    }

    eeprom_printf("EEPROM_ReadDataByte(0x%04x): 0x%02x\n", Address, DataByte);
    
    return DataByte;
}

uint16_t EEPROM_ReadDataWord(uint16_t Address) {
    uint16_t DataWord = 0xFFFF;

    if (Address <= FEE_DENSITY_BYTES) {
        /* Check word alignment */
        if (Address % 2) {
            DataWord = EEPROM_ReadDataByte(Address) | (EEPROM_ReadDataByte(Address + 1) << 8);
        } else {
            uint8_t storedData = EEPROM_ReadDataByte(Address);
            DataWord = *(uint16_t *)(&storedData);
        }
    }

    eeprom_printf("EEPROM_ReadDataWord(0x%04x): 0x%04x\n", Address, DataWord);

    return DataWord;
}

/*****************************************************************************
 *  Bind to eeprom_driver.c
 *******************************************************************************/
void eeprom_driver_init(void) { EEPROM_Init(); }

void eeprom_driver_erase(void) { EEPROM_Erase(); }

void eeprom_read_block(void *buf, const void *addr, size_t len) {
    const uint8_t *src  = (const uint8_t *)addr;
    uint8_t *      dest = (uint8_t *)buf;

    /* Check word alignment */
    if (len && (uintptr_t)src % 2) {
        /* Read the unaligned first byte */
        *dest++ = EEPROM_ReadDataByte((const uintptr_t)src++);
        --len;
    }

    uint16_t value;
    bool     aligned = ((uintptr_t)dest % 2 == 0);
    while (len > 1) {
        value = EEPROM_ReadDataWord((const uintptr_t)((uint16_t *)src));
        if (aligned) {
            *(uint16_t *)dest = value;
            dest += 2;
        } else {
            *dest++ = value;
            *dest++ = value >> 8;
        }
        src += 2;
        len -= 2;
    }
    if (len) {
        *dest = EEPROM_ReadDataByte((const uintptr_t)src);
    }
}

void eeprom_write_block(const void *buf, void *addr, size_t len) {
    uint8_t *      dest = (uint8_t *)addr;
    const uint8_t *src  = (const uint8_t *)buf;

    /* Check word alignment */
    if (len && (uintptr_t)dest % 2) {
        /* Write the unaligned first byte */
        EEPROM_WriteDataByte((uintptr_t)dest++, *src++);
        --len;
    }

    uint16_t value;
    bool     aligned = ((uintptr_t)src % 2 == 0);
    while (len > 1) {
        if (aligned) {
            value = *(uint16_t *)src;
        } else {
            value = *(uint8_t *)src | (*(uint8_t *)(src + 1) << 8);
        }
        EEPROM_WriteDataWord((uintptr_t)((uint16_t *)dest), value);
        dest += 2;
        src += 2;
        len -= 2;
    }

    if (len) {
        EEPROM_WriteDataByte((uintptr_t)dest, *src);
    }
}

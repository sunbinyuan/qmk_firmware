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

/*****************************************************************************
 *  Delete Flash Space used for user Data, deletes the whole space between
 *  RW_PAGE_BASE_ADDRESS and the last uC Flash Page
 ******************************************************************************/
uint16_t EEPROM_Init(void) {

    // Clear Flags
    // SN_FLASH->STATUS &= ~FLASH_PGERR

    return FEE_DENSITY_BYTES;
}

/*****************************************************************************
 *  Erase the whole reserved Flash Space used for user Data
 ******************************************************************************/
void EEPROM_Erase(void) {
    int page_num = 0;

    // delete all pages from specified start page to the last page
    do {
        FLASH_EraseSector(FEE_PAGE_BASE_ADDRESS + (page_num * FEE_PAGE_SIZE));
        page_num++;
    } while (page_num < FEE_DENSITY_PAGES);
}

/*****************************************************************************
 *  Writes once data byte to flash on specified address. If a byte is already
 *  written, the whole page must be copied to a buffer, the byte changed and
 *  the manipulated buffer written after PageErase.
 *******************************************************************************/
uint16_t EEPROM_WriteDataByte(uint16_t Address, uint8_t DataByte) {
    FLASH_Status FlashStatus = FLASH_OKAY;

    // exit if desired address is above the limit (e.G. under 2048 Bytes for 4 pages)
    if (Address > FEE_DENSITY_BYTES) {
        return FLASH_FAIL;
    }

    // check if new data is differ to current data, return if not, proceed if yes
    if (DataByte == *(__IO uint8_t *)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address))) {
        return FLASH_OKAY;
    }
 
    uint32_t addr = FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address);

    // update the data byte aligned in a 32-bit dword
    uint32_t value = *((uint32_t*)(addr & 0xFFFFFFFC));
    uint8_t* v8 = (uint8_t*)&value;
    v8[addr & 3] = DataByte;

    // program the 32-bit dword
    FlashStatus = FLASH_ProgramDWord(addr & 0xFFFFFFFC, value);

    return FlashStatus;
}

/*****************************************************************************
 *  Read once data byte from a specified address.
 *******************************************************************************/
uint8_t EEPROM_ReadDataByte(uint16_t Address) {
    uint8_t DataByte = 0xFF;

    // Get Byte from specified address
    DataByte = (*(__IO uint8_t *)(FEE_PAGE_BASE_ADDRESS + FEE_ADDR_OFFSET(Address)));

    return DataByte;
}

/*****************************************************************************
 *  Wrap library in AVR style functions.
 *******************************************************************************/
uint8_t eeprom_read_byte(const uint8_t *Address) {
    const uint16_t p = (const uint32_t)Address;
    return EEPROM_ReadDataByte(p);
}

void eeprom_write_byte(uint8_t *Address, uint8_t Value) {
    uint16_t p = (uint32_t)Address;
    EEPROM_WriteDataByte(p, Value);
}

void eeprom_update_byte(uint8_t *Address, uint8_t Value) {
    uint16_t p = (uint32_t)Address;
    EEPROM_WriteDataByte(p, Value);
}

uint16_t eeprom_read_word(const uint16_t *Address) {
    const uint16_t p = (const uint32_t)Address;
    return EEPROM_ReadDataByte(p) | (EEPROM_ReadDataByte(p + 1) << 8);
}

void eeprom_write_word(uint16_t *Address, uint16_t Value) {
    uint16_t p = (uint32_t)Address;
    EEPROM_WriteDataByte(p, (uint8_t)Value);
    EEPROM_WriteDataByte(p + 1, (uint8_t)(Value >> 8));
}

void eeprom_update_word(uint16_t *Address, uint16_t Value) {
    uint16_t p = (uint32_t)Address;
    EEPROM_WriteDataByte(p, (uint8_t)Value);
    EEPROM_WriteDataByte(p + 1, (uint8_t)(Value >> 8));
}

uint32_t eeprom_read_dword(const uint32_t *Address) {
    const uint16_t p = (const uint32_t)Address;
    return EEPROM_ReadDataByte(p) | (EEPROM_ReadDataByte(p + 1) << 8) | (EEPROM_ReadDataByte(p + 2) << 16) | (EEPROM_ReadDataByte(p + 3) << 24);
}

void eeprom_write_dword(uint32_t *Address, uint32_t Value) {
    uint16_t p = (const uint32_t)Address;
    EEPROM_WriteDataByte(p, (uint8_t)Value);
    EEPROM_WriteDataByte(p + 1, (uint8_t)(Value >> 8));
    EEPROM_WriteDataByte(p + 2, (uint8_t)(Value >> 16));
    EEPROM_WriteDataByte(p + 3, (uint8_t)(Value >> 24));
}

void eeprom_update_dword(uint32_t *Address, uint32_t Value) {
    uint16_t p             = (const uint32_t)Address;
    uint32_t existingValue = EEPROM_ReadDataByte(p) | (EEPROM_ReadDataByte(p + 1) << 8) | (EEPROM_ReadDataByte(p + 2) << 16) | (EEPROM_ReadDataByte(p + 3) << 24);
    if (Value != existingValue) {
        EEPROM_WriteDataByte(p, (uint8_t)Value);
        EEPROM_WriteDataByte(p + 1, (uint8_t)(Value >> 8));
        EEPROM_WriteDataByte(p + 2, (uint8_t)(Value >> 16));
        EEPROM_WriteDataByte(p + 3, (uint8_t)(Value >> 24));
    }
}

void eeprom_read_block(void *buf, const void *addr, size_t len) {
    const uint8_t *p    = (const uint8_t *)addr;
    uint8_t *      dest = (uint8_t *)buf;
    while (len--) {
        *dest++ = eeprom_read_byte(p++);
    }
}

void eeprom_write_block(const void *buf, void *addr, size_t len) {
    uint8_t *      p   = (uint8_t *)addr;
    const uint8_t *src = (const uint8_t *)buf;
    while (len--) {
        eeprom_write_byte(p++, *src++);
    }
}

void eeprom_update_block(const void *buf, void *addr, size_t len) {
    uint8_t *      p   = (uint8_t *)addr;
    const uint8_t *src = (const uint8_t *)buf;
    while (len--) {
        eeprom_write_byte(p++, *src++);
    }
}

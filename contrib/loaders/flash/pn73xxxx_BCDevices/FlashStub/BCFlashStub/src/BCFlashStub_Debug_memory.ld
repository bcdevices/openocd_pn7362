/*
 */

MEMORY
{
  /* Define each memory region */
  Flash (rx) : ORIGIN = 0x203000, LENGTH = 0x27800 /* 158K bytes (alias Flash) */  
  SRAM (rwx) : ORIGIN = 0x100020, LENGTH = 0xfe0 /* 4064 bytes (alias RAM) */  
  EEPROM (rx) : ORIGIN = 0x201200, LENGTH = 0xe00 /* 3584 bytes (alias Flash2) */  
}

  /* Define a symbol for the top of each memory region */
  __base_Flash = 0x203000  ; /* Flash */  
  __base_Flash = 0x203000 ; /* Flash */  
  __top_Flash = 0x203000 + 0x27800 ; /* 158K bytes */  
  __top_Flash = 0x203000 + 0x27800 ; /* 158K bytes */  
  __base_SRAM = 0x100020  ; /* SRAM */  
  __base_RAM = 0x100020 ; /* RAM */  
  __top_SRAM = 0x100020 + 0xfe0 ; /* 4064 bytes */  
  __top_RAM = 0x100020 + 0xfe0 ; /* 4064 bytes */  
  __base_EEPROM = 0x201200  ; /* EEPROM */  
  __base_Flash2 = 0x201200 ; /* Flash2 */  
  __top_EEPROM = 0x201200 + 0xe00 ; /* 3584 bytes */  
  __top_Flash2 = 0x201200 + 0xe00 ; /* 3584 bytes */  

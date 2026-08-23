#ifndef RFID_H
#define RFID_H

#include <stdint.h>

#define RFID_UID_SIZE 5U

typedef enum {
    RFID_STATUS_OK = 0,
    RFID_STATUS_ERROR,
    RFID_STATUS_TIMEOUT,
    RFID_STATUS_NO_CARD,
    RFID_STATUS_COLLISION,
    RFID_STATUS_CRC_ERROR
} RFID_Status;

typedef struct {
    uint8_t bytes[RFID_UID_SIZE];
    uint8_t size;
} RFID_UID;

void RFID_HardwareReset(void);
RFID_Status RFID_Init(void);
uint8_t RFID_GetVersion(void);
RFID_Status RFID_IsCardPresent(void);
RFID_Status RFID_ReadUID(RFID_UID *uid);
RFID_Status RFID_Poll(RFID_UID *uid);
RFID_Status RFID_Halt(void);

#endif // RFID_H

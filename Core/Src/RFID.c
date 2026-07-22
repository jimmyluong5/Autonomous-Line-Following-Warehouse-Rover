#include <RFID.h>
#include <main.h>
#include <string.h>

//spi stuff 
extern SPI_HandleTypeDef hspi1;

//rfid commands
#define PCD_IDLE        0x00U
#define PCD_CALC_CRC    0x03U
#define PCD_TRANSCEIVE  0x0CU
#define PCD_SOFT_RESET  0x0FU


///* PICC commands */

#define PICC_CMD_REQA            0x26U
#define PICC_CMD_WUPA            0x52U
#define PICC_CMD_SEL_CL1         0x93U
#define PICC_CMD_HLTA            0x50U

/* MFRC522 registers */

#define REG_COMMAND              0x01U
#define REG_COM_IRQ              0x04U
#define REG_DIV_IRQ              0x05U
#define REG_ERROR                0x06U
#define REG_FIFO_DATA            0x09U
#define REG_FIFO_LEVEL           0x0AU
#define REG_CONTROL              0x0CU
#define REG_BIT_FRAMING          0x0DU
#define REG_COLL                 0x0EU

#define REG_MODE                 0x11U
#define REG_TX_MODE              0x12U
#define REG_RX_MODE              0x13U
#define REG_TX_CONTROL           0x14U
#define REG_TX_ASK               0x15U

#define REG_CRC_RESULT_H         0x21U
#define REG_CRC_RESULT_L         0x22U

#define REG_MOD_WIDTH            0x24U
#define REG_RF_CFG               0x26U
#define REG_T_MODE               0x2AU
#define REG_T_PRESCALER          0x2BU
#define REG_T_RELOAD_H           0x2CU
#define REG_T_RELOAD_L           0x2DU

#define REG_VERSION              0x37U

/* Register bit masks */

#define BIT_COMMAND_POWER_DOWN   0x10U
#define BIT_FIFO_FLUSH           0x80U
#define BIT_START_SEND           0x80U
#define BIT_TX1_RF_EN            0x01U
#define BIT_TX2_RF_EN            0x02U

#define RFID_SPI_TIMEOUT_MS      100U
#define RFID_TRANSCEIVE_TIMEOUT  2000U
#define RFID_CRC_TIMEOUT         5000U

static void RFID_Select(void) {
    HAL_GPIO_WritePin(RFID_SPI_CS_GPIO_Port, RFID_SPI_CS_Pin, GPIO_PIN_SET);
}
static void RFID_Deselect(void) {
    HAL_GPIO_WritePin(RFID_SPI_CS_GPIO_Port, RFID_SPI_CS_Pin, GPIO_PIN_RESET);
}

static void RFID_WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t frame[2];

    /*
     * MFRC522 SPI write address
     *
     * Bit 7 is zero for write.
     * Register address occupies bits 6 through 1.
     * Bit 0 is zero.
     */
    frame[0] = (uint8_t)((reg << 1U) & 0x7EU);
    frame[1] = value;

    RFID_Select();

    (void)HAL_SPI_Transmit(
        &hspi1,
        frame,
        sizeof(frame),
        RFID_SPI_TIMEOUT_MS
    );

    RFID_Deselect();
}

static uint8_t RFID_ReadRegister(uint8_t reg)
{
    uint8_t tx[2];
    uint8_t rx[2];

    /*
     * Bit 7 is one for read.
     */
    tx[0] = (uint8_t)(0x80U | ((reg << 1U) & 0x7EU));
    tx[1] = 0x00U;

    rx[0] = 0U;
    rx[1] = 0U;

    RFID_Select();

    (void)HAL_SPI_TransmitReceive(
        &hspi1,
        tx,
        rx,
        sizeof(tx),
        RFID_SPI_TIMEOUT_MS
    );

    RFID_Deselect();

    return rx[1];
}

static void RFID_WriteRegisterArray(
    uint8_t reg,
    const uint8_t *data,
    uint8_t length
)
{
    uint8_t address;

    if ((data == NULL) || (length == 0U))
    {
        return;
    }

    address = (uint8_t)((reg << 1U) & 0x7EU);

    RFID_Select();

    (void)HAL_SPI_Transmit(
        &hspi1,
        &address,
        1U,
        RFID_SPI_TIMEOUT_MS
    );

    (void)HAL_SPI_Transmit(
        &hspi1,
        (uint8_t *)data,
        length,
        RFID_SPI_TIMEOUT_MS
    );

    RFID_Deselect();
}


static void RFID_SetRegisterBits(uint8_t reg, uint8_t mask)
{
    uint8_t value = RFID_ReadRegister(reg);

    RFID_WriteRegister(reg, (uint8_t)(value | mask));
}

static void RFID_ClearRegisterBits(uint8_t reg, uint8_t mask)
{
    uint8_t value = RFID_ReadRegister(reg);

    RFID_WriteRegister(reg, (uint8_t)(value & (uint8_t)(~mask)));
}

static void RFID_AntennaOn(void)
{
    uint8_t value = RFID_ReadRegister(REG_TX_CONTROL);

    if ((value & (BIT_TX1_RF_EN | BIT_TX2_RF_EN)) != (BIT_TX1_RF_EN | BIT_TX2_RF_EN)) {
        RFID_SetRegisterBits(REG_TX_CONTROL, BIT_TX1_RF_EN | BIT_TX2_RF_EN);
    }
}

static RFID_Status RFID_CalculateCRC(
    const uint8_t *data,
    uint8_t length,
    uint8_t result[2]
)
{
    uint32_t timeout;

    if ((data == NULL) || (result == NULL))
    {
        return RFID_STATUS_ERROR;
    }

    RFID_WriteRegister(REG_COMMAND, PCD_IDLE);

    /*
     * Clear the CRC interrupt flag.
     */
    RFID_WriteRegister(REG_DIV_IRQ, 0x04U);

    /*
     * Flush FIFO and load the input data.
     */
    RFID_SetRegisterBits(REG_FIFO_LEVEL, BIT_FIFO_FLUSH);
    RFID_WriteRegisterArray(REG_FIFO_DATA, data, length);

    RFID_WriteRegister(REG_COMMAND, PCD_CALC_CRC);

    timeout = RFID_CRC_TIMEOUT;

    while (timeout > 0U)
    {
        if ((RFID_ReadRegister(REG_DIV_IRQ) & 0x04U) != 0U)
        {
            RFID_WriteRegister(REG_COMMAND, PCD_IDLE);

            result[0] = RFID_ReadRegister(REG_CRC_RESULT_L);
            result[1] = RFID_ReadRegister(REG_CRC_RESULT_H);

            return RFID_STATUS_OK;
        }

        timeout--;
    }

    RFID_WriteRegister(REG_COMMAND, PCD_IDLE);

    return RFID_STATUS_TIMEOUT;
}

static RFID_Status RFID_Transceive(
    const uint8_t *tx_data,
    uint8_t tx_length,
    uint8_t *rx_data,
    uint8_t *rx_length,
    uint8_t *valid_bits
)
{
    uint8_t irq;
    uint8_t error;
    uint8_t fifo_length;
    uint8_t last_bits;
    uint8_t index;
    uint32_t timeout;

    if ((tx_data == NULL) ||
        (tx_length == 0U) ||
        (rx_data == NULL) ||
        (rx_length == NULL))
    {
        return RFID_STATUS_ERROR;
    }

    RFID_WriteRegister(REG_COMMAND, PCD_IDLE);

    /*
     * Clear interrupt flags.
     */
    RFID_WriteRegister(REG_COM_IRQ, 0x7FU);

    /*
     * Flush the FIFO buffer.
     */
    RFID_SetRegisterBits(REG_FIFO_LEVEL, BIT_FIFO_FLUSH);

    RFID_WriteRegisterArray(
        REG_FIFO_DATA,
        tx_data,
        tx_length
    );

    /*
     * Number of valid bits in the final transmitted byte.
     */
    if (valid_bits != NULL)
    {
        RFID_WriteRegister(
            REG_BIT_FRAMING,
            (uint8_t)(*valid_bits & 0x07U)
        );
    }
    else
    {
        RFID_WriteRegister(REG_BIT_FRAMING, 0x00U);
    }

    RFID_WriteRegister(REG_COMMAND, PCD_TRANSCEIVE);

    RFID_SetRegisterBits(REG_BIT_FRAMING, BIT_START_SEND);

    timeout = RFID_TRANSCEIVE_TIMEOUT;

    do
    {
        irq = RFID_ReadRegister(REG_COM_IRQ);
        timeout--;
    }
    while ((timeout > 0U) &&
           ((irq & 0x30U) == 0U) &&
           ((irq & 0x01U) == 0U));

    RFID_ClearRegisterBits(REG_BIT_FRAMING, BIT_START_SEND);

    if (timeout == 0U)
    {
        return RFID_STATUS_TIMEOUT;
    }

    /*
     * Timer interrupt means the card did not reply.
     */
    if ((irq & 0x01U) != 0U)
    {
        return RFID_STATUS_NO_CARD;
    }

    error = RFID_ReadRegister(REG_ERROR);

    /*
     * Buffer overflow, parity or protocol error.
     */
    if ((error & 0x13U) != 0U)
    {
        return RFID_STATUS_ERROR;
    }

    if ((error & 0x08U) != 0U)
    {
        return RFID_STATUS_COLLISION;
    }

    fifo_length = RFID_ReadRegister(REG_FIFO_LEVEL);

    if (fifo_length > *rx_length)
    {
        return RFID_STATUS_ERROR;
    }

    for (index = 0U; index < fifo_length; index++)
    {
        rx_data[index] = RFID_ReadRegister(REG_FIFO_DATA);
    }

    *rx_length = fifo_length;

    last_bits = (uint8_t)(RFID_ReadRegister(REG_CONTROL) & 0x07U);

    if (valid_bits != NULL)
    {
        *valid_bits = last_bits;
    }

    return RFID_STATUS_OK;
}

void RFID_HardwareReset(void)
{
    RFID_Deselect();

    HAL_GPIO_WritePin(
        RFID_RST_GPIO_Port,
        RFID_RST_Pin,
        GPIO_PIN_RESET
    );

    HAL_Delay(2U);

    HAL_GPIO_WritePin(
        RFID_RST_GPIO_Port,
        RFID_RST_Pin,
        GPIO_PIN_SET
    );

    HAL_Delay(50U);
}

RFID_Status RFID_Init(void)
{
    uint32_t timeout;

    RFID_HardwareReset();

    RFID_WriteRegister(REG_COMMAND, PCD_SOFT_RESET);

    HAL_Delay(50U);

    timeout = 1000U;

    /*
     * Wait until the PowerDown bit clears.
     */
    while (((RFID_ReadRegister(REG_COMMAND) &
             BIT_COMMAND_POWER_DOWN) != 0U) &&
           (timeout > 0U))
    {
        timeout--;
    }

    if (timeout == 0U)
    {
        return RFID_STATUS_TIMEOUT;
    }

    /*
     * Timer configuration used for card response timeouts.
     */
    RFID_WriteRegister(REG_T_MODE, 0x80U);
    RFID_WriteRegister(REG_T_PRESCALER, 0xA9U);
    RFID_WriteRegister(REG_T_RELOAD_H, 0x03U);
    RFID_WriteRegister(REG_T_RELOAD_L, 0xE8U);

    /*
     * Force 100 percent ASK modulation.
     */
    RFID_WriteRegister(REG_TX_ASK, 0x40U);

    /*
     * CRC preset value for ISO14443A.
     */
    RFID_WriteRegister(REG_MODE, 0x3DU);

    /*
     * Recommended modulation width.
     */
    RFID_WriteRegister(REG_MOD_WIDTH, 0x26U);

    /*
     * Receiver gain.
     */
    RFID_WriteRegister(REG_RF_CFG, 0x70U);

    RFID_WriteRegister(REG_TX_MODE, 0x00U);
    RFID_WriteRegister(REG_RX_MODE, 0x00U);

    RFID_AntennaOn();

    HAL_Delay(5U);

    /*
     * Version values commonly observed
     *
     * 0x91 MFRC522 version 1.0
     * 0x92 MFRC522 version 2.0
     *
     * Some clone boards return other nonzero values.
     */
    if ((RFID_GetVersion() == 0x00U) ||
        (RFID_GetVersion() == 0xFFU))
    {
        return RFID_STATUS_ERROR;
    }

    return RFID_STATUS_OK;
}

uint8_t RFID_GetVersion(void)
{
    return RFID_ReadRegister(REG_VERSION);
}

RFID_Status RFID_IsCardPresent(void)
{
    uint8_t command;
    uint8_t response[2];
    uint8_t response_length;
    uint8_t valid_bits;
    RFID_Status status;

    command = PICC_CMD_REQA;
    response_length = sizeof(response);
    valid_bits = 7U;

    /*
     * REQA is a seven-bit command.
     */
    status = RFID_Transceive(
        &command,
        1U,
        response,
        &response_length,
        &valid_bits
    );

    if (status != RFID_STATUS_OK)
    {
        return status;
    }

    /*
     * The ATQA response must contain 16 bits.
     */
    if ((response_length != 2U) || (valid_bits != 0U))
    {
        return RFID_STATUS_ERROR;
    }

    return RFID_STATUS_OK;
}

RFID_Status RFID_ReadUID(RFID_UID *uid)
{
    uint8_t command[2];
    uint8_t response[5];
    uint8_t response_length;
    uint8_t valid_bits;
    uint8_t bcc;
    RFID_Status status;

    if (uid == NULL)
    {
        return RFID_STATUS_ERROR;
    }

    memset(uid, 0, sizeof(*uid));

    /*
     * Select cascade level 1 and request all UID bits.
     */
    command[0] = PICC_CMD_SEL_CL1;
    command[1] = 0x20U;

    response_length = sizeof(response);
    valid_bits = 0U;

    RFID_WriteRegister(REG_COLL, 0x80U);

    status = RFID_Transceive(
        command,
        sizeof(command),
        response,
        &response_length,
        &valid_bits
    );

    if (status != RFID_STATUS_OK)
    {
        return status;
    }

    /*
     * Four UID bytes plus one BCC byte are expected.
     */
    if ((response_length != 5U) || (valid_bits != 0U))
    {
        return RFID_STATUS_ERROR;
    }

    bcc = (uint8_t)(
        response[0] ^
        response[1] ^
        response[2] ^
        response[3] );

    if (bcc != response[4])
    {
        return RFID_STATUS_CRC_ERROR;
    }

    /*
     * 0x88 is the cascade tag used by longer UIDs.
     * This basic driver currently returns only 4-byte UIDs.
     */
    if (response[0] == 0x88U)
    {
        return RFID_STATUS_ERROR;
    }

    memcpy(uid->bytes, response, RFID_UID_SIZE);
    uid->size = RFID_UID_SIZE;

    return RFID_STATUS_OK;
}

RFID_Status RFID_Poll(RFID_UID *uid)
{
    RFID_Status status;

    status = RFID_IsCardPresent();

    if (status != RFID_STATUS_OK)
    {
        return status;
    }

    return RFID_ReadUID(uid);
}

RFID_Status RFID_Halt(void)
{
    uint8_t frame[4];
    uint8_t response[1];
    uint8_t response_length;
    uint8_t valid_bits;
    RFID_Status status;

    frame[0] = PICC_CMD_HLTA;
    frame[1] = 0x00U;

    status = RFID_CalculateCRC(
        frame,
        2U,
        &frame[2]
    );

    if (status != RFID_STATUS_OK)
    {
        return status;
    }

    response_length = sizeof(response);
    valid_bits = 0U;

    /*
     * A correctly halted card does not send a response.
     */
    status = RFID_Transceive(
        frame,
        sizeof(frame),
        response,
        &response_length,
        &valid_bits
    );

    if ((status == RFID_STATUS_TIMEOUT) ||
        (status == RFID_STATUS_NO_CARD))
    {
        return RFID_STATUS_OK;
    }

    return status;
}
#ifndef _AHT10_I2C_H
#define _AHT10_I2C_H

/* Not sure if TRUE is already defined as a macro or not */
#ifndef TRUE
#define TRUE                              1
#endif
#ifndef FALSE
#define FALSE                             0
#endif
#define DELAY_AFTER_CMD                   TRUE                /* 1 means that it'll delay after CMD issuing, otherwise  */

#define I2C_AHT10_MASTER_SCL_IO           2                   /*  gpio number for I2C master clock */
#define I2C_AHT10_MASTER_SDA_IO           0                   /*  gpio number for I2C master data  */
#define I2C_AHT10_MASTER_NUM              I2C_NUM_0           /* I2C port number for master dev */
#define I2C_AHT10_MASTER_TX_BUF_DISABLE   0                   /* I2C master do not need buffer */
#define I2C_AHT10_MASTER_RX_BUF_DISABLE   0                   /* I2C master do not need buffer */

/* The AHT10 Datasheet shows the following workflow:
 * - send a 7-bit address with a write bit (0)
 * - send a measure command
 * - wait over 75ms
 * - send a 7-bit address with a read bit (1)
 * - interpret the status information
 * - Read the data? */
#define AHT10_BYTE_ZEROS                    (uint8_t)0x00       /* dummy zero data byte */
#define AHT10_BYTE_MEASURE                  (uint8_t)0x33       /* measure data byte shown in datasheet */
#define AHT10_MEAS_DELAY                    80                  /* check every N milliseconds for measurement complete */
#define AHT10_DELAY_PWR_ON                  50                  /* milliseconds to wait after power on */
#define AHT10_DELAY_CMD                     350                 /* even though this isn't in the datasheet this seems important */
#define AHT10_DELAY_SOFT_RESET              20                  /* should we need to soft reset, it takes less than 20ms */
#define AHT10_SENSOR_ADDR                   0x38                /* slave address for AHT10 sensor */
#define AHT10_CMD_INIT                      0xE1                /* Command to initialize the device */
#define AHT10_CMD_MEASURE                   0xAC                /* Command to measure temperature and humidity (and send response after delay) */
#define AHT10_CMD_SOFTRESET                 0xBA                /* Command to perform a soft reset (<20ms delay) */
#define AHT10_INIT_REG_NORMAL               0x00                /* sleep between measurements */
#define AHT10_INIT_REG_CYCLE                0x20                /* continuous measurement */
#define AHT10_INIT_REG_CMD                  0x40                /* command mode */
#define AHT10_INIT_REG_CAL                  0x08                /* calibration something or other */
#define AHT10_STATUS_BITS_BUSY              0x40                /* Status bit indicating busy (measuring) status */
#define AHT10_STATUS_BITS_MODE              0x30                /* Mode meanings: 00 is NOR, 01 is CYC, 1X is CMD */
#define AHT10_STATUS_BITS_CAL               0x04                /* Cal bit (set if calibrated) */
#define WRITE_BIT                           I2C_MASTER_WRITE    /* I2C master write */
#define READ_BIT                            I2C_MASTER_READ     /* I2C master read */
#define ACK_CHECK_EN                        0x1                 /* I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0                 /* I2C master will not check ack from slave */
#define ACK_VAL                             0x0                 /* I2C ack value */
#define NACK_VAL                            0x1                 /* I2C nack value */
#define LAST_NACK_VAL                       0x2                 /* I2C last_nack value */

void i2c_task_aht10(void *arg);

#endif /* _AHT10_I2C_H */ 

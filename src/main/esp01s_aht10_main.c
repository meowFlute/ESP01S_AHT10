/* Author: M. Scott Christensen (mscottchristensen@gmail.com)
 * File Creation Date: June 9, 2023
 * Update History: See git log
 * Description:
 *
 * ESP-01S + AHT10 Code to help me learn a bit more about 
 *      - REST APIs
 *      - Server-side programming
 *      - Admin/Data Analytics dashboards
 *      - OTA updates for embedded projects
 *      - Encryption
 *      - The temperature and humidity distribution of each room in my home
 * FAQ (probably, no one really asked)
 *      - Why not just use existing libraries?
 *          I kind of am, but the main reason is because I like programming
 *          I also aspire to do some of this type of thing in "production"
 *          later and would like to retain the skills to develop similar
 *          software on other platforms/hardware/etc
 * TODO
 * 1) Get AHT10 communication working over I2C
 * 2) Get WiFi working
 * 3) Get HTTP communication working with my server
 *      - I think the best method would be POSTing data back every 
 *          <configurable> minutes via a server side API (probably REST)
 *      - Server adds the timestamps for the data upon reception and logs them
 *          in a relational database
 *      - Server provides central interface for data viewing from all sensors
 * 
 * Stretch goals
 *      - Get libsodium-based encryption working on the transmission of data
 *      - Get OTA working for software updates from server
 *      - Get simple HTTP status server working for inspection
*/

/* Toolchain headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS headers */
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
#include "freertos/queue.h"

/* ESP8266EX Headers */
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

/* Driver Headers */
#include "driver/i2c.h"

/**
 * TEST CODE BRIEF
 *
 * This example will show you how to use I2C module by running two tasks on i2c bus:
 *
 * - read external i2c sensor, here we use a AHT10 sensor for instance.
 * - Use one I2C port(master mode) to read or write the other I2C port(slave mode) on one ESP8266 chip.
 *
 * Pin assignment:
 *
 * - master:
 *    GPIO14 is assigned as the data signal of i2c master port
 *    GPIO2 is assigned as the clock signal of i2c master port
 *
 * Connection:
 *
 * - connect sda/scl of sensor with GPIO14/GPIO2
 * - no need to add external pull-up resistors, driver will enable internal pull-up resistors.
 *
 * Test items:
 *
 * - read the sensor data, if connected.
 */

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
#define AHT10_PWR_ON_DELAY                  50                  /* milliseconds to wait after power on */
#define AHT10_CMD_DELAY                     350                 /* even though this isn't in the datasheet this seems important */
#define AHT10_SOFT_RESET_DELAY              20                  /* should we need to soft reset, it takes less than 20ms */
#define AHT10_SENSOR_ADDR                   0x38                /* slave address for AHT10 sensor */
#define AHT10_CMD_NORMAL_MODE               0xA8                /* No info in the data sheet on this, but it sleeps between measurement */
#define AHT10_CMD_INIT                      0xE1                /* Command to initialize the device */
#define AHT10_CMD_MEASURE                   0xAC                /* Command to measure temperature and humidity (and send response after delay) */
#define AHT10_CMD_SOFTRESET                 0xBA                /* Command to perform a soft reset (<20ms delay) */
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

/* Configure I2C communication according to what I see on the datasheet */
static esp_err_t i2c_master_init()
{
    int i2c_master_port = I2C_AHT10_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_AHT10_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_AHT10_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    i2c_driver_install(i2c_master_port, conf.mode);
    i2c_param_config(i2c_master_port, &conf);
    return ESP_OK;
}

/*  Write to AHT10
 *
 * 1. send data
 * ___________________________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | write data_len byte + ack  | stop |
 * --------|---------------------------|-------------------------|----------------------------|------|
 *
 * esp_err_t values
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
static esp_err_t i2c_master_aht10_write(i2c_port_t i2c_num, uint8_t reg_address, uint8_t *data, size_t data_len)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, AHT10_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_address, ACK_CHECK_EN);
    i2c_master_write(cmd, data, data_len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

/* read AHT10
 *
 * 1. send reg address
 * ______________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write reg_address + ack | stop |
 * --------|---------------------------|-------------------------|------|
 *
 * 2. read data
 * ___________________________________________________________________________________
 * | start | slave_addr + wr_bit + ack | read data_len byte + ack(last nack)  | stop |
 * --------|---------------------------|--------------------------------------|------|
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
static esp_err_t i2c_master_aht10_read(i2c_port_t i2c_num, uint8_t *data, size_t data_len)
{
    /* we don't have to write a register address, it always just returns the
     * following format:
     * - byte 0 is the status byte
     * - 20 bits of humidity data 
     * - 20 bits of temperature data 
     *
     * NOTE: if the status bit isn't set, then the data is nonsense 
     *       if you haven't triggered a measurement, the data is from the last
     *       measurement (assuming the status bit says it is valid data) */
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, AHT10_SENSOR_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

static esp_err_t i2c_master_aht10_init(i2c_port_t i2c_num)
{
    uint8_t cmd_data[2];
    vTaskDelay(AHT10_PWR_ON_DELAY / portTICK_RATE_MS);
    i2c_master_init();  // set the i2c master parameters for the esp8266
    cmd_data[0] = AHT10_BYTE_ZEROS;  // dummy data, the command is the register address here
    cmd_data[1] = AHT10_BYTE_ZEROS;
    /* this is where we send the init command to the aht10 */
    i2c_master_aht10_write(i2c_num, AHT10_CMD_INIT, cmd_data, 2); 
    
    /* is this needed? I'm not sure, but we'll try it both ways */
    vTaskDelay(AHT10_CMD_DELAY / portTICK_RATE_MS);

    return ESP_OK;
}

static void i2c_task_aht10(void *arg)
{
    uint8_t busy;
    uint8_t cmd_data[2];
    uint8_t rx_data[6];
    uint32_t temperature_raw_data, humidity_raw_data;
    float temperature, humidity;

    /* Basic flow
     * 1) Send the init command
     * 2) Send the measurement command
     * 3) Wait for measurement to be complete (~75ms, but you can see the
     *      status when you read)
     * 4) Perform transfer function on the received data
     * Loop 2-4 */

    /* 1) Send the init command */
    i2c_master_aht10_init(I2C_AHT10_MASTER_NUM);

    /* Loop 2-4 */
    while (1) 
    {
        /* 2) Send the measurement command */
        cmd_data[0] = AHT10_BYTE_MEASURE;
        cmd_data[1] = AHT10_BYTE_ZEROS;
        i2c_master_aht10_write(I2C_NUM_0, AHT10_CMD_MEASURE, cmd_data, 2);
        busy = 1U; 
        while (busy)
        {
            /* 3) wait some number of ms and read again */
            vTaskDelay(AHT10_MEAS_DELAY / portTICK_RATE_MS);

            /* Perform a read of the status byte */
            i2c_master_aht10_read(I2C_AHT10_MASTER_NUM, rx_data, 6);

            /* check the busy bit */
            if ((rx_data[0] & AHT10_STATUS_BITS_BUSY) == AHT10_STATUS_BITS_BUSY)
            {
                /* device is still busy */
                continue;
            }
            else
            {
                /* no longer busy */
                busy = 0U;
            }
        }
        /* 4) we'll include raw data extraction in the transfer function part */

        /* grab the 20-bit numbers for humidity and temperature */
        /* humidity is the first 20 bits of bytes 1-3 */
        humidity_raw_data = ((uint32_t)rx_data[1] << 16) | ((uint32_t)rx_data[2] << 8) | ((uint32_t)rx_data[3] >> 4);
        
        /* temperature is the final 20 bits of bytes 3-5 */
        temperature_raw_data = (((uint32_t)rx_data[3] & 0x0F) << 16) | ((uint32_t)rx_data[4] << 8) | (uint32_t)rx_data[5];
        
        /* perform transfer functions */
        /* humidity = (uint32_t / 2^20) * 100% */
        /* I'll just leave it as a decimal instead of a percentage */
        humidity = ((float)humidity_raw_data / (float)0x100000U);
        
        /* temperature = 200*(uint32_t / 2^20) - 50 */
        temperature = (200.0F*((float)humidity_raw_data / (float)0x100000U)) - 50.0F;

        /* report data */
        /* for debugging we'll output the data over serial */
        printf("\nstatus byte = 0x%02X\n", rx_data[0]);
        printf("\nhumidity raw data = 0x%08X\n", humidity_raw_data);
        printf("temperature raw data = 0x%08X\n", temperature_raw_data);
        printf("\nhumidity converted = %f", humidity);
        printf("temperature converted = %f", temperature);

        /* read once every 5 seconds 
         * they recommend a maximum of once every 2 seconds */
        vTaskDelay(5000 / portTICK_RATE_MS);
    }

    i2c_driver_delete(I2C_AHT10_MASTER_NUM);
}

void app_main(void)
{
    //start i2c task
    xTaskCreate(i2c_task_aht10, "i2c_task_aht10", 2048, NULL, 10, NULL);
}

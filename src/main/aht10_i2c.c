/* associated header file */
#include "aht10_i2c.h"

/* others necessary headers */
#include "driver/i2c.h"
#include "freertos/task.h"

/* ====================================
 * ========= STATIC FUNCTIONS =========
 * ==================================== */

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
    uint8_t read_data[6];
    uint8_t register_value = 0x10;
    vTaskDelay(AHT10_DELAY_PWR_ON / portTICK_RATE_MS);
    i2c_master_init();  // set the i2c master parameters for the esp8266
    
    //while (register_value != 0x00)
    //{
        printf("Attempting to write normal mode\n");
        cmd_data[0] = AHT10_INIT_REG_NORMAL; 
        cmd_data[1] = AHT10_BYTE_ZEROS;
        /* this is where we send the init command to the aht10 */
        i2c_master_aht10_write(i2c_num, AHT10_CMD_INIT, cmd_data, 1); 
    
#if DELAY_AFTER_CMD
        /* is this needed? I'm not sure, but we'll try it both ways */
        vTaskDelay(AHT10_DELAY_CMD / portTICK_RATE_MS);
#endif
        i2c_master_aht10_read(I2C_AHT10_MASTER_NUM, read_data, 6);
        register_value = read_data[0]; // read bits 5 and 6
        printf("Register value = 0x%02X\n", register_value);
    //}

    return ESP_OK;
}

/* ====================================
 * ========= GLOBAL FUNCTIONS =========
 * ==================================== */
void i2c_task_aht10(void *arg)
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
    printf("Initializing the AHT10\n");
    i2c_master_aht10_init(I2C_AHT10_MASTER_NUM);

    /* Loop 2-4 */
    while (1) 
    {
        /* 2) Send the measurement command */
        cmd_data[0] = AHT10_BYTE_MEASURE;
        cmd_data[1] = AHT10_BYTE_ZEROS;
        printf("writing to the AHT10 to command a measure\n");
        i2c_master_aht10_write(I2C_NUM_0, AHT10_CMD_MEASURE, cmd_data, 2);

        busy = 1U; 
        while (busy)
        {
            printf("Waiting for measurement delay\n");

            /* 3) wait some number of ms and read again */
            vTaskDelay(AHT10_MEAS_DELAY / portTICK_RATE_MS);

            /* Perform a read of the status byte */
            printf("Reading from the AHT10\n");
            i2c_master_aht10_read(I2C_AHT10_MASTER_NUM, rx_data, 6);

            /* check the busy bit */
            if ((rx_data[0] & AHT10_STATUS_BITS_BUSY) == AHT10_STATUS_BITS_BUSY)
            {
                /* device is still busy */
                printf("Device is still busy\n");
                continue;
            }
            else
            {
                /* no longer busy */
                printf("Device is no longer busy\n");
                busy = 0U;
            }
        }
        /* 4) we'll include raw data extraction in the transfer function part */

        /* grab the 20-bit numbers for humidity and temperature */
        /* humidity is the first 20 bits of bytes 1-3 */
        humidity_raw_data = ((uint32_t)rx_data[1] << 12) | ((uint32_t)rx_data[2] << 4) | ((uint32_t)rx_data[3] >> 4);
        
        /* temperature is the final 20 bits of bytes 3-5 */
        temperature_raw_data = (((uint32_t)rx_data[3] & 0x0F) << 16) | ((uint32_t)rx_data[4] << 8) | (uint32_t)rx_data[5];
        
        /* perform transfer functions */
        /* humidity = (uint32_t / 2^20) * 100% */
        /* I'll just leave it as a decimal instead of a percentage */
        humidity = (float)humidity_raw_data;
        humidity /= 1048576.0F;
        
        /* temperature = 200*(uint32_t / 2^20) - 50 */
        printf("\ntemperature before cast to float: %d\n", temperature_raw_data);
        temperature = (float)temperature_raw_data;
        printf("temperature after cast to float: %f\n", temperature);
        temperature *= 200.0F;
        printf("temperature after multiplication by 200.0F: %f\n", temperature);
        temperature /= 1048576.0F;
        printf("temperature after division by 1048576.0F: %f\n", temperature);
        temperature -= 50.0F;
        printf("temperature after subtraction of 50.0F: %f\n\n", temperature);

        /* report data */
        /* for debugging we'll output the data over serial */
        printf("status byte 0 = 0x%02X\n\n", rx_data[0]);
        printf("data bytes 1-5 = 0x%02X%02X%02X%02X%02X\n\n", rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5]);
        printf("humidity raw data = 0x%08X\n", humidity_raw_data);
        printf("temperature raw data = 0x%08X\n\n", temperature_raw_data);
        printf("humidity converted = %f\n", humidity);
        printf("temperature converted = %f\n\n", temperature);

        /* read once every 5 seconds 
         * they recommend a maximum of once every 2 seconds */
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
    fflush(stdout);
    i2c_driver_delete(I2C_AHT10_MASTER_NUM);
}

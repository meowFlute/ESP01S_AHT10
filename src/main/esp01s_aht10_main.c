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
 *
 * PLANNING -- The below are the general tasks I foresee wanting to tackle
 * DONE
 * 1) Get AHT10 communication working over I2C
 *
 * TODO
 * 2) Get WiFi working
 * 3) Get HTTP communication working with my server
 *      - I think the best method would be POSTing data back every 
 *          <configurable> minutes via a server side API (probably REST)
 *      - Server adds the timestamps for the data upon reception and logs them
 *          in a relational database
 *      - Server provides central interface for data viewing from all sensors
 * 4) Get power save / sleep working to minimize power levels
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
#include "freertos/event_groups.h"

/* ESP8266EX Headers */
#include "esp_system.h"
#include "esp_spi_flash.h"

/* local main functions */
#include "aht10_i2c.h"
#include "wifi_logging.h"

void app_main(void)
{
    /* start i2c task */
    xTaskCreate(i2c_task_aht10, "i2c_task_aht10", 2048, NULL, 10, NULL);

    /* initialize wifi */
    wifi_init_all();
}

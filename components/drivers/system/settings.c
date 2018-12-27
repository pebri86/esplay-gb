/**
 * @file settings.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "settings.h"

#include "nvs_flash.h"
#include "esp_heap_caps.h"

#include "string.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static const char* NvsNamespace = "micrones";
static const char* NvsKey_Backlight = "backlight";
static const char* NvsKey_Partition = "rom-partition";

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int32_t get_backlight_settings()
{
    // TODO: Move to header
    int result = 30;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i32(my_handle, NvsKey_Backlight, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}

void set_backlight_settings(int32_t value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    printf("Try saving ... \n");

    // Read
    err = nvs_set_i32(my_handle, NvsKey_Backlight, value);
    if (err != ESP_OK) abort();

    printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

    // Close
    nvs_close(my_handle);
}

int8_t get_rom_partition_settings()
{
    // TODO: Move to header
    int result = -1;

    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    // Read
    err = nvs_get_i8(my_handle, NvsKey_Partition, &result);
    if (err == ESP_OK)
    {
        printf("%s: value=%d\n", __func__, result);
    }

    // Close
    nvs_close(my_handle);

    return result;
}

void set_rom_partition_settings(int8_t value)
{
    // Open
    nvs_handle my_handle;
    esp_err_t err = nvs_open(NvsNamespace, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) abort();

    printf("Try saving ... \n");

    // Read
    err = nvs_set_i8(my_handle, NvsKey_Partition, value);
    if (err != ESP_OK) abort();

    printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

    // Close
    nvs_close(my_handle);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

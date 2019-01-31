/**
 * @file ui.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "ui.h"
#include "display.h"
#include "ili9341.h"
#include "gamepad.h"
#include "display.h"
#include "settings.h"
#include "string.h"
#include "sdcard.h"
#include "rom/crc.h"
#include "power.h"

/*********************
 *      DEFINES
 *********************/
#define TAG_DEBUG "DEBUG"
#define TAG_INFO  "INFO"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void init_rom_list();
static int get_menu_selected();
static lv_res_t list_release_action(lv_obj_t * btn);
static void create_header();
static void create_footer();
static void create_list_page(lv_obj_t * parent);
static lv_res_t slider_action(lv_obj_t *slider);
static lv_res_t vol_slider_action(lv_obj_t *slider);
static void create_settings_page(lv_obj_t *parent);
static void ui_task(void *arg);
static void lv_task(void *arg);
static void copy_rom_task(void *arg);
static void flash_rom(const char* fullPath);
static lv_res_t tab_load_callback(lv_obj_t * tab, uint16_t act_id);
static void check_battery(lv_obj_t* battery_label);
static void set_volume_icon();

LV_IMG_DECLARE(img_bubble_pattern);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_indev_t * indev;
static lv_group_t *group;
static lv_obj_t *header;
static lv_obj_t *volume_icon;
static lv_obj_t *battery;
static lv_obj_t *footer;
static lv_obj_t *list;
static lv_obj_t *slider;
static lv_obj_t *vol_slider;
static lv_obj_t *tv;
static lv_obj_t *tab1;
static lv_obj_t *tab2;
static int selected = -1;
static int brightness_value;
static int volume_value;
static bool ui_task_is_running = false;
static bool lv_task_is_running = false;
static int tabSize;

char* VERSION = NULL;
//const char* SD_CARD = "/sd";
char** files;
int fileCount;
const char* SD_CARD = "/sd";
const char* path = "/sd/roms/gbc";
size_t fullPathLength;
char* fullPath;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void ui_init()
{
    init_rom_list();

    xTaskCreate(&ui_task, "ui_task", 1024 * 8, NULL, 5, NULL);

    printf("ui_init done.\n");
}

void ui_create(void)
{
    const char* VER_PREFIX = "Ver: ";
    size_t ver_size = strlen(VER_PREFIX) + strlen(COMPILEDATE) + 1 + strlen(GITREV) + 1;
    VERSION = malloc(ver_size);
    if (!VERSION) abort();

    strcpy(VERSION, VER_PREFIX);
    strcat(VERSION, COMPILEDATE);
    strcat(VERSION, "-");
    strcat(VERSION, GITREV);

    lv_obj_t * scr = lv_page_create(NULL, NULL);
    lv_page_set_sb_mode(scr, LV_SB_MODE_OFF);
    lv_scr_load(scr);

    lv_obj_t *wp = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(wp, &img_bubble_pattern);
    lv_obj_set_width(wp, LV_HOR_RES * 4);
    lv_obj_set_protect(wp, LV_PROTECT_POS);

    /*create group for keypad input*/
    group = lv_group_create();
    lv_indev_set_group(indev, group);

    create_header();
    create_footer();

    static lv_style_t style_tv_btn_bg;
    lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
    style_tv_btn_bg.body.main_color = LV_COLOR_HEX(0x487fb7);
    style_tv_btn_bg.body.grad_color = LV_COLOR_HEX(0x487fb7);
    style_tv_btn_bg.body.padding.ver = 0;

    static lv_style_t style_tv_btn_rel;
    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.body.empty = 1;
    style_tv_btn_rel.body.border.width = 0;

    static lv_style_t style_tv_btn_pr;
    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
    style_tv_btn_pr.body.radius = 0;
    style_tv_btn_pr.body.opa = LV_OPA_50;
    style_tv_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.border.width = 0;
    style_tv_btn_pr.text.color = LV_COLOR_GRAY;

    tv = lv_tabview_create(scr, NULL);
    lv_obj_align(tv, header, LV_ALIGN_IN_TOP_LEFT, ((lv_obj_get_width(header)-LV_HOR_RES)/2), (lv_obj_get_height(header)));    
    lv_obj_set_size(tv, LV_HOR_RES, LV_VER_RES - (lv_obj_get_height(header) + lv_obj_get_height(footer)));
    lv_tabview_set_tab_load_action(tv, tab_load_callback);
#if 1
    lv_obj_set_parent(wp, ((lv_tabview_ext_t *) tv->ext_attr)->content);
    lv_obj_set_pos(wp, 0, -5);
#endif
    tab1 = lv_tabview_add_tab(tv, SYMBOL_DIRECTORY " Play");
    tab2 = lv_tabview_add_tab(tv, SYMBOL_SETTINGS " Settings");
    tabSize = lv_obj_get_height(tab1);

    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_INDIC, &lv_style_plain);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

    lv_group_add_obj(group, tv);

    create_list_page(tab1);
    create_settings_page(tab2);
}

int ui_choose_rom()
{
    return get_menu_selected();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void copy_rom_task(void *arg)
{
    printf("copy_rom_task started.\n");
    vTaskDelay(500);
    set_rom_name_settings(fullPath);
    set_menu_flag_settings(0);
    free(fullPath);

    // Restart device
    esp_restart();
    vTaskDelete(NULL);
}

static void lv_task(void *arg)
{
    printf("lv_task started.\n");
    lv_task_is_running = true;
    while(lv_task_is_running)
    {
        lv_tick_inc(portTICK_RATE_MS);
        vTaskDelay(1);
    }

    printf("lv_task done.\n");
    /* Remove the task from scheduler*/
    vTaskDelete(NULL);
    /* Never return*/
    while (1) { vTaskDelay(1);}
}

static void ui_task(void *arg)
{  
    ui_task_is_running = true;

    lv_init();
    lv_disp_drv_t disp;
    lv_disp_drv_init(&disp);
    disp.disp_flush = ili9341_flush;
    lv_disp_drv_register(&disp);
    xTaskCreate(&lv_task, "lv_task", 1024, NULL, 5, NULL);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read = lv_keypad_read;
    indev = lv_indev_drv_register(&indev_drv);
    lv_indev_init();
    ESP_LOGI(TAG_DEBUG, "(%s) RAM left %d", __func__ , esp_get_free_heap_size());

    ui_create();
    unsigned long previousMillis = xTaskGetTickCount();
    while(ui_task_is_running)
    {
        unsigned long currentMillis = xTaskGetTickCount();
        if ((currentMillis - previousMillis) > 5000)
        {
            check_battery(battery);
            previousMillis = currentMillis;
        }
        lv_task_handler();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }

    lv_task_is_running = false;

    ESP_LOGI(TAG_DEBUG, "(%s) RAM left %d", __func__ , esp_get_free_heap_size());
    printf("ui_task done.\n");
    /* Remove the task from scheduler*/
    vTaskDelete(NULL);

    /* Never return*/
    while (1) { vTaskDelay(1);}
}

static lv_res_t list_release_action(lv_obj_t * btn)
{    
    const char * label;
    label = lv_list_get_btn_text(btn);
    fullPathLength = strlen(path) + 1 + strlen(label) + 1;

    fullPath = (char*)malloc(fullPathLength);
    if (!fullPath) abort();

    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, label);

    lv_obj_t * mbox2 = lv_mbox_create(lv_scr_act(), NULL);
    lv_mbox_set_text(mbox2, "Loading ROM, Please wait...");

    xTaskCreate(&copy_rom_task, "copy_rom_task", 1024 * 5, NULL, 5, NULL);

    return LV_RES_OK;
}

static void init_rom_list() {
    // Check SD card
    esp_err_t ret = sdcard_open(SD_CARD);
    if (ret != ESP_OK)
    {
        printf("Error sdcard");
    }
    const char* result = NULL;

    printf("%s: HEAP=%#010x\n", __func__, esp_get_free_heap_size());

    files = 0;
    fileCount = sdcard_files_get(path, ".gbc", &files);
    printf("%s: fileCount=%d\n", __func__, fileCount);
    sdcard_close();
}

static int get_menu_selected()
{
    return selected;
}

static void create_header()
{
    header = lv_label_create(lv_scr_act(), NULL); /*First parameters (scr) is the parent*/
    lv_label_set_text(header, "ESPlay-GB");  /*Set the text*/
    lv_obj_align(header, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

    /****************
    * ADD A BATTERY ICON
    ****************/
    battery = lv_label_create(lv_scr_act(), NULL); /*First parameters (scr) is the parent*/    
    battery_level_init();
    check_battery(battery);
    lv_obj_align(battery, header, LV_ALIGN_IN_TOP_RIGHT, (LV_HOR_RES - lv_obj_get_width(header)-10)/2, 0);

    /****************
    * ADD A CLOCK
    ****************/
    volume_icon = lv_label_create(lv_scr_act(), NULL); /*First parameters (scr) is the parent*/
    lv_label_set_text(volume_icon, SYMBOL_MUTE); /*Set the text*/
    volume_value = (int) (get_volume_settings());
    set_volume_icon();
    lv_obj_align(volume_icon, header, LV_ALIGN_IN_TOP_LEFT, ((lv_obj_get_width(header)-LV_HOR_RES)/2)+5, 0);
}

static void create_footer()
{  
    footer = lv_label_create(lv_scr_act(), NULL); /*First parameters (scr) is the parent*/
    lv_label_set_text(footer, VERSION);  /*Set the text*/
    lv_obj_align(footer, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

static void create_list_page(lv_obj_t * parent)
{
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);

    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    /*********************
     * Create new styles
     ********************/
    /*Create a scroll bar style*/
    static lv_style_t style_sb;
    lv_style_copy(&style_sb, &lv_style_plain);
    style_sb.body.main_color = LV_COLOR_BLACK;
    style_sb.body.grad_color = LV_COLOR_BLACK;
    style_sb.body.border.color = LV_COLOR_WHITE;
    style_sb.body.border.width = 0;
    style_sb.body.border.opa = LV_OPA_70;
    style_sb.body.radius = LV_RADIUS_CIRCLE;
    style_sb.body.opa = LV_OPA_60;

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
    style_btn_pr.body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
    style_btn_pr.text.color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_WHITE;
    style_btn_rel.body.border.width = 0;
    style_btn_rel.body.border.opa = LV_OPA_100;
    style_btn_rel.body.radius = 0;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_ORANGE;
    style_btn_pr.body.grad_color = LV_COLOR_ORANGE;
    style_btn_pr.text.color = LV_COLOR_BLACK;

    /**************************************
     * Create a list with modified styles
     **************************************/

    /*Copy the previous list*/
    list = lv_list_create(parent, NULL);
    lv_obj_set_size(list, LV_HOR_RES, tabSize);
    lv_obj_align(list, parent, LV_ALIGN_IN_TOP_LEFT, ((lv_obj_get_width(parent)-LV_HOR_RES)/2), 0);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_pretty);
    lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, &style_btn_rel); /*Set the new button styles*/
    lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, &style_btn_pr);

    /*Add list elements*/
    for(int i=0; i < fileCount; i++){
        lv_list_add(list, NULL, files[i], list_release_action);
        //printf("%s\n", files[i]);
    }

    sdcard_files_free(files, fileCount);

    lv_group_add_obj(group, list);
    lv_group_focus_obj(list);
}

static void create_settings_page(lv_obj_t *parent)
{
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);

    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_height(parent, lv_obj_get_height(parent));
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    static lv_style_t style_txt;
    lv_style_copy(&style_txt, &lv_style_plain);
    style_txt.text.color = LV_COLOR_WHITE;

    /*Create slider brightness label*/
    lv_obj_t * label_brightness = lv_label_create(parent, NULL); /*First parameters (scr) is the parent*/
    lv_obj_set_style(label_brightness, &style_txt);
    lv_label_set_text(label_brightness, SYMBOL_IMAGE);  /*Set the text*/
    lv_obj_align(label_brightness, parent, LV_ALIGN_IN_TOP_LEFT, 5, 10);

    /*Create a bar, an indicator and a knob style*/
    static lv_style_t style_bar;
    static lv_style_t style_indic;
    static lv_style_t style_knob;

    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color =  LV_COLOR_BLACK;
    style_bar.body.grad_color =  LV_COLOR_GRAY;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_WHITE;
    style_bar.body.opa = LV_OPA_60;
    style_bar.body.padding.hor = 0;
    style_bar.body.padding.ver = LV_DPI / 10;

    lv_style_copy(&style_indic, &lv_style_pretty);
    style_indic.body.grad_color =  LV_COLOR_RED;
    style_indic.body.main_color =  LV_COLOR_WHITE;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.shadow.width = LV_DPI / 10;
    style_indic.body.shadow.color = LV_COLOR_RED;
    style_indic.body.padding.hor = LV_DPI / 30;
    style_indic.body.padding.ver = LV_DPI / 30;

    lv_style_copy(&style_knob, &lv_style_pretty);
    style_knob.body.radius = LV_RADIUS_CIRCLE;
    style_knob.body.opa = LV_OPA_70;

    /*Create a slider*/
    slider = lv_slider_create(parent, NULL);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_BG, &style_bar);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_INDIC, &style_indic);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_KNOB, &style_knob);
    lv_obj_set_size(slider, LV_HOR_RES - 40 - lv_obj_get_width(label_brightness) , LV_DPI / 3);
    lv_obj_align(slider, label_brightness, LV_ALIGN_OUT_RIGHT_MID, 17, 0); /*Align to below the chart*/
    lv_slider_set_action(slider, slider_action);
    lv_slider_set_range(slider, 1, 10);
    int value = (int) (get_backlight_settings() / 10);
    lv_slider_set_value(slider, value);

    /*Create slider brightness label*/
    lv_obj_t * label_volume = lv_label_create(parent, NULL); /*First parameters (scr) is the parent*/
    lv_obj_set_style(label_volume, &style_txt);
    lv_label_set_text(label_volume, SYMBOL_VOLUME_MAX);  /*Set the text*/
    lv_obj_align(label_volume, label_brightness, LV_ALIGN_IN_TOP_LEFT, 5, 30);

    /*Create a second slider*/
    vol_slider = lv_slider_create(parent, NULL);
    lv_slider_set_style(vol_slider, LV_SLIDER_STYLE_BG, &style_bar);
    lv_slider_set_style(vol_slider, LV_SLIDER_STYLE_INDIC, &style_indic);
    lv_slider_set_style(vol_slider, LV_SLIDER_STYLE_KNOB, &style_knob);
    lv_obj_set_size(vol_slider, LV_HOR_RES - 42 - lv_obj_get_width(label_volume), LV_DPI / 3);
    lv_obj_align(vol_slider, label_volume, LV_ALIGN_OUT_RIGHT_MID, 15, 0); /*Align to below the chart*/
    lv_slider_set_action(vol_slider, vol_slider_action);
    lv_slider_set_range(vol_slider, 0, 4);
    lv_slider_set_value(vol_slider, volume_value);
}

/**
 * Called when a new value on the slider on the Chart tab is set
 * @param slider pointer to the slider
 * @return LV_RES_OK because the slider is not deleted in the function
 */
static lv_res_t slider_action(lv_obj_t *slider)
{
    int v = lv_slider_get_value(slider);
    brightness_value = (int)(v * 10);

    //set display brightness in percent;
    set_display_brightness(brightness_value);
    set_backlight_settings((int32_t) brightness_value);

    return LV_RES_OK;
}

static lv_res_t vol_slider_action(lv_obj_t *vol_slider)
{
    int8_t v = lv_slider_get_value(vol_slider);
    volume_value = (int)(v);
    set_volume_icon();
    set_volume_settings((int8_t) volume_value);

    return LV_RES_OK;
}

static void flash_rom(const char* fullPath)
{
    esp_err_t ret;
    
    ret = sdcard_open(SD_CARD);
    if (ret != ESP_OK)
    {
        printf("Error sdcard\n");
    }
    
    printf("%s: HEAP=%#010x\n", __func__, esp_get_free_heap_size());

    printf("Opening file '%s'.\n", fullPath);

    FILE* file = fopen(fullPath, "rb");
    if (file == NULL)
    {
        printf("%s: File open error", __func__);
    }

    const int WRITE_BLOCK_SIZE = 256;
    void* data = malloc(WRITE_BLOCK_SIZE);
    if (!data)
    {
        printf("%s: Data memory error", __func__);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);

    // location to beginning of files
    fseek(file, 0, SEEK_SET);

    const esp_partition_t* part = esp_partition_find_first(0x40, 1, NULL);
    if (part == NULL)
    {
        printf("esp_partition_find_first failed. (0x40)\n");
    }

    // erase entire partition
    ret = esp_partition_erase_range(part, 0, part->size);
    if (ret != ESP_OK)
    {
        printf("sesp_partition_erase_range failed. \n");
    }

    const size_t FLASH_START_ADDRESS = part->address;
    printf("%s: FLASH_START_ADDRESS=%#010x\n", __func__, FLASH_START_ADDRESS);

    size_t curren_flash_address = FLASH_START_ADDRESS;

    for(size_t i = 0; i<file_size; i+=WRITE_BLOCK_SIZE)
    {
        fseek(file, i, SEEK_SET);
        fread(data, WRITE_BLOCK_SIZE+1, 1, file);
        ret = spi_flash_write(curren_flash_address + i, data, WRITE_BLOCK_SIZE);
        if (ret != ESP_OK)
        {
            printf("spi_flash_write failed. address=%#08x\n", curren_flash_address + i);
        }
    }
    printf("Flash done.\n");
    close(file);
}

static lv_res_t tab_load_callback(lv_obj_t * tab, uint16_t act_id)
{
    switch(act_id)
    {
        case 0: 
            lv_group_add_obj(group, list); // Add active tab contant to group
            /* Remove other tabs from group */
            lv_group_remove_obj(slider);
            lv_group_remove_obj(vol_slider);
            break;

        case 1:             
            lv_group_add_obj(group, slider); // Add active tab contant to group
            lv_group_add_obj(group, vol_slider); // Add active tab contant to group
            /* Remove other tabs from group */
            lv_group_remove_obj(list);
            break;
    }
    /*  focus on the tab content */
    lv_group_focus_next(group);

    return LV_RES_OK;
}

static void check_battery(lv_obj_t * battery_label)
{
    battery_state bat;
    battery_level_read(&bat);
    printf("Battery Voltage: %d mV, Percentage : %d %%\n", bat.millivolts, bat.percentage);

    char * icon = SYMBOL_BATTERY_EMPTY;

    //update label
    if (bat.percentage <= 100 && bat.percentage > 80)
        icon = SYMBOL_BATTERY_FULL;
    else if (bat.percentage < 80 && bat.percentage > 60)
        icon = SYMBOL_BATTERY_3;
    else if (bat.percentage < 60 && bat.percentage > 40)
        icon = SYMBOL_BATTERY_2;
    else if (bat.percentage < 40 && bat.percentage > 5)
        icon = SYMBOL_BATTERY_1;
    else if (bat.percentage < 5)
        icon = SYMBOL_BATTERY_EMPTY;
    else icon = SYMBOL_BATTERY_FULL;

    lv_label_set_text(battery_label, icon);
}

static void set_volume_icon()
{
    char * icon = SYMBOL_MUTE SYMBOL_CLOSE;

    //update label
    if (volume_value <= 4 && volume_value >= 3)
        icon = SYMBOL_VOLUME_MAX;
    else if (volume_value <= 2 && volume_value >= 1)
        icon = SYMBOL_VOLUME_MID;
    else icon = SYMBOL_MUTE SYMBOL_CLOSE;

    lv_label_set_text(volume_icon, icon);
}
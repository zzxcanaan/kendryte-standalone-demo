/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <bsp.h>
#include "fpioa.h"
#include "lcd_raw.h"
#include "sysctl.h"
#include "nt35310.h"
#include "timer_lv.h"
#include "lvgl.h"

#define LV_CONF_INCLUDE_SIMPLE 1 1

#include "lv_test_theme/lv_test_theme.h"
#include "lv_test_btn/lv_test_btn.h"
#include "lv_test_chart/lv_test_chart.h"
#include "lv_test_btnm/lv_test_btnm.h"
#include "lv_test_preload/lv_test_preload.h"

#define tick_period_milliseconds 1
uint32_t g_lcd_gram[LCD_X_MAX * LCD_Y_MAX / 2] __attribute__((aligned(128)));

void lv_timer_init();

void lvgl_tick();

void my_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p);

void lv_tutorial_hello_world(void);

void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc);

static lv_disp_drv_t disp_drv;

static void io_set_power(void) {
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
}

static void io_mux_init(void) {
    fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);
}


/* zzx add for test */
static lv_res_t btn_click_action(lv_obj_t * btn)
{
    uint8_t id = lv_obj_get_free_num(btn);

    printf("Button %d is released\n", id);

    /* The button is released.
     * Make something here */

    return LV_RES_OK; /*Return OK if the button is not deleted*/
}

void lv_test_btn_2(void)
{
    /*Create a title label*/
    lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Default buttons");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);

    /*Create a normal button*/
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_cont_set_fit(btn1, true, true); /*Enable resizing horizontally and vertically*/
    lv_obj_align(btn1, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_free_num(btn1, 1);   /*Set a unique number for the button*/
    lv_btn_set_action(btn1, LV_BTN_ACTION_CLICK, btn_click_action);

    /*Add a label to the button*/
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Normal");

    /*Copy the button and set toggled state. (The release action is copied too)*/
    lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), btn1);
    lv_obj_align(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_btn_set_state(btn2, LV_BTN_STATE_TGL_REL);  /*Set toggled state*/
    lv_obj_set_free_num(btn2, 2);               /*Set a unique number for the button*/

    /*Add a label to the toggled button*/
    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");

    /*Copy the button and set inactive state.*/
    lv_obj_t * btn3 = lv_btn_create(lv_scr_act(), btn1);
    lv_obj_align(btn3, btn2, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_btn_set_state(btn3, LV_BTN_STATE_INA);   /*Set inactive state*/
    lv_obj_set_free_num(btn3, 3);               /*Set a unique number for the button*/

    /*Add a label to the inactive button*/
    label = lv_label_create(btn3, NULL);
    lv_label_set_text(label, "Inactive");
}


/*Will be called on click of a button of a list*/
static lv_res_t list_release_action(lv_obj_t * list_btn)
{
    printf("List element click:%s\n", lv_list_get_btn_text(list_btn));

    return LV_RES_OK; /*Return OK because the list is not deleted*/
}
void lv_test_list(void)
{
    /************************
     * Create a default list
     ************************/

    /*Crate the list*/
    lv_obj_t * list1 = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(list1, 130, 170);
    lv_obj_align(list1, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 40);

    /*Add list elements*/
    lv_list_add(list1, SYMBOL_FILE, "New", list_release_action);
    lv_list_add(list1, SYMBOL_DIRECTORY, "Open", list_release_action);
    lv_list_add(list1, SYMBOL_CLOSE, "Delete", list_release_action);
    lv_list_add(list1, SYMBOL_EDIT, "Edit", list_release_action);
    lv_list_add(list1, SYMBOL_SAVE, "Save", list_release_action);

    /*Create a label above the list*/
    lv_obj_t * label;
    label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Default");
    lv_obj_align(label, list1, LV_ALIGN_OUT_TOP_MID, 0, -10);

    /*********************
     * Create new styles
     ********************/
    /*Create a scroll bar style*/
    static lv_style_t style_sb;
    lv_style_copy(&style_sb, &lv_style_plain);
    style_sb.body.main_color = LV_COLOR_BLACK;
    style_sb.body.grad_color = LV_COLOR_BLACK;
    style_sb.body.border.color = LV_COLOR_WHITE;
    style_sb.body.border.width = 1;
    style_sb.body.border.opa = LV_OPA_70;
    style_sb.body.radius = LV_RADIUS_CIRCLE;
    style_sb.body.opa = LV_OPA_60;

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
    style_btn_rel.body.grad_color = LV_COLOR_BLACK;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 1;
    style_btn_rel.body.border.opa = LV_OPA_50;
    style_btn_rel.body.radius = 0;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
    style_btn_pr.body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
    style_btn_pr.text.color = LV_COLOR_MAKE(0xbb, 0xd5, 0xf1);

    /**************************************
     * Create a list with modified styles
     **************************************/

    /*Copy the previous list*/
    lv_obj_t * list2 = lv_list_create(lv_scr_act(),list1);
    lv_obj_align(list2, NULL, LV_ALIGN_IN_TOP_RIGHT, -20, 40);
    lv_list_set_sb_mode(list2, LV_SB_MODE_AUTO);
    lv_list_set_style(list2, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list2, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_list_set_style(list2, LV_LIST_STYLE_BTN_REL, &style_btn_rel); /*Set the new button styles*/
    lv_list_set_style(list2, LV_LIST_STYLE_BTN_PR, &style_btn_pr);

    /*Create a label above the list*/
    label = lv_label_create(lv_scr_act(), label);       /*Copy the previous label*/
    lv_label_set_text(label, "Modified");
    lv_obj_align(label, list2, LV_ALIGN_OUT_TOP_MID, 0, -10);
}

void lv_test_base_obj(void)
{
    /*Create a simple base object*/
    lv_obj_t * obj1;
    obj1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(obj1, 150, 40);
    lv_obj_set_style(obj1, &lv_style_plain_color);
    lv_obj_align(obj1, NULL, LV_ALIGN_IN_TOP_MID, 0, 0/*40*/);

    /*Copy the previous object and enable drag*/
    lv_obj_t * obj2;
    obj2 = lv_obj_create(lv_scr_act(), obj1);
    lv_obj_set_style(obj2, &lv_style_pretty_color);
    lv_obj_set_drag(obj2, true);
    lv_obj_align(obj2, NULL, LV_ALIGN_CENTER, 0, 0);

    static lv_style_t style_shadow;
    lv_style_copy(&style_shadow, &lv_style_pretty);
    style_shadow.body.shadow.width = 6;
    style_shadow.body.radius = LV_RADIUS_CIRCLE;

    /*Copy the previous object (drag is already enabled)*/
    lv_obj_t * obj3;
    obj3 = lv_obj_create(lv_scr_act(), obj2);
    lv_obj_set_style(obj3, &style_shadow);
    lv_obj_align(obj3, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0/*-40*/);
}



/* end */


int main(void) {
    printf("lcd-test test\n");
    io_mux_init();
    io_set_power();

    lv_log_register_print(my_print);

    lv_init();
    lv_timer_init();

    lcd_init();
    printf("init lcd down\n");


    lv_disp_drv_init(&disp_drv);
    printf("init 1\n");
    disp_drv.disp_flush = my_disp_flush;
    printf("init 2\n");
    lv_disp_drv_register(&disp_drv);


    printf("init lvgl_demo driver down\n");


    lcd_clear(YELLOW);
    lcd_draw_picture(0, 0, 240, 160, g_lcd_gram);
    lcd_draw_string(16, 40, "Canaan", RED);
    lcd_draw_string(16, 80, "Kendryte K210", BLUE);

//    lv_test_preload_1();
//    lv_tutorial_hello_world();
//    lv_test_chart_1();
//    lv_test_btnm_1();
//    lv_test_btn_1();
//    lv_theme_t *th = lv_theme_alien_init(21, NULL);
//    lv_theme_t *th = lv_theme_night_init(21, NULL);
//    lv_test_theme_1(th);
//    lv_test_btn_2();
//    lv_test_list();
    lv_test_base_obj();
    lv_task_handler();
    while(1);
//    while (1) {
//        lv_task_handler();
//        msleep(10);
//    }
}

void lv_timer_init() {
    init_timer_lv(lvgl_tick, tick_period_milliseconds);

}

void lvgl_tick() {
    lv_tick_inc(tick_period_milliseconds);
}

void my_disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_p) {
    lcd_draw_picture_by_half((uint16_t) x1, (uint16_t) y1, (uint16_t) (x2 - x1 + 1), (uint16_t) (y2 - y1 + 1),
                             (uint16_t *) color_p);
    lv_flush_ready();
}

void lv_tutorial_hello_world(void) {
    lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);

    lv_label_set_text(label1, "Hello world!");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
}


void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc) {
    printf("level:[%d],file[%s],line:%d\t%s", level, file, line, dsc);
}
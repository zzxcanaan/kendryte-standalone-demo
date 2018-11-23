//
// Created by jiangxingquan on 2018/11/18.
//

#include <stdio.h>
#include <plic.h>
#include <timer.h>
#include <sysctl.h>
#include "timer_lv.h"


void init_timer_lv(void(*func)(), uint32_t milliseconds) {
    plic_init();
    timer_init(TIMER_DEVICE_0);
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, milliseconds * 1e6);
    timer_set_irq(TIMER_DEVICE_0, TIMER_CHANNEL_0, func, 1);
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    sysctl_enable_irq();
}

#include "reg.h"

static void init_led(void);
static unsigned long long get_time(void);

static void init_led(void)
{
    /* OE:output enable */
    SIO_GPIO_OE_CLR = 1 << 10;
    SIO_GPIO_OUT_CLR = 1 << 10;

    /* GPIOとして使う為の設定 */
    IO_BANK0_GPIO10_CTRL_RW = 5;
    PADS_BANK0_BASE_GPIO10_CLR = 1 << 8;

    SIO_GPIO_OE_SET = 1 << 10;
}

static unsigned long long get_time(void)
{
    volatile unsigned long time_l = (TIMER0_TIMELR);
    volatile unsigned long time_h = (TIMER0_TIMEHR);
    return ((unsigned long long)time_h << 32u) | time_l;
}

static void wait_ms(unsigned long time_ms)
{
    unsigned long long target_time;
    unsigned long long current_time;

    current_time = get_time();
    target_time = current_time + (time_ms * 1000);

    while (target_time > current_time)
    {
        current_time = get_time();
    }
    return;
}

void main(void)
{

    init_led();

    while (1)
    {
        SIO_GPIO_OUT_SET = 1 << 10;
        wait_ms(250);
        SIO_GPIO_OUT_CLR = 1 << 10;
        wait_ms(250);
    }
}
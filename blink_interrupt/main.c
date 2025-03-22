#include "reg.h"
#include "hardware/irq.h"

static void init_led(void);
static unsigned long long get_time(void);
static void reload_alarm0(void);
static void timer_interrupt(void);
static void init_timer(void);

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

static void reload_alarm0(void)
{
    TIMER0_ALARM0 = (unsigned long)(get_time() + 250000); // 250ms
}

static void timer_interrupt(void)
{
    reload_alarm0();      // 次の割り込みを設定
    TIMER0_INTR = 1 << 0; // 割り込みフラグをクリア

    if ((SIO_GPIO_OUT_RW >> 10) & 1)
    {
        // LEDが点灯している場合
        SIO_GPIO_OUT_CLR = 1 << 10; // LEDを消灯
    }
    else
    {
        // LEDが消灯している場合
        SIO_GPIO_OUT_SET = 1 << 10; // LEDを点灯
    }
}

static void init_timer(void)
{
    irq_set_exclusive_handler(0, timer_interrupt); // 割り込みハンドラを設定
    irq_set_enabled(0, 1);                         // 割り込みを有効化
    reload_alarm0();                               // 最初の割り込みを設定
    TIMER0_INTE = 1 << 0;                          // タイマー割り込みを有効化
}

void main(void)
{
    init_led();
    init_timer();

    while (1)
    {
        // 割り込みがない時に別の処理ができる
    }
}
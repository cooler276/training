#include "reg.h"
#include "hardware/irq.h"

typedef struct
{
    unsigned char cycle_period;
    unsigned char cycle_counter;
    unsigned char duty_period;
    unsigned char duty_counter;
} software_pwm;

software_pwm SoftPwm;

static void init_led(void);
static unsigned long long get_time(void);
static void reload_alarm0(void);
static void timer_interrupt(void);
static void init_timer(void);
static int software_pwm_update(software_pwm *sp);

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
    volatile unsigned long time_l = (TIMER0_TIMELR);     // 下位32bit
    volatile unsigned long time_h = (TIMER0_TIMEHR);     // 上位32bit
    return ((unsigned long long)time_h << 32u) | time_l; // 64bitに結合
}

static void reload_alarm0(void)
{
    TIMER0_ALARM0 = (unsigned long)(get_time() + 100); // 100クロック後に割り込み
}

static void timer_interrupt(void)
{
    int ret = 0;
    reload_alarm0();                     // 次の割り込みを設定
    TIMER0_INTR = 1 << 0;                // 割り込みフラグをクリア
    ret = software_pwm_update(&SoftPwm); // ソフトウェアPWMの更新
    if (ret == 1)
    {
        // 周期が終わったら
        SoftPwm.duty_period++;                          // デューティー比をインクリメント
        if (SoftPwm.duty_period > SoftPwm.cycle_period) // デューティー比が周期を超えたら
        {
            SoftPwm.duty_period = 0; // デューティー比をリセット
        }
    }
}

static void init_timer(void)
{
    irq_set_exclusive_handler(0, timer_interrupt); // 割り込みハンドラを設定
    irq_set_enabled(0, 1);                         // 割り込みを有効化
    reload_alarm0();                               // 最初の割り込みを設定
    TIMER0_INTE = 1 << 0;                          // タイマー割り込みを有効化
}

static int software_pwm_update(software_pwm *sp)
{
    int ret = 0;
    if (sp->cycle_counter > sp->cycle_period)
    {
        // 周期が終わったら
        sp->cycle_counter = 0; // 周期をリセット
        sp->duty_counter = 0;  // デューティー比をリセット
        ret = 1;               // フラグを立てる
    }
    else
    {
        // 周期中なら
        sp->cycle_counter++; // 周期をインクリメント
        sp->duty_counter++;  // デューティー比をインクリメント
        if (sp->duty_counter > sp->duty_period)
        {
            // デューティー比を超えたら
            SIO_GPIO_OUT_CLR = 1 << 10; // LEDを消灯
        }
        else
        {
            // デューティー比中なら
            SIO_GPIO_OUT_SET = 1 << 10; // LEDを点灯
        }
    }
    return ret;
}

void main(void)
{
    SoftPwm.cycle_period = 200; // 20ms
    SoftPwm.cycle_counter = 0;
    SoftPwm.duty_period = 0;
    SoftPwm.duty_counter = 0;

    init_led();
    init_timer();

    while (1)
    {
        /* Do Nothing */
    }
}
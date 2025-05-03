#include "reg.h"
#include "hardware/irq.h"

// ソフトウェアPWM制御用の構造体
typedef struct
{
    unsigned char cycle_period;  // PWM周期 (単位: 割り込み回数)
    unsigned char cycle_counter; // 現在の周期カウンタ
    unsigned char duty_period;   // HIGHレベルの期間 (デューティー比、単位: 割り込み回数)
    unsigned char duty_counter;  // 現在のデューティー比カウンタ
} software_pwm;

software_pwm SoftPwm; // ソフトウェアPWM制御構造体のインスタンス

// LEDの初期化を行う関数
static void init_led(void);
// 現在の時間を取得する関数
static unsigned long long get_time(void);
// タイマー0のアラーム0を再設定する関数
static void reload_alarm0(void);
// タイマー割り込みが発生した際に実行される関数
static void timer_interrupt(void);
// タイマー関連の初期化を行う関数
static void init_timer(void);
// ソフトウェアPWMのデューティー比を更新し、LEDを制御する関数
static int software_pwm_update(software_pwm *sp);

// LEDの初期化を行う関数
static void init_led(void)
{
    /* OE:output enable */
    // GPIO10番ピンの出力機能を無効にする (最初は出力を止めておく)
    SIO_GPIO_OE_CLR = 1 << 10;
    // GPIO10番ピンの出力をクリア (LOWレベルにする)
    SIO_GPIO_OUT_CLR = 1 << 10;

    /* GPIOとして使う為の設定 */
    // IOバンク0のGPIO10番ピンをGPIO機能として設定 (値5はGPIOモードを示す)
    IO_BANK0_GPIO10_CTRL_RW = 5;
    // IOバンク0のGPIO10番ピンのプルアップ/プルダウン抵抗などをクリア (デフォルト設定にする)
    PADS_BANK0_BASE_GPIO10_CLR = 1 << 8;

    // GPIO10番ピンの出力機能を有効にする (ここで初めて出力が可能になる)
    SIO_GPIO_OE_SET = 1 << 10;
}

// 現在の時間を取得する関数 (64ビットのunsigned long long型で返す)
static unsigned long long get_time(void)
{
    volatile unsigned long time_l = (TIMER0_TIMELR);     // 下位32bit
    volatile unsigned long time_h = (TIMER0_TIMEHR);     // 上位32bit
    return ((unsigned long long)time_h << 32u) | time_l; // 64bitに結合
}

// タイマー0のアラーム0を再設定する関数
static void reload_alarm0(void)
{
    TIMER0_ALARM0 = (unsigned long)(get_time() + 100); // 100クロック後に割り込みが発生するように設定
}

// タイマー割り込みが発生した際に実行される関数
static void timer_interrupt(void)
{
    int ret = 0;
    reload_alarm0();                     // 次の割り込みタイミングを設定
    TIMER0_INTR = 1 << 0;                // タイマー0の割り込みフラグをクリア (これを行わないと割り込みが止まらない)
    ret = software_pwm_update(&SoftPwm); // ソフトウェアPWMのデューティー比を更新し、LEDを制御する
    if (ret == 1)
    {
        // 周期が終了した場合
        SoftPwm.duty_period++;                          // デューティー比をインクリメント
        if (SoftPwm.duty_period > SoftPwm.cycle_period) // デューティー比が周期を超えた場合
        {
            SoftPwm.duty_period = 0; // デューティー比をリセット
        }
    }
}

// タイマー関連の初期化を行う関数
static void init_timer(void)
{
    irq_set_exclusive_handler(0, timer_interrupt); // 割り込み番号0にタイマー割り込みハンドラ (timer_interrupt関数) を設定
    irq_set_enabled(0, 1);                         // 割り込み番号0を有効にする
    reload_alarm0();                               // 最初の割り込みタイミングを設定
    TIMER0_INTE = 1 << 0;                          // タイマー0のアラーム0割り込みを有効にする
}

// ソフトウェアPWMのデューティー比を更新し、LEDを制御する関数
static int software_pwm_update(software_pwm *sp)
{
    int ret = 0;
    if (sp->cycle_counter > sp->cycle_period)
    {
        // 周期が終了した場合
        sp->cycle_counter = 0; // 周期カウンタをリセット
        sp->duty_counter = 0;  // デューティー比カウンタをリセット
        ret = 1;               // 周期終了フラグを立てる
    }
    else
    {
        // 周期内の場合
        sp->cycle_counter++; // 周期カウンタをインクリメント
        sp->duty_counter++;  // デューティー比カウンタをインクリメント
        if (sp->duty_counter > sp->duty_period)
        {
            // デューティー比期間が終了した場合 (LOWレベル出力)
            SIO_GPIO_OUT_CLR = 1 << 10; // LEDを消灯
        }
        else
        {
            // デューティー比期間内の場合 (HIGHレベル出力)
            SIO_GPIO_OUT_SET = 1 << 10; // LEDを点灯
        }
    }
    return ret;
}

// メイン関数 (プログラムのエントリーポイント)
void main(void)
{
    SoftPwm.cycle_period = 200; // PWM周期を200に設定 (タイマー割り込み100クロック毎なので、200*100クロック = 20ms)
    SoftPwm.cycle_counter = 0;
    SoftPwm.duty_period = 0; // 初期デューティー比は0%
    SoftPwm.duty_counter = 0;

    init_led();   // LEDの初期化
    init_timer(); // タイマーの初期化

    while (1)
    {
        /* Do Nothing */
        // 割り込み処理でPWM制御を行うため、メインループでは何も処理を行わない
    }
}

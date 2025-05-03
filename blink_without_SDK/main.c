#include "reg.h"

// LEDの初期化を行う関数
static void
init_led(void);
// 現在の時間を取得する関数
static unsigned long long get_time(void);
// 指定したミリ秒だけ待機する関数
static void wait_ms(unsigned long time_ms);

// LEDの初期化を行う関数
static void init_led(void)
{
    /* OE: output enable (出力有効) */
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
    // タイマー0の下位32ビットの値を読み込む (volatile指定で最適化を防ぐ)
    volatile unsigned long time_l = (TIMER0_TIMELR);
    // タイマー0の上位32ビットの値を読み込む (volatile指定で最適化を防ぐ)
    volatile unsigned long time_h = (TIMER0_TIMEHR);
    // 上位32ビットを左に32ビットシフトし、下位32ビットと論理和を取ることで64ビットの値を合成する
    return ((unsigned long long)time_h << 32u) | time_l;
}

// 指定したミリ秒だけ待機する関数
static void wait_ms(unsigned long time_ms)
{
    unsigned long long target_time;  // 目標時間
    unsigned long long current_time; // 現在の時間

    // 現在の時間を取得
    current_time = get_time();
    // 目標時間を計算 (現在の時間に、待ちたいミリ秒をマイクロ秒に変換して加える)
    target_time = current_time + (time_ms * 1000);

    // 現在時間が目標時間に達するまでループ
    while (target_time > current_time)
    {
        // ループごとに現在の時間を再取得
        current_time = get_time();
    }
    return; // 待ち時間終了
}

// メイン関数 (プログラムのエントリーポイント)
void main(void)
{
    // LEDの初期化
    init_led();

    // 無限ループ (プログラムを永遠に繰り返す)
    while (1)
    {
        // GPIO10番ピンの出力をセット (HIGHレベルにする -> LED点灯)
        SIO_GPIO_OUT_SET = 1 << 10;
        // 250ミリ秒待機
        wait_ms(250);
        // GPIO10番ピンの出力をクリア (LOWレベルにする -> LED消灯)
        SIO_GPIO_OUT_CLR = 1 << 10;
        // 250ミリ秒待機
        wait_ms(250);
    }
}
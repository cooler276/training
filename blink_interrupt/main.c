#include "reg.h"          // レジスタ定義のヘッダーファイル
#include "hardware/irq.h" // 割り込み関連のヘッダーファイル

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

// タイマー0のアラーム0を再設定する関数
static void reload_alarm0(void)
{
    // 現在の時間に250000マイクロ秒 (250ミリ秒) を加えた値を、アラーム0に設定する
    TIMER0_ALARM0 = (unsigned long)(get_time() + 250000);
}

// タイマー割り込みが発生した際に実行される関数
static void timer_interrupt(void)
{
    reload_alarm0();      // 次の割り込みタイミングを設定する
    TIMER0_INTR = 1 << 0; // タイマー0の割り込みフラグをクリアする (これを行わないと、割り込みが止まらない)

    // GPIO10番ピンの出力状態をチェックする
    if ((SIO_GPIO_OUT_RW >> 10) & 1)
    {
        // GPIO10番ピンの10ビット目が1 (HIGH、LEDが点灯している) 場合
        SIO_GPIO_OUT_CLR = 1 << 10; // GPIO10番ピンをクリア (LOW、LEDを消灯する)
    }
    else
    {
        // GPIO10番ピンの10ビット目が0 (LOW、LEDが消灯している) 場合
        SIO_GPIO_OUT_SET = 1 << 10; // GPIO10番ピンをセット (HIGH、LEDを点灯する)
    }
}

// タイマー関連の初期化を行う関数
static void init_timer(void)
{
    // 割り込み番号0に、タイマー割り込みハンドラ (timer_interrupt関数) を設定する
    irq_set_exclusive_handler(0, timer_interrupt);
    // 割り込み番号0を有効にする
    irq_set_enabled(0, 1);
    // 最初の割り込みタイミングを設定する
    reload_alarm0();
    // タイマー0のアラーム0割り込みを有効にする
    TIMER0_INTE = 1 << 0;
}

// メイン関数 (プログラムのエントリーポイント)
void main(void)
{
    // LEDの初期化
    init_led();
    // タイマーの初期化 (割り込み設定を含む)
    init_timer();

    // 無限ループ (割り込みドリブンなので、ここでは主に待機する)
    while (1)
    {
        // 割り込みが発生していない間は、ここで別の処理を実行できる
        // 例: 他のセンサーの値を読む、通信を行う、など
    }
}

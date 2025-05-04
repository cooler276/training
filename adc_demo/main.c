#include <stdio.h>          // 標準入出力ライブラリ (printf など)
#include "pico/stdlib.h"    // Pico SDK の標準ライブラリ (GPIO、クロックなど)
#include "hardware/adc.h"   // ハードウェアADC (アナログ-デジタル変換) 機能のライブラリ
#include "hardware/timer.h" // ハードウェアタイマー機能のライブラリ

// 読み取るADチャネルを定義
// 0: GP26 (ADC0) 照度センサ
// 1: GP27 (ADC1) ボリューム
// 2: GP28 (ADC2) マイク
#define ADC_CHANNEL 0

// タイマー割り込み周期 (マイクロ秒)
#define TIMER_INTERVAL_US 100000 // 100ms

// volatile キーワード：
// 割り込みハンドラでの変更がメインループで反映されない、といった問題を防ぐ。
volatile bool timer_flag = false; // タイマー割り込みが発生したことを示すフラグ

// タイマー割り込み関数
// この関数は、設定されたタイマーの周期ごとに実行される。
bool repeating_timer_callback(struct repeating_timer *rt)
{
    timer_flag = true; // タイマー割り込みフラグを立てる
    return true;       // タイマーを継続させる (繰り返し実行する)
}

int main()
{
    // 標準入出力の初期化 (通常はシリアルポート)
    stdio_init_all();

    // ADC (アナログ-デジタル変換器) の初期化
    adc_init();

    // ADCを使用するGPIOピンの設定
    // PicoのGPIOは、様々な機能に使用できる。
    // ここでは、GP26, GP27, GP28をアナログ入力として設定する。
    //  - gpio_set_function(ピン番号, 機能): ピンの機能を設定する。
    //  - gpio_set_dir(ピン番号, 方向): ピンの方向を設定する (入力または出力)。
    gpio_set_function(26, GPIO_FUNC_SIO); // GP26をSIO (標準入出力) 機能に設定
    gpio_set_dir(26, GPIO_IN);            // GP26を入力に設定
    gpio_set_function(27, GPIO_FUNC_SIO); // GP27をSIO機能に設定
    gpio_set_dir(27, GPIO_IN);            // GP27を入力に設定
    gpio_set_function(28, GPIO_FUNC_SIO); // GP28をSIO機能に設定
    gpio_set_dir(28, GPIO_IN);            // GP28を入力に設定

    // 選択されたADチャネルを設定
    // ADCには複数の入力チャネルがあり、どれを読み取るかを指定する必要がある。
    if (ADC_CHANNEL == 0)
    {
        adc_select_input(0); // ADC0 (GP26) を選択
    }
    else if (ADC_CHANNEL == 1)
    {
        adc_select_input(1); // ADC1 (GP27) を選択
    }
    else if (ADC_CHANNEL == 2)
    {
        adc_select_input(2); // ADC2 (GP28) を選択
    }
    else
    {
        // 定義されていないADC_CHANNELが指定された場合は、エラーとしてプログラムを終了する。
        return 1;
    }

    // タイマーの設定
    struct repeating_timer timer; // タイマー構造体を宣言
    // 繰り返しタイマーを設定する関数
    // - 第1引数: 繰り返し間隔 (マイクロ秒)。
    // - 第2引数: コールバック関数 (タイマー割り込み時に実行される関数)
    // - 第3引数: コールバック関数に渡すユーザーデータ (ここではNULL)
    // - 第4引数: 設定するタイマー構造体へのポインタ
    add_repeating_timer_us(TIMER_INTERVAL_US, repeating_timer_callback, NULL, &timer);

    // メインループ
    while (true)
    {
        // タイマー割り込みフラグが立っているか確認
        if (timer_flag)
        {
            timer_flag = false; // フラグをクリア (次の割り込みを待つ)

            // AD値の読み取り
            // adc_read()関数は、選択されているADCチャネルの電圧をデジタル値として読み取る。
            // 戻り値は12ビットの符号なし整数 (0～4095) 。
            uint16_t raw = adc_read();
            printf("AD Value: %d\n", raw); // 読み取った値を表示
        }
    }

    return 0;
}

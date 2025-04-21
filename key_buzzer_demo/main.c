#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// GPIOピンの定義
#define BUTTON_PIN 3  // ボタンが接続されているGPIOピン番号
#define BUZZER_PIN 12 // ブザーが接続されているGPIOピン番号

// ラの音（A4）を再生する関数
// 引数: slice_num - PWMスライス番号
void play_note_a(uint slice_num)
{
    int freq = 440; // ラの音（A4）の周波数（440Hz）

    // PWMクロックを設定
    // クロック分周比を設定してPWMの動作速度を調整
    pwm_set_clkdiv(slice_num, 125.0f);

    // PWMの周期を設定
    // 周波数に基づいてPWMの周期を計算
    pwm_set_wrap(slice_num, 125000 / freq);

    // PWMのデューティサイクルを設定
    // デューティサイクルを50%に設定して音を生成
    pwm_set_chan_level(slice_num, PWM_CHAN_A, (125000 / freq) * 0.5);
}

int main()
{
    // 標準入出力を初期化（デバッグ用）
    stdio_init_all();

    // ボタン用GPIOの初期化
    gpio_init(BUTTON_PIN);             // GPIOピンを初期化
    gpio_set_dir(BUTTON_PIN, GPIO_IN); // ボタンは入力モードに設定
    gpio_pull_up(BUTTON_PIN);          // プルアップ抵抗を有効化（ボタンが押されていないときはHIGH）

    // ブザー用GPIOの初期化
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);       // ブザーをPWM機能に設定
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN); // PWMスライス番号を取得

    // PWMの設定
    pwm_config config = pwm_get_default_config(); // デフォルトのPWM設定を取得
    pwm_init(slice_num, &config, true);           // PWMを初期化して有効化

    // メインループ
    while (true)
    {
        // ボタンが押されたかどうかをチェック
        if (gpio_get(BUTTON_PIN) == 0) // ボタンが押されるとGPIOピンがLOWになる
        {
            play_note_a(slice_num); // ラの音を再生
        }
        else
        {
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 0); // ブザーをOFF（デューティサイクルを0に設定）
        }

        // デバウンス用の短い遅延
        // ボタンのチャタリングを防ぐために10msの遅延を挿入
        sleep_ms(10);
    }
}
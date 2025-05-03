# 概要

* 周期のタイマー割り込みを利用してボタンの状態を定期的に取得し、メインループでその状態に応じてブザーのPWM出力を制御する。

# 動作
## 初期化

1. ボタンを接続した GPIO ピンを入力モードに設定する。
2. ブザーを接続した GPIO ピンを PWM 機能に設定する。
3. pwm_init(slice_num, &config, true) 関数で、PWM を初期化し、有効にする。
4. pwm_set_chan_level(slice_num, PWM_CHAN_A, BUZZER_OFF) 関数で、ブザーの初期デューティサイクルを 0 に設定し、音を鳴らさない状態にする。
5. add_repeating_timer_ms(TIMER_INTERVAL_MS, timer_callback, NULL, &timer) 関数で、10ms 周期のタイマー割り込みを設定し、timer_callback 関数が定期的に実行されるようにする。

## 割り込み処理

1. 設定されたタイマーの周期 (TIMER_INTERVAL_MS) ごとに、timer_callback() 関数が実行される。
2. timer_callback() 関数内で、gpio_get(BUTTON_PIN) 関数を呼び出してボタンの状態を読み取る。
3. ボタンが押されている（入力レベルが LOW）場合、グローバル変数 button_pressed を true に設定する。<br>ボタンが押されていない（入力レベルが HIGH）場合、グローバル変数 button_pressed を false に設定する。

## メインループ

1. main() 関数は、while(true) の無限ループに入る。
2. ループ内で、グローバル変数 button_pressed の値をチェックする。
3. button_pressed が true の場合、ブザーを鳴らす。<br>button_pressed が false の場合、ブザーを停止する。

# 補足

* **PWMスライス:** PWM信号を生成できるハードウェアモジュール。各PWMスライスは、それぞれ個別の周波数やデューティサイクルを設定可能。`pwm_gpio_to_slice_num()` 関数は、指定されたGPIOピンに接続されているPWMスライスの番号を取得する。
    ```c
    // ブザー用GPIOの初期化
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    ```
* **PWMの周波数と周期:** `play_note_a` 関数では、再生音の周波数に基づきPWMの周期を設定する。`pwm_set_clkdiv()` でPWMのクロック分周比を設定し、`pwm_set_wrap()` で1周期のカウント値を設定することで、最終的なPWM信号の周波数が決定する。
    ```c
    void play_note_a(uint slice_num)
    {
        int freq = 220; // ラの音（A3）の周波数（220Hz）

        // pwm_set_clkdiv()はPWMのクロック分周比を設定する関数
        // 125.0fはPWMのクロック分周比（125MHz / 125 = 1MHz）を指定している
        pwm_set_clkdiv(slice_num, 125.0f);

        // PWMの周期を設定
        // pwm_set_wrap()はPWMの周期を設定する関数
        pwm_set_wrap(slice_num, 125000 / freq);

        // pwm_set_chan_level()はPWMのデューティサイクルを設定する関数
        pwm_set_chan_level(slice_num, PWM_CHAN_A, (125000 / freq) * BUZZER_ON);
    }
    ```
* **タイマー割り込み:** 10ms周期で発生するタイマー割り込みにより、ボタンの状態を定期的にチェックする。これにより、メインループはボタンの状態変化をリアルタイムに近い形で検知し、応答できる。
    ```c
    // タイマー割り込みの周期 (ms)
    #define TIMER_INTERVAL_MS 10

    // タイマーの設定
    struct repeating_timer timer;
    add_repeating_timer_ms(TIMER_INTERVAL_MS, timer_callback, NULL, &timer);
    ```
* **volatileキーワード:** グローバル変数 `button_pressed` は `volatile` キーワードで修飾されている。そうすることで最適化による変化を防ぐ。
    ```c
    // ボタンの状態を格納するグローバル変数（volatileキーワードで最適化を防ぐ）
    volatile bool button_pressed = false;
    ```

* **CMakeLists.txt:** ライブラリの追加設定 `target_link_libraries`に `hardware_pwm` と `hardware_timer` を追加することに注意。
    ```
    target_link_libraries(key_buzzer_demo
        hardware_pwm
        hardware_timer
        pico_stdlib)
    ```
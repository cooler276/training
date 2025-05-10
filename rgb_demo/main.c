#include <stdio.h>           // 標準入出力ライブラリ（printf などを使うため）
#include <math.h>            // 数学関数ライブラリ（sin, cos などを使う場合がある。ここでは fmodf, floorf を使用）
#include "pico/stdlib.h"     // PICO SDK の基本的な機能を使うためのヘッダファイル
#include "hardware/pio.h"    // PICO の Programmable I/O (PIO) 機能を使うためのヘッダファイル
#include "hardware/clocks.h" // PICO のクロック制御機能を使うためのヘッダファイル
#include "ws2812.pio.h"      // WS2812 (RGB LED) を制御するための PIO プログラムのヘッダファイル

// 設定: RGBW（ホワイト）チャンネルを持つLEDを使うかどうか。ここでは使わないので false
#define IS_RGBW false
// 設定: WS2812 のデータ信号を接続する GPIO ピンの番号
#define WS2812_PIN 22

// 関数: HSV (Hue:色相, Saturation:彩度, Value:明度) カラーモデルを RGB (Red:赤, Green:緑, Blue:青) カラーモデルに変換します
// h: 色相 (0-360度の範囲)
// s: 彩度 (0.0-1.0 の範囲)
// v: 明度 (0.0-1.0 の範囲)
// *r, *g, *b: 変換された赤、緑、青の値を格納するポインタ (0-255 の範囲)
void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // 彩度が 0 の場合、色はグレーになります
    if (s == 0.0f)
    {
        *r = *g = *b = (uint8_t)(v * 255.0f); // 明度をスケールして RGB 全てに同じ値を設定
    }
    else
    {
        // 色相を 0-360度の範囲に調整
        float hue = fmodf(h, 360.0f);
        if (hue < 0)
        {
            hue += 360.0f;
        }
        // 色相を 6 つのセクターに分割 (各 60 度)
        float sector = hue / 60.0f;
        // 現在のセクターの整数部分
        int i = floorf(sector);
        // セクター内の小数部分
        float f = sector - i;
        // 一時的な計算用変数
        float p = v * (1 - s);
        float q = v * (1 - s * f);
        float t = v * (1 - s * (1 - f));

        // セクターに応じて RGB の値を計算
        switch (i)
        {
        case 0: // 赤から黄色
            *r = (uint8_t)(v * 255.0f);
            *g = (uint8_t)(t * 255.0f);
            *b = (uint8_t)(p * 255.0f);
            break;
        case 1: // 黄色から緑
            *r = (uint8_t)(q * 255.0f);
            *g = (uint8_t)(v * 255.0f);
            *b = (uint8_t)(p * 255.0f);
            break;
        case 2: // 緑からシアン
            *r = (uint8_t)(p * 255.0f);
            *g = (uint8_t)(v * 255.0f);
            *b = (uint8_t)(t * 255.0f);
            break;
        case 3: // シアンから青
            *r = (uint8_t)(p * 255.0f);
            *g = (uint8_t)(q * 255.0f);
            *b = (uint8_t)(v * 255.0f);
            break;
        case 4: // 青からマゼンタ
            *r = (uint8_t)(t * 255.0f);
            *g = (uint8_t)(p * 255.0f);
            *b = (uint8_t)(v * 255.0f);
            break;
        default: // マゼンタから赤
            *r = (uint8_t)(v * 255.0f);
            *g = (uint8_t)(p * 255.0f);
            *b = (uint8_t)(q * 255.0f);
            break;
        }
    }
}

// 関数: RGB のデータを WS2812 LED に送信します
// pio: 使用する PIO コントローラのインスタンス
// sm: 使用するステートマシンの番号
// r, g, b: 赤、緑、青の色の値 (0-255 の範囲)
void ws2812_send_pixel(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b)
{
    // WS2812 は GRB の順で色データを受け取るため、ここで順番を入れ替えて 32bit のデータを作成
    uint32_t pixel = ((g << 16) | (r << 8) | b);
    // PIO ステートマシンにデータを送信 (blocking なので送信完了まで待ちます)
    pio_sm_put_blocking(pio, sm, pixel << 8u); // 8bit 左シフトすることで、PIO プログラムのデータ形式に合わせます
}

int main()
{
    // 標準入出力 (USB シリアルなど) を初期化します
    stdio_init_all();
    // 使用する PIO コントローラのインスタンスを取得 (PICO には PIO0 と PIO1 の 2 つがあります)
    PIO pio = pio0;
    // 使用するステートマシンの番号 (各 PIO コントローラには複数のステートマシンがあります)
    uint sm = 0;

    // WS2812 を制御するための PIO プログラムをロードし、そのオフセット (メモリ上の位置) を取得します
    uint offset = pio_add_program(pio, &ws2812_program);
    // ロードした PIO プログラムを初期化します
    // pio: 使用する PIO コントローラ
    // sm: 使用するステートマシン
    // offset: ロードしたプログラムのオフセット
    // WS2812_PIN: データピン
    // 800000: WS2812 の通信速度 (800kHz)
    // IS_RGBW: RGBW モードかどうか
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // グラデーションの基準となる色相 (Hue) を定義します (0-360度の範囲)。
    // ここでは、赤(0), 黄(60), 緑(120), シアン(180), 青(240), マゼンタ(300) を基準としています。
    const float colors_hsv[] = {0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f};
    // 定義した色相の数を計算
    const int num_hues = sizeof(colors_hsv) / sizeof(colors_hsv[(0)]);
    // 現在表示している基準色のインデックス
    int current_hue_index = 0;
    // 彩度 (色の鮮やかさ)。1.0 で最大の色鮮やかさになります。
    const float saturation = 1.0f;
    // 明度 (色の明るさ)。1.0 で最大の明るさになります。
    const float value = 1.0f;
    // グラデーションのステップ数。この数が多いほど滑らかなグラデーションになりますが、変化が遅くなります。
    const int gradient_steps = 100;
    // グラデーションの各ステップ間の遅延時間 (ミリ秒)。この値が小さいほど変化が速くなります。
    const uint32_t gradient_delay = 5;

    // 無限ループ
    while (1)
    {
        // 現在の基準色の色相を取得
        float start_hue = colors_hsv[current_hue_index];
        // 次の基準色のインデックスを計算 (配列の最後まできたら最初に戻る)
        int next_hue_index = (current_hue_index + 1) % num_hues;
        // 次の基準色の色相を取得
        float end_hue = colors_hsv[next_hue_index];

        // 現在の色から次の色へ、色相を滑らかに変化させてグラデーション表示
        for (int i = 0; i <= gradient_steps; ++i)
        {
            // 現在のグラデーションステップにおける色相を計算
            float current_hue = start_hue + (end_hue - start_hue) * i / gradient_steps;
            // HSV を RGB に変換
            uint8_t r, g, b;
            hsv_to_rgb(current_hue, saturation, value, &r, &g, &b);
            // 変換した RGB 値を LED に送信
            ws2812_send_pixel(pio, sm, r, g, b);
            // グラデーションの速度を調整するための遅延
            sleep_ms(gradient_delay);
        }

        // 次のグラデーションのために現在の基準色のインデックスを更新
        current_hue_index = next_hue_index;
    }

    // プログラムが正常に終了した場合の戻り値 (通常は 0)
    return 0;
}
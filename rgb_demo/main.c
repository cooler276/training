#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h" // ws2812.pio のヘッダファイルをインクルード

#define IS_RGBW false // RGBW（ホワイト）を使うかどうか。通常はfalse（RGBモード）
#define WS2812_PIN 22 // WS2812のデータラインを接続するGPIOピン

// RGBデータをWS2812に送信する関数
void ws2812_send_pixel(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b)
{
    // RGB順でデータをセット。GRB順で送信するため、g, r, bの順にシフトして格納。
    uint32_t pixel = ((g << 16) | (r << 8) | b);

    // 送信するピクセルデータをPIOに送信
    pio_sm_put_blocking(pio, sm, pixel << 8u); // GRBフォーマットでデータを送信
}

int main()
{
    stdio_init_all(); // 標準入出力の初期化
    PIO pio = pio0;   // PIO0を使用
    uint sm = 0;      // ステートマシン0を使用

    // PIOプログラムのオフセットを取得
    uint offset = pio_add_program(pio, &ws2812_program);

    // PIOプログラムを初期化、WS2812ピンに800kHzの信号を送信する設定
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    while (1)
    {
        // 赤色を送信
        ws2812_send_pixel(pio, sm, 255, 0, 0);
        sleep_ms(500); // 500ms待機

        // 緑色を送信
        ws2812_send_pixel(pio, sm, 0, 255, 0);
        sleep_ms(500); // 500ms待機

        // 青色を送信
        ws2812_send_pixel(pio, sm, 0, 0, 255);
        sleep_ms(500); // 500ms待機
    }

    return 0; // プログラム終了
}
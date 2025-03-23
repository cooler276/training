
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* 定義 */
#define I2C_SDA_PIN 6                                          // SDAピン
#define I2C_SCL_PIN 7                                          // SCLピン
#define I2C_SPEED 1000000                                      // I2Cの速度（1MHz）
#define SSD1327_ADDR 0x3D                                      // SSD1327のI2Cアドレス（0x3C or 0x3D）
#define DISPLAY_WIDTH 128                                      // ディスプレイの幅
#define DISPLAY_HEIGHT 128                                     // ディスプレイの高さ
#define DISPLAY_DATA_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 2) // ディスプレイのデータサイズ

/* グローバル変数 */
i2c_inst_t *i2c = i2c1; // 使用するI2Cインスタンス
uint8_t buffer[DISPLAY_DATA_SIZE];

/* プロトタイプ宣言 */
static void i2c_init_pico();
static void ssd1327_send_command(uint8_t command);                                                  // コマンド送信関数
static void ssd1327_send_data(uint8_t data);                                                        // データ送信関数
static void ssd1327_set_window(uint16_t X_start, uint16_t Y_start, uint16_t X_end, uint16_t Y_end); // ウィンドウ設定関数
static void ssd1327_init();                                                                         // 初期化関数
static void ssd1327_set_display(uint8_t *data);                                                     // ディスプレイ設定関数

/* 関数 */
static void i2c_init_pico()
{
    i2c_init(i2c, I2C_SPEED);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

// コマンド送信関数
static void ssd1327_send_command(uint8_t command)
{
    uint8_t buffer[2] = {0x00, command}; // 0x00はコマンドモードを示す
    i2c_write_blocking(i2c, SSD1327_ADDR, buffer, 2, false);
}

// データ送信関数
static void ssd1327_send_data(uint8_t data)
{
    uint8_t buffer[2] = {0x40, data}; // 0x40はデータモードを示す
    i2c_write_blocking(i2c, SSD1327_ADDR, buffer, 2, false);
}

static void ssd1327_set_window(uint16_t X_start, uint16_t Y_start, uint16_t X_end, uint16_t Y_end)
{
    // 座標がディスプレイ範囲内かどうかをチェック
    if (X_start >= DISPLAY_WIDTH || Y_start >= DISPLAY_HEIGHT ||
        X_end >= DISPLAY_WIDTH || Y_end >= DISPLAY_HEIGHT)
    {
        return; // 範囲外の場合は何もしない
    }

    // コラム（X座標）の設定
    ssd1327_send_command(0x15);        // コラムアドレス設定コマンド
    ssd1327_send_command(X_start / 2); // Xstart 座標を2で割って送信
    ssd1327_send_command(X_end / 2);   // Xend 座標を2で割って送信

    // 行（Y座標）の設定
    ssd1327_send_command(0x75);    // 行アドレス設定コマンド
    ssd1327_send_command(Y_start); // Ystart 座標を送信
    ssd1327_send_command(Y_end);   // Yend 座標を送信
}

// 初期化
static void ssd1327_init()
{
    // 初期化コマンド群

    ssd1327_send_command(0xae); // OLED パネルをオフにする

    // コラムアドレスの設定
    ssd1327_send_command(0x15); // コラムアドレスの設定開始
    ssd1327_send_command(0x00); // 開始コラムアドレス 0
    ssd1327_send_command(0x7f); // 終了コラムアドレス 127

    // 行アドレスの設定
    ssd1327_send_command(0x75); // 行アドレスの設定開始
    ssd1327_send_command(0x00); // 開始行アドレス 0
    ssd1327_send_command(0x7f); // 終了行アドレス 127

    // コントラスト制御の設定
    ssd1327_send_command(0x81); // コントラスト制御コマンド
    ssd1327_send_command(0x80); // コントラスト値設定（0x80は中程度のコントラスト）

    // セグメントリマップ（セグメントの反転）設定
    ssd1327_send_command(0xa0); // セグメントリマップの設定
    ssd1327_send_command(0x51); // リマップの設定値（これにより表示の向きが変わる）

    // スタートライン設定
    ssd1327_send_command(0xa1); // スタートラインの設定コマンド
    ssd1327_send_command(0x00); // スタートライン 0（最初のラインから描画開始）

    // 表示オフセットの設定
    ssd1327_send_command(0xa2); // 表示オフセットの設定コマンド
    ssd1327_send_command(0x00); // オフセット 0（オフセットなし）

    // 全画面表示オン（反転表示を解除）
    ssd1327_send_command(0xa4); // 全画面表示（反転表示）オフ

    // マルチプレックス比の設定
    ssd1327_send_command(0xa8); // マルチプレックス比設定コマンド
    ssd1327_send_command(0x7f); // 64:1 マルチプレックス比（64行表示）

    // フェーズ長設定
    ssd1327_send_command(0xb1); // フェーズ長設定コマンド
    ssd1327_send_command(0xf1); // フェーズ長設定値（反応速度などに影響）

    // 表示のリセット（ON）
    ssd1327_send_command(0xab); // 表示リセットコマンド
    ssd1327_send_command(0x01); // リセットを有効にする（表示をクリア）

    // フェーズ長設定（別のレジスタ）
    ssd1327_send_command(0xb6); // フェーズ長設定コマンド
    ssd1327_send_command(0x0f); // フェーズ長設定値（別のタイミング調整）

    // プリチャージ期間設定
    ssd1327_send_command(0xbe); // プリチャージ期間設定コマンド
    ssd1327_send_command(0x0f); // 設定値（プリチャージ期間を調整）

    // VCOMH（コモングラウンド）の設定
    ssd1327_send_command(0xbc); // VCOMH設定コマンド
    ssd1327_send_command(0x08); // VCOMHの選択（調整可能）

    // 表示時計の設定
    ssd1327_send_command(0xd5); // 表示時計制御の設定コマンド
    ssd1327_send_command(0x62); // 表示クロックの設定値（表示周期に影響）

    // コントラストの微調整
    ssd1327_send_command(0xfd); // コントラスト微調整コマンド
    ssd1327_send_command(0x12); // 微調整値（コントラストを微調整）

    ssd1327_send_command(0xAF); // OLED パネルをオンにする
}

static void ssd1327_set_display(uint8_t *data)
{
    const int32_t data_length = DISPLAY_DATA_SIZE + 1;
    uint8_t buffer[data_length]; // 0x40はデータモードを示す
    buffer[0] = 0x40;
    for (int i = 0; i < DISPLAY_DATA_SIZE; i++)
    {
        buffer[i + 1] = data[i];
    }
    ssd1327_set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    i2c_write_blocking(i2c, SSD1327_ADDR, buffer, data_length, false);
}

int main()
{
    // Raspberry Pi Pico の初期化
    stdio_init_all();
    i2c_init_pico();

    // SSD1327 の初期化
    ssd1327_init();

    // 画面のクリア
    for (int i = 0; i < DISPLAY_DATA_SIZE; i++)
    {
        buffer[i] = 0;
    }
    ssd1327_set_display(buffer);

    while (true)
    {
        for (int i = 0; i < DISPLAY_DATA_SIZE; i++)
        {
            buffer[i] = 255;
        }
        ssd1327_set_display(buffer);
    }

    return 0;
}
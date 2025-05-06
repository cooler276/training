#include <stdio.h>         // 標準入出力ライブラリ（printfなど）
#include "pico/stdlib.h"   // Pico SDKの標準ライブラリ
#include "hardware/i2c.h"  // I2C通信用ライブラリ
#include "hardware/gpio.h" // GPIO制御用ライブラリ

// SHTC3のI2Cアドレス定義
#define SHTC3_I2C_ADDR (0x70) // SHTC3のI2Cアドレス（通常は0x70）

// SHTC3のレジスタ定義
#define SHTC3_REG_NORMAL_T_F (0x7866) // 温度を先に読み取るノーマルモードのコマンド
#define SHTC3_REG_WAKEUP (0x3517)     // ウェイクアップコマンド

// I2Cポートとピン定義
#define I2C_PORT i2c0 // 使用するI2Cポート（i2c0）
#define I2C_SDA_PIN 8 // SDAピン（データ線）のGPIO番号（GP8）
#define I2C_SCL_PIN 9 // SCLピン（クロック線）のGPIO番号（GP9）

// CRC-8計算関数
// データが正しく送受信されたかを確認するためのチェックサムを計算
uint8_t shtc3_crc8(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0xff;  // CRCの初期値
    uint8_t poly = 0x31; // 多項式（CRC計算で使用）
    while (len--)
    {                   // データ長分繰り返す
        crc ^= *data++; // データとCRCのXORを取る
        for (uint8_t i = 0; i < 8; i++)
        { // 8回繰り返す（ビット数）
            if (crc & 0x80)
            {                            // CRCの最上位ビットが1かどうか
                crc = (crc << 1) ^ poly; // 左シフトして多項式とXOR
            }
            else
            {
                crc = crc << 1; // 左シフト
            }
        }
    }
    return crc; // 計算結果のCRC値を返す
}

// SHTC3にコマンドを送信する関数
// SHTC3に命令（コマンド）を送る
bool shtc3_write_command(uint16_t command)
{
    uint8_t write_buf[2] = {
        // 送信するデータを格納する配列
        (command >> 8) & 0xFF, // コマンドの上位8ビット
        command & 0xFF         // コマンドの下位8ビット
    };
    // I2Cでデータを送信。成功したら2を返す
    int ret = i2c_write_blocking(I2C_PORT, SHTC3_I2C_ADDR, write_buf, 2, false);
    return ret == 2; // 2バイト送信成功でtrue、それ以外はfalse
}

// SHTC3から温度と湿度を読み取る関数
// SHTC3から温度と湿度のデータを取得する
bool shtc3_read_temp_humidity(float *temp, float *humidity)
{
    uint8_t cmd_msb = (SHTC3_REG_NORMAL_T_F >> 8) & 0xFF; // 測定コマンドの上位8ビット
    uint8_t cmd_lsb = SHTC3_REG_NORMAL_T_F & 0xFF;        // 測定コマンドの下位8ビット
    uint8_t write_buf[2] = {cmd_msb, cmd_lsb};            // 送信するデータ
    uint8_t read_buf[6];                                  // 受信するデータを格納する配列

    // 測定コマンドを送信
    int ret = i2c_write_blocking(I2C_PORT, SHTC3_I2C_ADDR, write_buf, 2, false);
    if (ret == PICO_ERROR_GENERIC)
    { // 送信エラーが発生した場合
        printf("SHTC3への測定コマンド送信エラー\n");
        return false;
    }

    sleep_ms(15); // 測定完了まで待つ（SHTC3の仕様で15ms以上必要）

    // 測定データを読み取る
    ret = i2c_read_blocking(I2C_PORT, SHTC3_I2C_ADDR, read_buf, 6, false);
    if (ret == PICO_ERROR_GENERIC)
    { // 受信エラーが発生した場合
        printf("SHTC3からのデータ読み取りエラー\n");
        return false;
    }

    // CRCチェック
    // 受信したデータが正しいかCRCを使って確認
    if (shtc3_crc8(read_buf, 2) != read_buf[2] || shtc3_crc8(read_buf + 3, 2) != read_buf[5])
    {
        printf("CRCチェックエラー：データが破損しています\n");
        return false;
    }

    // 温度と湿度を計算
    // 生のデータ（16ビット値）を実際の温度と湿度に変換
    uint16_t temp_raw = (read_buf[0] << 8) | read_buf[1];     // 温度データ
    uint16_t humidity_raw = (read_buf[3] << 8) | read_buf[4]; // 湿度データ

    *temp = (float)temp_raw * 175.0f / 65535.0f - 45.0f; // 温度に変換
    *humidity = (float)humidity_raw * 100.0f / 65535.0f; // 湿度に変換

    return true; // 成功
}

// SHTC3を初期化する関数
// SHTC3を動作開始状態にする
bool SHTC3_Init()
{
    uint8_t buffer[4] = {0, 0, 0, 0};                               // 送信するデータ（初期化用）
    i2c_write_blocking(I2C_PORT, SHTC3_I2C_ADDR, buffer, 3, false); // 初期化コマンド送信 (仮実装)
    shtc3_write_command(SHTC3_REG_WAKEUP);                          // ウェイクアップコマンド送信
    sleep_ms(1);                                                    // ウェイクアップ後の安定化待ち
    return true;
}

// メイン関数
// プログラムの実行開始地点
int main()
{
    stdio_init_all(); // 標準入出力の初期化（printfなどを使えるようにする）

    // I2Cの初期化
    i2c_init(I2C_PORT, 100 * 1000);                // I2Cポートを100kHzで初期化
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // SDAピンをI2Cとして設定
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // SCLピンをI2Cとして設定
    gpio_pull_up(I2C_SDA_PIN);                     // SDAピンをプルアップ
    gpio_pull_up(I2C_SCL_PIN);                     // SCLピンをプルアップ

    SHTC3_Init(); // SHTC3センサを初期化

    while (true)
    {                                // 無限ループ（プログラムをずっと実行し続ける）
        float temperature, humidity; // 温度と湿度を格納する変数
        if (shtc3_read_temp_humidity(&temperature, &humidity))
        {
            // 温度と湿度を読み取り成功した場合
            printf("温度: %.2f °C, 湿度: %.2f %%\n", temperature, humidity); // 結果を表示
        }
        else
        {
            // 読み取り失敗した場合
            printf("温度・湿度の読み取りに失敗しました\n");
        }
        sleep_ms(1000); // 1秒待つ
    }

    return 0; // プログラム終了（通常は到達しない）
}
#include <stdio.h>                   // 標準入出力ライブラリ
#include "pico/stdlib.h"             // Pico SDK の標準ライブラリ
#include "hardware/i2c.h"            // I2C 通信ライブラリ
#include "hardware/gpio.h"           // GPIO 制御ライブラリ
#include "sensirion_voc_algorithm.h" // Sensirion VOC アルゴリズムライブラリ

// SGP40 I2C アドレス
#define SGP40_ADDR 0x59 // SGP40 センサーの I2C アドレス

// I2C ポートとピン (配線に合わせて調整)
#define I2C_PORT i2c0 // 使用する I2C ポート (i2c0 または i2c1)
#define I2C_SDA_PIN 8 // SDA (データ) ピン
#define I2C_SCL_PIN 9 // SCL (クロック) ピン

// SGP40 コマンド
uint8_t WITH_HUM_COMP[] = {0x26, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 湿度補償付き raw データ用

// I2C 読み取りバイト関数
static uint16_t SGP40_ReadByte()
{
    uint8_t Rbuf[3];                                         // 受信バッファ
    i2c_read_blocking(I2C_PORT, SGP40_ADDR, Rbuf, 3, false); // I2C 受信
    return (Rbuf[0] << 8) | Rbuf[1];                         // 受信データを 16 ビットに合成して返す
}

// CRC8 チェックサム計算
uint8_t crc_value(uint8_t msb, uint8_t lsb)
{
    uint8_t crc = 0xFF; // CRCの初期値を0xFFに設定

    crc ^= msb; // 上位バイト(msb)とCRCのXORをとる
    for (int i = 0; i < 8; i++)
    { // 8回繰り返す (1バイトは8ビットのため)
        if (crc & 0x80)
        {                            // CRCの最上位ビット(MSB)が1かどうかをチェック
            crc = (crc << 1) ^ 0x31; // MSBが1の場合、左シフトして0x31とのXORをとる
        }
        else
        {
            crc <<= 1; // MSBが0の場合、左シフトする
        }
    }

    if (lsb != 0)
    {               // 下位バイト(lsb)が0でない場合
        crc ^= lsb; // 下位バイトとCRCのXORをとる
        for (int i = 0; i < 8; i++)
        { // 8回繰り返す
            if (crc & 0x80)
            {                            // CRCのMSBが1かどうかをチェック
                crc = (crc << 1) ^ 0x31; // MSBが1の場合、左シフトして0x31とのXORをとる
            }
            else
            {
                crc <<= 1; // MSBが0の場合、左シフトする
            }
        }
    }
    return crc; // 最終的なCRC値を返す
}

// SGP40 初期化
uint8_t SGP40_init(void)
{
    uint8_t cmd_feature_set[] = {0x20, 0x2F};  // Feature Set コマンド
    uint8_t cmd_measure_test[] = {0x28, 0x0E}; // Measure Test コマンド

    i2c_write_blocking(I2C_PORT, SGP40_ADDR, cmd_feature_set, 2, false); // Feature Set コマンド送信
    sleep_ms(250);                                                       // 250ms 待機
    if (SGP40_ReadByte() != 0x3220)
    {
        printf("Self test failed (Feature Set)\n"); // エラーメッセージ
        return 1;                                   // エラーを返す
    }

    i2c_write_blocking(I2C_PORT, SGP40_ADDR, cmd_measure_test, 2, false); // Measure Test コマンド送信
    sleep_ms(250);                                                        // 250ms 待機
    if (SGP40_ReadByte() != 0xD400)
    {
        printf("Self test failed (Measure Test)\n"); // エラーメッセージ
        return 1;                                    // エラーを返す
    }
    return 0; // 正常終了
}

// SGP40 Raw データ測定 (温度と湿度を引数として受け取る)
uint16_t SGP40_MeasureRaw(float temp, float humi)
{
    uint16_t h = humi * 0xffff / 100;               // 湿度を 16bit 値に変換
    uint8_t paramh[2] = {h >> 8, h & 0xff};         // 上位バイトと下位バイトに分割
    uint8_t crch = crc_value(paramh[0], paramh[1]); // 湿度パラメータの CRC 値を計算

    uint16_t t = (temp + 45) * 0xffff / 175;        // 温度を 16bit 値に変換
    uint8_t paramt[2] = {t >> 8, t & 0xff};         // 上位バイトと下位バイトに分割
    uint8_t crct = crc_value(paramt[0], paramt[1]); // 温度パラメータの CRC 値を計算

    WITH_HUM_COMP[2] = paramh[0]; // 湿度パラメータ (上位バイト) を設定
    WITH_HUM_COMP[3] = paramh[1]; // 湿度パラメータ (下位バイト) を設定
    WITH_HUM_COMP[4] = crch;      // 湿度パラメータの CRC 値を設定
    WITH_HUM_COMP[5] = paramt[0]; // 温度パラメータ (上位バイト) を設定
    WITH_HUM_COMP[6] = paramt[1]; // 温度パラメータ (下位バイト) を設定
    WITH_HUM_COMP[7] = crct;      // 温度パラメータの CRC 値を設定

    i2c_write_blocking(I2C_PORT, SGP40_ADDR, WITH_HUM_COMP, 8, false); // 湿度補償付き raw データ測定コマンド送信
    sleep_ms(31);                                                      // 31ms 待機
    return SGP40_ReadByte();                                           // SGP40 からの応答 (raw データ) を返す
}

int main()
{
    stdio_init_all();                              // 標準入出力の初期化
    i2c_init(I2C_PORT, 100 * 1000);                // I2C の初期化 (100kHz)
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // SDA ピンを I2C 機能に設定
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // SCL ピンを I2C 機能に設定
    gpio_pull_up(I2C_SDA_PIN);                     // SDA ピンをプルアップ
    gpio_pull_up(I2C_SCL_PIN);                     // SCL ピンをプルアップ

    VocAlgorithmParams voc_params; // VOC アルゴリズムのパラメータ構造体を定義

    if (SGP40_init() != 0)
    {
        printf("SGP40 initialization failed\n"); // SGP40 の初期化に失敗した場合のエラーメッセージ
        return 1;                                // エラーを返す
    }

    printf("SGP40 VOC Index Reader (100ms interval)\n");                // プログラムの開始メッセージ
    printf("I2C SDA Pin: %d, SCL Pin: %d\n", I2C_SDA_PIN, I2C_SCL_PIN); // 使用する I2C ピンを表示
    printf("SGP40 I2C Address: 0x%02X\n", SGP40_ADDR);                  // SGP40 の I2C アドレスを表示

    float temperature = 25.0f; // 仮の温度 (摂氏)
    float humidity = 50.0f;    // 仮の湿度 (%)
    uint32_t voc_index;        // VOC Index を格納する変数

    VocAlgorithm_init(&voc_params); // VOC アルゴリズムの初期化

    while (true)
    {
        uint16_t sraw = SGP40_MeasureRaw(temperature, humidity); // SGP40 から raw データを取得
        VocAlgorithm_process(&voc_params, sraw, &voc_index);     // VOC Index を計算
        printf("VOC Index: %ld\n", voc_index);                   // VOC Index を表示
        sleep_ms(100);                                           // 100ms 待機
    }

    return 0; // プログラム終了
}

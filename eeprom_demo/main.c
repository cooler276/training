#include <stdio.h>        // 標準入出力ライブラリ（printf関数など）
#include "pico/stdlib.h"  // Pico SDKの標準ライブラリ
#include "hardware/i2c.h" // I2C通信関連のライブラリ
#include <string.h>       // 文字列操作関連のライブラリ（memcmp関数など）

// I2Cポートとピン定義
#define I2C_PORT i2c0 // 使用するI2Cポート
#define I2C_SDA_PIN 8 // I2CのSDA (シリアルデータ) ピンとしてGP8を使用
#define I2C_SCL_PIN 9 // I2CのSCL (シリアルクロック) ピンとしてGP9を使用

// EEPROMのI2Cアドレス
#define AT24CXX_I2C_ADDR 0x50 // AT24CXXシリーズEEPROMのI2Cアドレス

// I2C初期化関数
void i2c_init_eeprom()
{
    // I2Cを初期化する。第一引数は使用するI2Cポート、第二引数は通信速度。
    // 100 * 1000 は 100kHz を意味する。最大400kHzまで対応。
    i2c_init(I2C_PORT, 100 * 1000);

    // GPIO (汎用入出力) ピンをI2Cの機能として設定する。
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // SDAピンをI2C機能に設定
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // SCLピンをI2C機能に設定

    // I2C通信では、プルアップ抵抗が必要となる。
    // 内蔵プルアップ抵抗を有効にする。
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

// EEPROMからデータを読み出す関数
// reg: 読み出しを開始するEEPROM内のアドレス (レジスタ)
// pData: 読み出したデータを格納するバッファへのポインタ
// Len: 読み出すデータのバイト数
bool EEPROM_Read(uint8_t reg, uint8_t *pData, uint8_t Len)
{
    int ret; // I2C通信の戻り値を格納する変数

    // 1. 読み出すEEPROM内のアドレスを送信する。
    uint8_t addr[1] = {reg}; // 読み出し開始アドレスを格納する1バイトの配列
    // i2c_write_blocking関数を使って、EEPROMのアドレスと読み出したい内部アドレスを送信する。
    // 第四引数の 'true' は、stopビットを送信しないことを意味する。
    // これは、続けて読み出し動作を行うために必要となる。
    ret = i2c_write_blocking(I2C_PORT, AT24CXX_I2C_ADDR, addr, 1, true);
    if (ret < 0)
    {
        printf("I2Cアドレス送信エラー (Read): %d\n", ret);
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
        return false;   // 読み出し失敗
    }

    // 2. EEPROMからデータを読み出す。
    // i2c_read_blocking関数を使って、指定したバイト数のデータをEEPROMから読み込む。
    // 第四引数の 'false' は、読み出し完了後にstopビットを送信することを意味する。
    ret = i2c_read_blocking(I2C_PORT, AT24CXX_I2C_ADDR, pData, Len, false);
    if (ret < 0)
    {
        printf("I2C読み出しエラー: %d\n", ret);
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
        return false;   // 読み出し失敗
    }

    return true; // 読み出し成功
}

// EEPROMにデータを書き込む関数
// reg: 書き込みを開始するEEPROM内のアドレス (レジスタ)
// pData: 書き込むデータへのポインタ
// Len: 書き込むデータのバイト数
bool EEPROM_Write(uint8_t reg, uint8_t *pData, uint8_t Len)
{
    int ret; // I2C通信の戻り値を格納する変数

    // EEPROMに書き込むデータは、最初に書き込みたいアドレスを含む必要がある。
    // そのため、書き込みアドレス (reg) と書き込むデータ (pData) を結合したバッファを作成する。
    uint8_t buffer[Len + 1]; // アドレス1バイト + データLenバイトのバッファ
    buffer[0] = reg;         // バッファの最初のバイトに書き込みアドレスを設定
    for (uint8_t i = 0; i < Len; i++)
    {
        buffer[i + 1] = pData[i]; // バッファの2バイト目以降に書き込むデータをコピー
    }

    // i2c_write_blocking関数を使って、EEPROMのアドレス、書き込むデータ (アドレスを含む)、データ長を送信する。
    // 第四引数の 'false' は、書き込み完了後にstopビットを送信することを意味する。
    ret = i2c_write_blocking(I2C_PORT, AT24CXX_I2C_ADDR, buffer, Len + 1, false);
    if (ret < 0)
    {
        printf("I2C書き込みエラー: %d\n", ret);
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
        return false;   // 書き込み失敗
    }

    sleep_ms(5); // EEPROMの書き込み動作が完了するまで少し待つ
    return true; // 書き込み成功
}

int main()
{
    stdio_init_all(); // 標準入出力 (USBシリアルなど) を初期化
    sleep_ms(10000);  // 速すぎて目で追いにくいため確認用に少し待機

    printf("EEPROM I2C テスト 開始\n");
    sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機

    // I2C初期化
    i2c_init_eeprom();
    printf("I2C 初期化\n");
    sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機

    // 書き込みテスト
    uint8_t write_address = 0x00;                    // 書き込みを開始するEEPROM内のアドレス
    uint8_t write_data[] = {0xA1, 0xB2, 0xC3, 0xD4}; // 書き込むデータ
    printf("書き込みデータ [0x%02X, 0x%02X, 0x%02X, 0x%02X] アドレス 0x%02X...\n",
           write_data[0], write_data[1], write_data[2], write_data[3], write_address);
    sleep_ms(1000); // 書き込み開始メッセージ表示後、少し待機
    if (EEPROM_Write(write_address, write_data, sizeof(write_data)))
    {
        printf("書き込み成功\n");
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
    }
    else
    {
        printf("書き込み失敗\n");
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
    }

    // 読み出しテスト
    uint8_t read_address = 0x00;             // 読み出しを開始するEEPROM内のアドレス (書き込んだアドレスと同じ)
    uint8_t read_buffer[sizeof(write_data)]; // 読み出したデータを格納するバッファ (書き込むデータと同じサイズ)
    printf("読み込み開始アドレス 0x%02X...\n", read_address);
    if (EEPROM_Read(read_address, read_buffer, sizeof(read_buffer)))
    {
        printf("読み込み成功 データ: [0x%02X, 0x%02X, 0x%02X, 0x%02X]\n",
               read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]);
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機

        // 書き込んだデータと読み出したデータを比較
        if (memcmp(write_data, read_buffer, sizeof(write_data)) == 0)
        {
            printf("読み込みデータと書き込みデータが一致\n");
            sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
        }
        else
        {
            printf("読み込みデータと書き込みデータが不一致\n");
            sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
        }
    }
    else
    {
        printf("読み込み失敗\n");
        sleep_ms(1000); // 速すぎて目で追いにくいため確認用に少し待機
    }

    printf("テスト終了\n");
    while (true)
    {
        // テスト終了後、無限ループに入る。
    }

    return 0;
}
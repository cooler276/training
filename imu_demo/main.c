#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2Cポートの設定
#define I2C_PORT i2c0 // 使用するI2Cのポート番号
#define SDA_PIN 8     // I2CのSDA (Serial Data) ピン
#define SCL_PIN 9     // I2CのSCL (Serial Clock) ピン

// QMI8658というセンサーのI2Cスレーブアドレス
#define QMI8658_SLAVE_ADDR_L 0x6A // QMI8658の7ビットアドレス（下位ビットが0の場合）
#define QMI8658_SLAVE_ADDR_H 0x6B // QMI8658の7ビットアドレス（下位ビットが1の場合）
#define INTERVAL 100              // データの読み取り間隔（ミリ秒）

// QMI8658センサーのレジスタアドレス（設定やデータの読み取りに使う番号）
#define QMI8658Register_WhoAmI 0x00 // デバイスIDを確認するレジスタ
#define QMI8658Register_Ctrl1 0x02  // センサーの動作モードを制御するレジスタ
#define QMI8658Register_Ctrl2 0x03  // 加速度センサーの設定レジスタ
#define QMI8658Register_Ctrl3 0x04  // ジャイロセンサーの設定レジスタ
#define QMI8658Register_Ctrl5 0x05  // LPF (ローパスフィルタ) / HPF (ハイパスフィルタ) の設定レジスタ
#define QMI8658Register_Ctrl7 0x0A  // センサーの電源モードなどを設定するレジスタ
#define QMI8658Register_Ax_L 0x35   // 加速度センサーのX軸データ（下位バイト）のレジスタ

// LPF (ローパスフィルタ) / HPF (ハイパスフィルタ) の設定値
#define A_LSP_MODE_3 0x03 // 加速度センサーのLPFモード3の設定値
#define G_LSP_MODE_3 0x30 // ジャイロセンサーのLPFモード3の設定値

// 加速度とジャイロのLSB (Least Significant Bit) あたりの値
// センサーの感度によって変わる。初期化後に設定される。
static unsigned short acc_lsb_div = 0;
static unsigned short gyro_lsb_div = 0;

// QMI8658のスレーブアドレスを保持する変数。初期化時にどちらのアドレスか判別する。
static unsigned char QMI8658_slave_addr = QMI8658_SLAVE_ADDR_L;

// 加速度とジャイロのオフセット（バイアス）を補正するための変数
float acc_offset[3] = {0.0f, 0.0f, 0.0f};  // X, Y, Z軸の加速度オフセット
float gyro_offset[3] = {0.0f, 0.0f, 0.0f}; // X, Y, Z軸のジャイロオフセット

// I2Cで指定したレジスタに1バイトのデータを書き込む関数
void i2c_write_byte(uint8_t reg, uint8_t value)
{
    uint8_t data[] = {reg, value}; // 書き込むレジスタアドレスと値を配列に格納
    // I2Cデバイスにデータを送信（書き込み）。
    // I2C_PORT: 使用するI2Cポート
    // QMI8658_slave_addr: I2Cスレーブアドレス
    // data: 送信するデータ配列
    // 2: 送信するデータのバイト数（レジスタアドレス + 値）
    // false: 送信後にSTOPコンディションを送信しない（続けて通信を行う可能性がある場合）
    i2c_write_blocking(I2C_PORT, QMI8658_slave_addr, data, 2, false);
}

// I2Cで指定したレジスタから複数バイトのデータを読み込む関数
void i2c_read_bytes(uint8_t reg, uint8_t *buf, size_t len)
{
    // 読み込みたいレジスタアドレスをI2Cデバイスに送信（書き込みモードでアドレスのみ送信）
    i2c_write_blocking(I2C_PORT, QMI8658_slave_addr, &reg, 1, true);
    // I2Cデバイスからデータを読み込む
    // I2C_PORT: 使用するI2Cポート
    // QMI8658_slave_addr: I2Cスレーブアドレス
    // buf: 読み込んだデータを格納するバッファ（配列）
    // len: 読み込むデータのバイト数
    // false: 受信後にSTOPコンディションを送信しない
    i2c_read_blocking(I2C_PORT, QMI8658_slave_addr, buf, len, false);
}

// QMI8658のLPF (ローパスフィルタ) と HPF (ハイパスフィルタ) を設定する関数
void set_lpf_hpf()
{
    uint8_t ctrl5_value = 0;

    // Ctrl5レジスタの現在の値を読み込む
    i2c_read_bytes(QMI8658Register_Ctrl5, &ctrl5_value, 1);

    // LPFの設定を有効にする（下位4ビットを操作）
    ctrl5_value &= 0xF0;                // 上位4ビット（HPF設定）を保持し、下位4ビットを0クリア
    ctrl5_value |= A_LSP_MODE_3;        // 加速度計のLPF設定を反映
    ctrl5_value |= (G_LSP_MODE_3 >> 4); // ジャイロスコープのLPF設定を反映（Ctrl5の下位4ビット）

    // LPFとHPFの設定値をCtrl5レジスタに書き込む
    i2c_write_byte(QMI8658Register_Ctrl5, ctrl5_value);
}

// QMI8658センサーを初期化する関数
unsigned char QMI8658_init(void)
{
    unsigned char chip_id = 0x00;
    // QMI8658には2つの可能性のあるI2Cアドレスがあるため、両方試す
    for (int i = 0; i < 2; i++)
    {
        QMI8658_slave_addr = (i == 0) ? QMI8658_SLAVE_ADDR_L : QMI8658_SLAVE_ADDR_H;
        // 最大5回まで初期化を試みる
        for (int retry = 0; retry < 5; retry++)
        {
            // WhoAmIレジスタを読み取り、デバイスIDを確認
            i2c_read_bytes(QMI8658Register_WhoAmI, &chip_id, 1);
            if (chip_id == 0x05) // QMI8658のデバイスIDは0x05
            {
                printf("QMI8658をアドレス 0x%X で初期化しました\n", QMI8658_slave_addr);

                // センサーの初期設定
                i2c_write_byte(QMI8658Register_Ctrl1, 0x60); // 加速度計とジャイロスコープを有効化 (0b01100000)
                i2c_write_byte(QMI8658Register_Ctrl2, 0x23); // 加速度計の設定: ±8g, 1kHz (0b00100011)
                i2c_write_byte(QMI8658Register_Ctrl3, 0x53); // ジャイロスコープの設定: ±2000dps, 1kHz (0b01010011)
                i2c_write_byte(QMI8658Register_Ctrl7, 0x03); // センサーの動作モード設定 (0b00000011)

                // LPFとHPFの設定
                set_lpf_hpf();

                // 加速度とジャイロのLSBあたりの値を設定（±8gと±2000dpsの場合）
                acc_lsb_div = (1 << 12); // 2^12 = 4096 (±8gの分解能)
                gyro_lsb_div = 16;       // ±2000dpsの分解能
                return 1;                // 初期化成功
            }
            sleep_ms(10); // 少し待ってから再試行
        }
    }
    printf("QMI8658の初期化に失敗しました\n");
    return 0; // 初期化失敗
}

// 加速度とジャイロの生データを読み取る関数
void read_acc_gyro(float *acc, float *gyro)
{
    uint8_t buf[12]; // 加速度(3軸 * 2バイト) + ジャイロ(3軸 * 2バイト) = 12バイト
    // センサーの加速度とジャイロのデータレジスタから12バイト読み込む
    i2c_read_bytes(QMI8658Register_Ax_L, buf, 12);

    // 加速度データの処理 (リトルエンディアン形式)
    for (int i = 0; i < 3; i++)
    {
        // 下位バイトと上位バイトを組み合わせて16ビットの符号付き整数にする
        int16_t raw = (int16_t)((buf[i * 2 + 1] << 8) | buf[i * 2]);
        // 生データを物理単位 (g) に変換
        acc[i] = (float)(raw * 1.0f) / acc_lsb_div;
    }

    // ジャイロデータの処理 (リトルエンディアン形式)
    for (int i = 0; i < 3; i++)
    {
        // 下位バイトと上位バイトを組み合わせて16ビットの符号付き整数にする
        int16_t raw = (int16_t)((buf[i * 2 + 7] << 8) | buf[i * 2 + 6]);
        // 生データを物理単位 (dps: degree per second) に変換
        gyro[i] = (float)(raw * 1.0f) / gyro_lsb_div;
    }
}

// センサーのキャリブレーションを行い、オフセット値を計算する関数
void calibrate_sensor()
{
    float acc_sum[3] = {0.0f, 0.0f, 0.0f};
    float gyro_sum[3] = {0.0f, 0.0f, 0.0f};
    int samples = 200; // キャリブレーションのために読み取るサンプル数

    printf("センサーのキャリブレーションを開始します...\n");

    // 指定された回数だけデータを読み取り、それぞれの軸の合計値を計算
    for (int i = 0; i < samples; i++)
    {
        float acc[3], gyro[3];
        read_acc_gyro(acc, gyro); // 生データを読み取る

        for (int j = 0; j < 3; j++)
        {
            acc_sum[j] += acc[j];   // 各軸の加速度の合計値を加算
            gyro_sum[j] += gyro[j]; // 各軸のジャイロの合計値を加算
        }
        sleep_ms(5); // 少し待つ
    }

    // 各軸の平均値を計算し、オフセット値とする
    for (int j = 0; j < 3; j++)
    {
        acc_offset[j] = acc_sum[j] / samples;   // 加速度のオフセット
        gyro_offset[j] = gyro_sum[j] / samples; // ジャイロのオフセット
    }

    printf("キャリブレーション完了。加速度オフセット: [%f, %f, %f], ジャイロオフセット: [%f, %f, %f]\n",
           acc_offset[0], acc_offset[1], acc_offset[2],
           gyro_offset[0], gyro_offset[1], gyro_offset[2]);
}

// オフセットを考慮して加速度とジャイロのデータを読み取る関数
void read_acc_gyro_with_offset(float *acc, float *gyro)
{
    read_acc_gyro(acc, gyro); // まず生データを読み取る

    // 各軸のデータから事前に計算したオフセット値を引く
    for (int i = 0; i < 3; i++)
    {
        acc[i] -= acc_offset[i];
        gyro[i] -= gyro_offset[i];
    }
}

// メイン関数
int main()
{
    stdio_init_all(); // 標準入出力の初期化

    // I2Cの初期化
    i2c_init(I2C_PORT, 400 * 1000); // I2Cポートを400kHzの速度で初期化

    // I2CピンのGPIO機能を設定
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C); // SDAピンをI2Cの機能に設定
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C); // SCLピンをI2Cの機能に設定

    // I2Cピンにプルアップ抵抗を設定（外付けのプルアップ抵抗がない場合）
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // QMI8658センサーの初期化
    if (!QMI8658_init())
    {
        return 1; // 初期化に失敗したらプログラムを終了
    }

    // センサーのキャリブレーションを実行
    calibrate_sensor();

    float acc[3], gyro[3]; // 加速度とジャイロの値を格納する配列
    // メインループ
    while (1)
    {
        // オフセットを考慮して加速度とジャイロのデータを読み取る
        read_acc_gyro_with_offset(acc, gyro);
        // 読み取った値をコンソールに出力
        printf("Acc: [%f, %f, %f], Gyro: [%f, %f, %f]\n",
               acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2]);
        sleep_ms(INTERVAL); // 指定された間隔で待機
    }

    return 0; // プログラムが正常に終了した場合
}

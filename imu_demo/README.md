# 概要

* I2C通信を用いてQMI8658 6軸IMUセンサから加速度とジャイロのデータを読み取る。

# 動作

## 初期化

1.  `i2c_init(I2C_PORT, 400 * 1000)` 関数により、指定されたI2Cポート (`I2C_PORT`) を400kHzの通信速度で初期化する。

2.  `gpio_set_function()` 関数を用いて、SDAピン (`SDA_PIN`) と SCLピン (`SCL_PIN`) をI2Cの機能として設定する。

3.  `gpio_pull_up()` 関数を用いて、SDAピンとSCLピンに内蔵プルアップ抵抗を有効にする。

4.  `QMI8658_init()` 関数を呼び出し、QMI8658センサを初期化する。

    - QMI8658の2つの可能性のあるI2Cアドレス (`QMI8658_SLAVE_ADDR_L` と `QMI8658_SLAVE_ADDR_H`) を順に試し、`QMI8658Register_WhoAmI` レジスタを読み取ってデバイスID (`0x05`) を確認する。
    - 初期化に成功した場合、加速度計とジャイロスコープを有効にし、それぞれの設定 (±8g, 1kHz および ±2000dps, 1kHz) およびセンサーの動作モードを設定する。
    - ローパスフィルタ (LPF) とハイパスフィルタ (HPF) の設定を行う。
    - 加速度とジャイロのLSBあたりの値を設定する。

## 加速度・ジャイロ読み取り処理

1.  `read_acc_gyro(float *acc, float *gyro)` 関数は、QMI8658センサから加速度とジャイロの生データを読み取り、指定されたポインタ (`acc`, `gyro`) に格納する。

2.  `i2c_read_bytes(QMI8658Register_Ax_L, buf, 12)` 関数を用いて、加速度 (X, Y, Z軸 各2バイト) とジャイロ (X, Y, Z軸 各2バイト) の計12バイトのデータを読み取る。

3.  読み取った16ビットの生データ (リトルエンディアン形式) を、それぞれのLSBあたりの値で割り、浮動小数点型の加速度 (g) とジャイロ (dps) に変換する。

    - 加速度: `acc[i] = (float)(raw * 1.0f) / acc_lsb_div;`
    - ジャイロ: `gyro[i] = (float)(raw * 1.0f) / gyro_lsb_div;`

## センサーキャリブレーション処理

1.  `calibrate_sensor()` 関数は、センサーのオフセット (バイアス) をキャリブレーションし、その値を `acc_offset` と `gyro_offset` グローバル変数に格納する。

2.  指定されたサンプル数 (`samples = 200`) だけ加速度とジャイロの生データを読み取り、各軸の合計値を計算する。

3.  各軸の合計値をサンプル数で割り、平均値をオフセット値とする。

4.  キャリブレーション完了後、計算されたオフセット値をシリアルモニタに出力する。

## オフセット補正済みデータ読み取り処理

1.  `read_acc_gyro_with_offset(float *acc, float *gyro)` 関数は、`read_acc_gyro()` 関数で読み取った生データから、`calibrate_sensor()` 関数で計算されたオフセット値を減算し、補正済みの加速度とジャイロのデータを指定されたポインタに格納する。

    - `acc[i] -= acc_offset[i];`
    - `gyro[i] -= gyro_offset[i];`

## STOPビットについて

I2C通信では、マスターデバイス (この場合はRaspberry Pi Pico) が通信の開始と終了を制御する。<br>**STOPビット**とは通信の終了を示すもの。

- **コマンド送信におけるSTOPビット:** `i2c_write_byte()` 関数や `i2c_write_blocking()` 関数では、データの送信が完了した後、第4引数に `false` を指定することで、STOPビットを送信しない場合がある（続けて通信を行う意図がある場合）。初期化処理や設定書き込みなどで使用される。

- **データ読み取り処理におけるSTOPビット:** `i2c_read_bytes()` 関数では、読み込みたいレジスタアドレスを送信する `i2c_write_blocking()` 関数で `true` を指定し、STOPビットを送信して一旦通信を終え、続けて `i2c_read_blocking()` 関数でデータを読み取る。`i2c_read_blocking()` 関数は、読み取り完了後にSTOPビットを送信する。

    STOPビットを適切に送信することで、I2Cバス上の他のデバイスとの通信の衝突を防ぎ、正常な通信シーケンスを維持することが可能。

## メインループ

1.  `main()` 関数内で、まず `stdio_init_all()` 関数により標準入出力 (USBシリアルなど) を初期化する。

2.  `i2c_init()` 関数と `gpio_set_function()`、`gpio_pull_up()` 関数を用いてI2C通信を初期化する。

3.  `QMI8658_init()` 関数を呼び出し、QMI8658センサを初期化する。初期化に失敗した場合はプログラムを終了する。

4.  `calibrate_sensor()` 関数を呼び出し、センサーのキャリブレーションを実行する。

5.  無限ループ (`while(1)`) に入り、以下の処理を繰り返す。

    - `read_acc_gyro_with_offset()` 関数を呼び出し、オフセット補正済みの加速度とジャイロのデータを読み取る。
    - 読み取った加速度とジャイロの値をシリアルモニタに出力する。
    - `sleep_ms(INTERVAL)` により、指定された間隔 (`INTERVAL = 100` ミリ秒) の遅延を設ける。

# 補足

* **I2Cポートとピン:**

    ```c
    #define I2C_PORT i2c0
    #define SDA_PIN 8
    #define SCL_PIN 9
    ```

    使用するI2Cポート (`i2c0`) と、SDA (シリアルデータ) ピンとしてGP8、SCL (シリアルクロック) ピンとしてGP9を定義している。

* **QMI8658のI2Cアドレス:**

    ```c
    #define QMI8658_SLAVE_ADDR_L 0x6A
    #define QMI8658_SLAVE_ADDR_H 0x6B
    ```

    QMI8658センサには2つの可能性のあるI2Cアドレスがあり、プログラム内で両方を試している。

* **QMI8658のレジスタ定義:**

    ```c
    #define QMI8658Register_WhoAmI 0x00
    #define QMI8658Register_Ctrl1 0x02
    #define QMI8658Register_Ctrl2 0x03
    #define QMI8658Register_Ctrl3 0x04
    #define QMI8658Register_Ctrl5 0x05
    #define QMI8658Register_Ctrl7 0x0A
    #define QMI8658Register_Ax_L 0x35
    ```

    QMI8658の様々な制御レジスタやデータレジスタのアドレスを定義している。

* **LPF/HPF設定:**

    ```c
    #define A_LSP_MODE_3 0x03
    #define G_LSP_MODE_3 0x30
    ```

    ローパスフィルタ (LPF) およびハイパスフィルタ (HPF) の設定値を定義している。

* **I2Cの初期化:**

    `i2c_init()` 関数でI2C通信速度を設定し、`gpio_set_function()` でGPIOピンをI2C機能に割り当て、`gpio_pull_up()` でプルアップ抵抗を有効にしている。I2C通信にはプルアップ抵抗が不可欠。

* **QMI8658へのコマンド送信とデータ読み取り:**

    `i2c_write_byte()` 関数や `i2c_write_blocking()` 関数を用いて設定コマンドを送信し、`i2c_read_bytes()` 関数を用いてセンサーから生データを読み取る。

* **センサーキャリブレーション:**

    一定期間データを収集し平均値を計算することで、センサーの静的なオフセットを推定し補正している。

* **CMakeLists.txt:** I2C関連の機能を利用するため、`target_link_libraries` に `hardware_i2c` を追加する必要がある。

    ```cmake
    target_link_libraries(imu_demo
        hardware_i2c
    )
    ```


# 注意事項
出力結果が安定していない為、改善の必要がある。
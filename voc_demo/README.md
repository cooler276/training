# 概要

* I2C通信を用いてSensirion社製VOCセンサー SGP40 から VOC Index を読み取る。
* オプションで温度と湿度による補償を行う。

# 動作

## 初期化

1.  `i2c_init(I2C_PORT, 100 * 1000)` 関数により、指定されたI2Cポート (`I2C_PORT`) を100kHzの通信速度で初期化する。

2.  `gpio_set_function()` 関数を用いて、SDAピン (`I2C_SDA_PIN`) と SCLピン (`I2C_SCL_PIN`) をI2Cの機能として設定する。

3.  `gpio_pull_up()` 関数を用いて、SDAピンとSCLピンに内蔵プルアップ抵抗を有効にする。I2C通信にはプルアップ抵抗が不可欠。

4.  `SGP40_init()` 関数を呼び出し、SGP40センサの初期化とセルフテストを行う。

    * `cmd_feature_set` コマンド (`0x20`, `0x2F`) を送信し、Feature Set を確認する。応答が `0x3220` でない場合はエラーとする。
    * `cmd_measure_test` コマンド (`0x28`, `0x0E`) を送信し、Measure Test を実行する。応答が `0xD400` でない場合はエラーとする。
    * これらのセルフテストにより、センサーが正常に動作しているかを確認する。

## VOC Index 読み取り処理

1.  `SGP40_MeasureRaw(float temp, float humi)` 関数は、SGP40センサから湿度補償付きの raw データを読み取る。温度 (`temp`) と湿度 (`humi`) の値を引数として受け取る。

2.  入力された温度と湿度の値を、SGP40 が要求する形式の16ビット値に変換する。

    * 湿度 (0-100%): `humi * 0xffff / 100`
    * 温度 (-45 - 130℃): `(temp + 45) * 0xffff / 175`

3.  変換された温度と湿度の各16ビットデータに対して、CRC-8 チェックサムを `crc_value(uint8_t msb, uint8_t lsb)` 関数を用いて計算する。

    * **CRC（巡回冗長検査）について:** CRCは、デジタルデータが伝送中や保存中に誤り（ビット化け）がないかを検出するための誤り検出符号の一種です。送信側がデータから特定の計算方法に基づいてチェックサム（CRC値）を生成し、データと一緒に送信します。受信側も同様の計算をデータに対して行い、得られたCRC値が送信されてきたCRC値と一致するかどうかを比較することで、データの信頼性を検証します。
    * SGP40 センサーとの通信においては、コマンドパラメータにCRC-8チェックサムを付加する必要があります。
    * `crc_value()` 関数は、入力された上位バイト (`msb`) と下位バイト (`lsb`) から、あらかじめ定義された多項式 (`0x31`) を用いたビット演算を行い、8ビットのCRC値を生成します。

4.  湿度補償付き raw データ測定コマンド (`0x26`, `0x0F`) に、変換された湿度と温度の16ビットデータ（上位バイト、下位バイト）とそれぞれのCRC値を付加した8バイトのコマンドを `i2c_write_blocking()` 関数を用いて SGP40 へ送信する。

5.  測定が完了するまで `sleep_ms()` で待機する。

6.  `SGP40_ReadByte()` 関数を用いて SGP40 から 3 バイトの応答（raw VOC データ 2バイト + CRC 1バイト）を読み取る。

7.  読み取った 2 バイトの raw VOC データを 16 ビット値に合成して返す。

8.  `main()` 関数内の `while(true)` ループで、`VocAlgorithm_process(&voc_params, sraw, &voc_index)` 関数を用いて、取得した raw VOC データ (`sraw`) を VOC Index (`voc_index`) に変換する。この処理には、Sensirion 提供の VOC アルゴリズムライブラリが使用される。

## STOPビットについて

I2C通信では、マスターデバイス (この場合はRaspberry Pi Pico) が通信の開始と終了を制御する。<br>**STOPビット**とは通信の終了を示すもの。

- **コマンド送信におけるSTOPビット:** `i2c_write_blocking()` 関数では、コマンドの送信が完了した後、第4引数に `false` を指定することで、STOPビットを送信している箇所と、続けて読み取りを行うために `true` を指定している箇所がある。`false` を指定した場合は、コマンド送信後にSTOPビットが送信され、I2Cバスが解放される。`true` を指定した場合は、リスタートコンディションが送信され、続けて読み取りなどのトランザクションを行うことができる。

- **データ読み取り処理におけるSTOPビット:** `SGP40_ReadByte()` 関数内で使用される `i2c_read_blocking()` 関数は、データの読み取り完了後にSTOPビットを送信する。

STOPビットを適切に送信することで、I2Cバス上の他のデバイスとの通信の衝突を防ぎ、正常な通信シーケンスを維持することが可能。

## メインループ

1.  `main()` 関数内で、まず `stdio_init_all()` 関数により標準入出力 (USBシリアルなど) を初期化する。

2.  `i2c_init()` 関数と `gpio_set_function()`、`gpio_pull_up()` 関数を用いてI2C通信を初期化する。

3.  `SGP40_init()` 関数を呼び出し、SGP40センサを初期化する。初期化に失敗した場合はエラーメッセージを出力して終了する。

4.  VOC アルゴリズムのパラメータ構造体 `VocAlgorithmParams voc_params` を定義し、`VocAlgorithm_init(&voc_params)` 関数で初期化する。

5.  無限ループ (`while(true)`) に入り、以下の処理を繰り返す。

    * 仮の温度 (`temperature = 25.0f`) と湿度 (`humidity = 50.0f`) の値を設定する。
    * `SGP40_MeasureRaw(temperature, humidity)` 関数を呼び出し、SGP40 から raw VOC データを取得する。
    * `VocAlgorithm_process(&voc_params, sraw, &voc_index)` 関数を呼び出し、raw VOC データを VOC Index に変換する。
    * 計算された VOC Index の値をシリアルモニタに出力する。
    * `sleep_ms(100)` により、100ミリ秒の遅延を設ける。

## 補足

* **SGP40のI2Cアドレス:**

    ```c
    #define SGP40_ADDR 0x59
    ```

    SGP40センサのI2Cアドレスとして `0x59` を定義している。

* **SGP40 コマンド:**

    ```c
    uint8_t WITH_HUM_COMP[] = {0x26, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 湿度補償付き raw データ用
    ```

    湿度補償付きの raw データを読み取るためのコマンド (`0x26`, `0x0f`) を定義している。後続の6バイトは、温度と湿度のパラメータおよびそれらのCRC値を格納するために使用される。

* **I2Cの初期化:**

    `i2c_init()` 関数でI2C通信速度を設定し、`gpio_set_function()` でGPIOピンをI2C機能に割り当て、`gpio_pull_up()` でプルアップ抵抗を有効にしている。I2C通信にはプルアップ抵抗が不可欠。

* **SGP40へのコマンド送信とデータ読み取り:**

    `i2c_write_blocking()` 関数を用いて SGP40 にコマンドを送信し、`SGP40_ReadByte()` 関数を用いて SGP40 からの応答データを読み取る。コマンド送信時には、STOPビットの送信タイミングを制御するために、`i2c_write_blocking()` の第4引数を適切に設定している。

* **CRC-8チェックサムの計算:**

    送信する温度と湿度のパラメータに対して、データの整合性を保証するために `crc_value()` 関数を用いて CRC-8 チェックサムを計算している。**CRC（巡回冗長検査）は、データ伝送時の誤りを検出するための重要な技術であり、SGP40との通信においてもパラメータの信頼性を高めるために用いられています。**

* **Sensirion VOC アルゴリズムライブラリ:**

    `sensirion_voc_algorithm.h` をインクルードし、取得した raw VOC データを VOC Index に変換するために、`VocAlgorithm_init()` および `VocAlgorithm_process()` 関数を使用している。このライブラリは、SGP40 の特性に基づいて VOC Index を算出するための専門的なアルゴリズムを提供する。

* **CMakeLists.txt:** I2C関連の機能を利用するため、`target_link_libraries` に `hardware_i2c` を追加する必要がある。また、Sensirion VOC アルゴリズムライブラリのソースファイル (`sensirion_voc_algorithm.c`) を `target_sources` に追加する必要がある。

    ```cmake
    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_i2c
    )

    target_sources(${PROJECT_NAME}
        main.c
        sensirion_voc_algorithm.c
    )
    ```

## ライセンス

本プロジェクトでは、Sensirion AG によって提供されている VOC アルゴリズムライブラリ (`sensirion_voc_algorithm.c`) を使用しています。
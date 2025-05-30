# 概要

* I2C通信を用いてSSD1327 OLEDディスプレイに簡単な顔の表情をアニメーション表示する。
* 目と口の形状をランダムに変化させ、簡単なメッセージを表示する。

# 動作

## 初期化

1.  `i2c_init(i2c, I2C_SPEED)` 関数により、指定されたI2Cインスタンス (`i2c`) を定義された通信速度 (`I2C_SPEED`) で初期化する。

2.  `gpio_set_function()` 関数を用いて、SDAピン (`I2C_SDA_PIN`) と SCLピン (`I2C_SCL_PIN`) をI2Cの機能として設定する。

3.  `gpio_pull_up()` 関数を用いて、SDAピンとSCLピンに内蔵プルアップ抵抗を有効にする。I2C通信にはプルアップ抵抗が不可欠。

4.  `ssd1327_init()` 関数を呼び出し、SSD1327 OLEDディスプレイを初期化する。

    * `ssd1327_send_command()` 関数を用いて、ディスプレイのコントラスト設定、表示方向、スリープモード解除などの初期化コマンドを送信する。

## 画面クリア処理

1.  `main()` 関数内で、初期化後に表示バッファ (`buffer`) の全要素を 0 でクリアする。SSD1327は4ビットグレースケールであるため、0は最も暗い状態を示す。

2.  `ssd1327_set_display(buffer)` 関数を呼び出し、クリアされたバッファの内容をディスプレイに送信し、画面を黒で初期化する。

## 表情アニメーション処理 (メインループ内)

1.  無限ループ (`while(true)`) に入り、以下の処理を繰り返す。

2.  **表示バッファのクリア:** ループの最初に、表示バッファ (`buffer`) の全要素を再び 0 でクリアし、前回の表示を消去する。

3.  **目の形状のランダム決定:** `rand() % 2` により、目の形状 (`eye_type`) をランダムに決定する (0: 丸い目, 1: 横長の線)。

4.  **目の描画:** 決定された `eye_type` に基づき、`set_pixel()` 関数を用いて左右の目の形状をバッファに描画する。丸い目の場合は指定された範囲のピクセルを点灯させ、横長の目の場合は中心の水平ラインのピクセルを点灯させる。

5.  **口の形状のランダム決定:** `rand() % 2` により、口の形状 (`mouth_type`) をランダムに決定する (0: ニコニコ, 1: 真一文字)。

6.  **口の描画:** 決定された `mouth_type` に基づき、`set_pixel()` 関数を用いて口の形状をバッファに描画する。ニコニコ口の場合は指定された範囲のピクセルを点灯させ、真一文字の場合は一番下の水平ラインのピクセルを点灯させる。

7.  **メッセージ表示:** 目の形状と口の形状の組み合わせに応じて、`draw_string()` 関数を用いて簡単なメッセージ ("HAPPY", "ZZZZ", "HEY", "HUNGRY") をディスプレイの下部に描画する。`draw_string()` 関数は、`draw_char()` 関数を内部で呼び出し、8x8ドットフォント (`font8x8.h`) を用いて文字を描画する。

8.  **バッファの送信と表示更新:** `ssd1327_set_display(buffer)` 関数を呼び出し、描画された内容を含むバッファをSSD1327に送信し、ディスプレイの表示を更新する。`ssd1327_set_window()` 関数でディスプレイ全体の書き込み範囲を設定した後、`i2c_write_blocking()` 関数を用いてデータを送信する。

9.  **遅延:** `sleep_ms(1000)` 関数により、1秒間の遅延を設け、表情が一定間隔で変化するようにする。

## STOPビットについて

I2C通信では、マスターデバイス (この場合はRaspberry Pi Pico) が通信の開始と終了を制御する。<br>**STOPビット**とは通信の終了を示すもの。

- **コマンド送信におけるSTOPビット:** `ssd1327_send_command()` 関数では、コマンドの送信が完了した後、`i2c_write_blocking()` 関数の第4引数に `false` を指定することで、STOPビットを送信している。これにより、SSD1327はコマンドの受信が完了したことを認識し、処理を開始する。

- **データ送信処理におけるSTOPビット:** `ssd1327_set_display()` 関数では、表示データ（制御バイト `0x40` を含む）の送信が完了した後、`i2c_write_blocking()` 関数の第4引数に `false` を指定している。これは、場合によっては連続したI2Cトランザクション（例えば、コマンド送信直後のデータ送信）を行う際に、STOPビットを送信しないことで効率的な通信を行うためである。ただし、このプログラムの構成では、各 `i2c_write_blocking()` の後に通常STOPビットが送信されるように設定されていることが多い。

    `i2c_write_blocking()` 関数の `false` の指定は、必ずしもSTOPビットを抑制するわけではなく、続けて Start リピートなどの他のI2Cトランザクションを開始する可能性があることを示唆している。多くのI2Cライブラリの実装では、単独の書き込み操作の完了時にはSTOPビットが送信される。

- **データ読み取りがない場合のSTOPビット:** このプログラムでは、SSD1327からデータを読み取る処理は直接的には行っていない。マスターであるPicoからSSD1327へのコマンド送信やデータ送信が主な通信であるため、各 `i2c_write_blocking()` の完了後にSTOPビットが送信され、I2Cバスを解放することが重要となる。

STOPビットを適切に送信することで、I2Cバス上の他のデバイスとの通信の衝突を防ぎ、正常な通信シーケンスを維持することが可能。

## 補足

* **I2Cポートとピン:**

    ```c
    #define i2c i2c1
    #define I2C_SDA_PIN 6
    #define I2C_SCL_PIN 7
    ```

    使用するI2Cインスタンス (`i2c1`) と、SDA (シリアルデータ) ピンとしてGP6、SCL (シリアルクロック) ピンとしてGP7を定義している。

* **I2Cの通信速度:**

    ```c
    #define I2C_SPEED 1000000
    ```

    I2Cの通信速度を 1MHz に設定している。

* **SSD1327のI2Cアドレス:**

    ```c
    #define SSD1327_ADDR 0x3D
    ```

    SSD1327 OLEDディスプレイの一般的なI2Cアドレスとして `0x3D` を定義している。

* **ディスプレイのサイズ:**

    ```c
    #define DISPLAY_WIDTH 128
    #define DISPLAY_HEIGHT 128
    #define DISPLAY_DATA_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 2)
    ```

    SSD1327 OLEDディスプレイの幅と高さ、および必要なデータサイズを定義している。SSD1327は1ピクセルあたり4ビットを使用するため、データサイズは総ピクセル数の半分になる。

* **SSD1327へのコマンド送信:**

    `ssd1327_send_command()` 関数を用いて、SSD1327にコマンドを送信する。コマンドは制御バイト `0x00` とともにI2Cで送信される。

* **SSD1327へのデータ送信:**

    `ssd1327_set_display()` 関数内で、表示データは制御バイト `0x40` とともにI2Cで送信される。

* **フォントデータ:**

    8x8ドットフォントのデータは `font8x8.h` ファイルに定義されている。`draw_char()` 関数はこのフォントデータを用いて文字のピクセルパターンを決定する。

* **CMakeLists.txt:** I2C関連の機能を利用するため、`target_link_libraries` に `hardware_i2c` を追加する必要がある。また、`font8x8.h` ファイルがプロジェクトに含まれるように設定する必要がある場合がある。

    ```cmake
    target_link_libraries(lcd_demo
        pico_stdlib
        hardware_i2c
    )
    ```

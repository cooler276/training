# 概要
* 接続されたLEDを一定間隔で点滅させるプログラム。

# 動作

## 初期化

1. init_led()関数で、LEDを接続したGPIOピンを出力モードに設定し、消灯状態にする。

## メインループ

1. main()関数は、無限ループに入る。
2. ループ内で、GPIOピンをHIGHレベルにしてLEDを点灯させ、wait_ms()関数で250ミリ秒待機する。
3. 次に、GPIOピンをLOWレベルにしてLEDを消灯させ、wait_ms()関数で250ミリ秒待機する。
4. この処理を繰り返すことで、LEDが250ミリ秒間隔で点滅する。

# 補足

* LEDの点滅間隔は、main()関数内のwait_ms(250)の値を変更することで調整できる。
* このプログラムは、**ポーリング方式**でLEDを制御しているため、CPU負荷が高くなる可能性がある。

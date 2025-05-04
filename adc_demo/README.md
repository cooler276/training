# 概要
* 指定されたアナログ入力ピンの値を定期的に読み取り、USBシリアルに出力する。

# 動作
## 初期化

1. ADC (アナログ-デジタル変換器) の初期化
2. ADCを使用するGPIOピンの設定
3. ADC入力チャネルの設定
4. タイマーの設定

## 割り込み処理

1. 設定された時間間隔 (TIMER_INTERVAL_US) ごとに、repeating_timer_callback() 関数が実行される。
2. timer_flag を true に設定し、メインループにAD変換のタイミングを通知する。

## メインループ

1. main() 関数は、while(true) の無限ループに入る。
2. timer_flag が true になると、次の処理を行う。<br> timer_flag を false にリセットする。<br> adc_read() 関数を呼び出して、選択されたアナログ入力チャネルの値を読み取る。<br> printf() 関数で、読み取ったAD値をUSBシリアルに出力する。

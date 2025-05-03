# Development Environment
1. [Raspberry Pi Pico 2 W](https://www.raspberrypi.com/products/raspberry-pi-pico-2/?variant=pico-2-w)<br>評価ボード

2. [Pico-Sensor-Kit-B](https://www.waveshare.com/wiki/Pico-Sensor-Kit-B)<br>センサ拡張ボード

3. [Visual Studio Code](https://code.visualstudio.com/)<br>統合開発環境

4. [Raspberry Pi Pico Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico)<br>Pico用のVScode拡張機能

5. [Live Server](https://marketplace.visualstudio.com/items?itemName=ritwickdey.LiveServer)<br>サーバを簡単に立てるVScode拡張機能<br>ツールで使用する

## Recommend Extension
1. [indent-rainbow](https://marketplace.visualstudio.com/items?itemName=oderwat.indent-rainbow)<br>インデントが色で表示される

2. [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)<br>ソースコードの見た目を自動で整える

3. [Code Spell Checker](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker)<br>英単語のスペルミスを発見してくれる


# Document
1. [Raspberry Pi Pico 2 Data Sheet](https://akizukidenshi.com/goodsaffix/rp2350-datasheet.pdf)

2. [Raspberry Pi Pico Hardware APIs](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html)

3. [Pico-Sensor-Kit-B 回路図](https://files.waveshare.com/upload/4/4a/Pico-Sensor-Kit_V1.1.pdf)

4. [EEPROM Data Sheet](https://files.waveshare.com/upload/5/54/AT24C04B-08B.PDF)

5. [空気センサ Data Sheet](https://files.waveshare.com/upload/8/89/SGP40_Datasheet.pdf)

6. [6軸センサ Data Sheet](https://files.waveshare.com/upload/5/5f/QMI8658A_Datasheet_Rev_A.pdf)

7. ~~[赤外線センサ Data Sheet](https://files.waveshare.com/upload/c/c0/IRM-H638T-TR2.pdf)~~ 未使用

8. [液晶コントローラ Data Sheet](https://files.waveshare.com/upload/a/ac/SSD1327-datasheet.pdf)

9. [3色LED Data Sheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)

10. ~~[モーターコントローラ Data Sheet](https://files.waveshare.com/upload/6/68/PCA96_datasheet.pdf)~~ 未使用

11. ~~[モータードライバ Data Sheet](https://files.waveshare.com/upload/6/62/TB6612FNG_datasheet_en.pdf)~~ 未使用


# Contents
| # | Name | Description | Used hardware | Peripheral | 
| - | - | - | - | - |
| 1 | blink_without_SDK | Pico SDKを使用せずにLEDを点滅させる<br>timerを使ってwait処理を作成 | LED | Timer<br>GPIO |
| 2 | blink_interrupt | Timer割り込みを使ってLEDを点滅させる | LED | Timer<br>GPIO |
| 3 | software_pwm | Timer割り込みを使ってPWM出力を模擬する| LED | Timer<br>GPIO |
| 4 | key_buzzer_demo | スイッチを押すとブザーが鳴る | スイッチ<br>ブザー | PWM<br>GPIO |
| 5 | adc_demo | AD入力のセンサ値を読み出す | 照度センサ<br>ボリューム<br>マイク | ADC |
| 6 | eeprom_demo | EEPROMにデータを書き込み、読み出す | EEPROM | I2C |
| 7 | temperature_humidity_demo | 温湿度センサー値を読み出す | 温湿度センサー | I2C |
| 8 | imu_demo | 6軸センサー値を読み出す | 6軸センサー | I2C |
| 9 | lcd_demo | ディスプレイに出力する | ディスプレイ | I2C |
| 10 | rgb_demo | 3色LEDを光らす | 3色LED | PIO |
| 12 | adc_ble_demo | AD入力のセンサ値を読み出しBLE経由で送信する | 照度センサ<br>ボリューム<br>マイク | ADC<br>BLE |
| 13 | Network_demo | aaaa | LED | Wifi<br>GPIO |

# Tool
| # | Name | Description | 
| - | - | - |
| 1 | live_data_plotter_via_usb | printfで出力したADCなどをグラフ化するツール |
| 2 | live_data_plotter_via_ble | BLEで出力したADCなどをグラフ化するツール |


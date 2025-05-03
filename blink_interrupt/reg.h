// -------------------- リセット制御レジスタ (RESETS) --------------------
#define RESETS_BASE (0x40020000) // リセット制御レジスタのベースアドレス

// 全体リセット制御レジスタ (ソフトウェアリセットのトリガー)
#define RESETS_RESET_RW (*(volatile unsigned long *)(RESETS_BASE + 0x0 + 0x0000))  // 読み書き
#define RESETS_RESET_XOR (*(volatile unsigned long *)(RESETS_BASE + 0x0 + 0x1000)) // 排他的論理和 (ビット反転)
#define RESETS_RESET_SET (*(volatile unsigned long *)(RESETS_BASE + 0x0 + 0x2000)) // ビットセット (1を書き込むと対応するビットが1になる)
#define RESETS_RESET_CLR (*(volatile unsigned long *)(RESETS_BASE + 0x0 + 0x3000)) // ビットクリア (1を書き込むと対応するビットが0になる)

// リセット完了ステータスレジスタ (各ブロックのリセット完了フラグ)
#define RESETS_RESET_DONE_RW (*(volatile unsigned long *)(RESETS_BASE + 0x8 + 0x0000))  // 読み書き
#define RESETS_RESET_DONE_XOR (*(volatile unsigned long *)(RESETS_BASE + 0x8 + 0x1000)) // 排他的論理和 (ビット反転)
#define RESETS_RESET_DONE_SET (*(volatile unsigned long *)(RESETS_BASE + 0x8 + 0x2000)) // ビットセット
#define RESETS_RESET_DONE_CLR (*(volatile unsigned long *)(RESETS_BASE + 0x8 + 0x3000)) // ビットクリア

// -------------------- シングルIOレジスタ (SIO) --------------------
#define SIO_BASE (0xd0000000) // シングルIOレジスタのベースアドレス

// GPIO出力データレジスタ (各GPIOピンの出力値を設定/読み取り)
#define SIO_GPIO_OUT_RW (*(volatile unsigned long *)(SIO_BASE + 0x10))  // 読み書き (全GPIOピンの状態)
#define SIO_GPIO_OUT_SET (*(volatile unsigned long *)(SIO_BASE + 0x18)) // 特定のGPIOピンをHIGHにする (1を書き込む)
#define SIO_GPIO_OUT_CLR (*(volatile unsigned long *)(SIO_BASE + 0x20)) // 特定のGPIOピンをLOWにする (1を書き込む)
#define SIO_GPIO_OUT_XOR (*(volatile unsigned long *)(SIO_BASE + 0x28)) // 特定のGPIOピンの出力を反転させる (1を書き込む)

// GPIO出力イネーブルレジスタ (各GPIOピンの出力有効/無効を設定)
#define SIO_GPIO_OE_RW (*(volatile unsigned long *)(SIO_BASE + 0x30))  // 読み書き (全GPIOピンの出力イネーブル状態)
#define SIO_GPIO_OE_SET (*(volatile unsigned long *)(SIO_BASE + 0x38)) // 特定のGPIOピンを出力有効にする (1を書き込む)
#define SIO_GPIO_OE_CLR (*(volatile unsigned long *)(SIO_BASE + 0x40)) // 特定のGPIOピンを出力無効にする (1を書き込む)
#define SIO_GPIO_OE_XOR (*(volatile unsigned long *)(SIO_BASE + 0x48)) // 特定のGPIOピンの出力イネーブル状態を反転させる (1を書き込む)

// -------------------- IOバンク0レジスタ (IO_BANK0) --------------------
#define IO_BANK0_BASE (0x40028000) // IOバンク0レジスタのベースアドレス

// GPIO10ピンの状態レジスタ (入力レベルなど)
#define IO_BANK0_GPIO10_STATUSL_RW (*(volatile unsigned long *)(IO_BANK0_BASE + 0x050 + 0x0000))  // 読み書き
#define IO_BANK0_GPIO10_STATUSL_XOR (*(volatile unsigned long *)(IO_BANK0_BASE + 0x050 + 0x1000)) // 排他的論理和
#define IO_BANK0_GPIO10_STATUSL_SET (*(volatile unsigned long *)(IO_BANK0_BASE + 0x050 + 0x2000)) // ビットセット
#define IO_BANK0_GPIO10_STATUSL_CLR (*(volatile unsigned long *)(IO_BANK0_BASE + 0x050 + 0x3000)) // ビットクリア

// GPIO10ピンの制御レジスタ (機能選択、プルアップ/プルダウン設定など)
#define IO_BANK0_GPIO10_CTRL_RW (*(volatile unsigned long *)(IO_BANK0_BASE + 0x054 + 0x0000))  // 読み書き
#define IO_BANK0_GPIO10_CTRL_XOR (*(volatile unsigned long *)(IO_BANK0_BASE + 0x054 + 0x1000)) // 排他的論理和
#define IO_BANK0_GPIO10_CTRL_SET (*(volatile unsigned long *)(IO_BANK0_BASE + 0x054 + 0x2000)) // ビットセット
#define IO_BANK0_GPIO10_CTRL_CLR (*(volatile unsigned long *)(IO_BANK0_BASE + 0x054 + 0x3000)) // ビットクリア

// -------------------- パッド制御レジスタ (PADS_BANK0) --------------------
#define PADS_BANK0_BASE (0x40038000) // パッド制御レジスタのベースアドレス

// GPIO10ピンのパッド制御レジスタ (ドライブ強度、スルーレート、プルアップ/プルダウン設定など)
#define PADS_BANK0_BASE_GPIO10_RW (*(volatile unsigned long *)(PADS_BANK0_BASE + 0x2C + 0x0000))  // 読み書き
#define PADS_BANK0_BASE_GPIO10_XOR (*(volatile unsigned long *)(PADS_BANK0_BASE + 0x2C + 0x1000)) // 排他的論理和
#define PADS_BANK0_BASE_GPIO10_SET (*(volatile unsigned long *)(PADS_BANK0_BASE + 0x2C + 0x2000)) // ビットセット
#define PADS_BANK0_BASE_GPIO10_CLR (*(volatile unsigned long *)(PADS_BANK0_BASE + 0x2C + 0x3000)) // ビットクリア

// -------------------- クロック制御レジスタ (CLOCKS) --------------------
#define CLOCKS_BASE (0x40010000) // クロック制御レジスタのベースアドレス

// 基準クロック制御レジスタ
#define CLK_REF_CTRL_RW (*(volatile unsigned long *)(CLOCKS_BASE + 0x30 + 0x0000))  // 読み書き
#define CLK_REF_CTRL_XOR (*(volatile unsigned long *)(CLOCKS_BASE + 0x30 + 0x1000)) // 排他的論理和
#define CLK_REF_CTRL_SET (*(volatile unsigned long *)(CLOCKS_BASE + 0x30 + 0x2000)) // ビットセット
#define CLK_REF_CTRL_CLR (*(volatile unsigned long *)(CLOCKS_BASE + 0x30 + 0x3000)) // ビットクリア

// システムクロック制御レジスタ
#define CLK_SYS_CTRL_RW (*(volatile unsigned long *)(CLOCKS_BASE + 0x3C + 0x0000))  // 読み書き
#define CLK_SYS_CTRL_XOR (*(volatile unsigned long *)(CLOCKS_BASE + 0x3C + 0x1000)) // 排他的論理和
#define CLK_SYS_CTRL_SET (*(volatile unsigned long *)(CLOCKS_BASE + 0x3C + 0x2000)) // ビットセット
#define CLK_SYS_CTRL_CLR (*(volatile unsigned long *)(CLOCKS_BASE + 0x3C + 0x3000)) // ビットクリア

// システムクロック復旧制御レジスタ
#define CLK_SYS_RESUS_CTRL_RW (*(volatile unsigned long *)(CLOCKS_BASE + 0x84 + 0x0000))  // 読み書き
#define CLK_SYS_RESUS_CTRL_XOR (*(volatile unsigned long *)(CLOCKS_BASE + 0x84 + 0x1000)) // 排他的論理和
#define CLK_SYS_RESUS_CTRL_SET (*(volatile unsigned long *)(CLOCKS_BASE + 0x84 + 0x2000)) // ビットセット
#define CLK_SYS_RESUS_CTRL_CLR (*(volatile unsigned long *)(CLOCKS_BASE + 0x84 + 0x3000)) // ビットクリア

// -------------------- 外部水晶発振器レジスタ (XOSC) --------------------
#define XOSC_BASE (0x40048000) // 外部水晶発振器レジスタのベースアドレス

// 外部水晶発振器制御レジスタ
#define XOSC_CTRL_RW (*(volatile unsigned long *)(XOSC_BASE + 0x00 + 0x0000))  // 読み書き
#define XOSC_CTRL_XOR (*(volatile unsigned long *)(XOSC_BASE + 0x00 + 0x1000)) // 排他的論理和
#define XOSC_CTRL_SET (*(volatile unsigned long *)(XOSC_BASE + 0x00 + 0x2000)) // ビットセット
#define XOSC_CTRL_CLR (*(volatile unsigned long *)(XOSC_BASE + 0x00 + 0x3000)) // ビットクリア

// 外部水晶発振器ステータスレジスタ
#define XOSC_STATUS_RW (*(volatile unsigned long *)(XOSC_BASE + 0x04 + 0x0000))  // 読み書き
#define XOSC_STATUS_XOR (*(volatile unsigned long *)(XOSC_BASE + 0x04 + 0x1000)) // 排他的論理和
#define XOSC_STATUS_SET (*(volatile unsigned long *)(XOSC_BASE + 0x04 + 0x2000)) // ビットセット
#define XOSC_STATUS_CLR (*(volatile unsigned long *)(XOSC_BASE + 0x04 + 0x3000)) // ビットクリア

// 外部水晶発振器起動制御レジスタ
#define XOSC_STARTUP_RW (*(volatile unsigned long *)(XOSC_BASE + 0x0C + 0x0000))  // 読み書き
#define XOSC_STARTUP_XOR (*(volatile unsigned long *)(XOSC_BASE + 0x0C + 0x1000)) // 排他的論理和
#define XOSC_STARTUP_SET (*(volatile unsigned long *)(XOSC_BASE + 0x0C + 0x2000)) // ビットセット
#define XOSC_STARTUP_CLR (*(volatile unsigned long *)(XOSC_BASE + 0x0C + 0x3000)) // ビットクリア

// 外部水晶発振器カウンタレジスタ
#define XOSC_COUNT_RW (*(volatile unsigned long *)(XOSC_BASE + 0x10 + 0x0000))  // 読み書き
#define XOSC_COUNT_XOR (*(volatile unsigned long *)(XOSC_BASE + 0x10 + 0x1000)) // 排他的論理和
#define XOSC_COUNT_SET (*(volatile unsigned long *)(XOSC_BASE + 0x10 + 0x2000)) // ビットセット
#define XOSC_COUNT_CLR (*(volatile unsigned long *)(XOSC_BASE + 0x10 + 0x3000)) // ビットクリア

// -------------------- タイマー0レジスタ (TIMER0) --------------------
#define TIMER0_BASE (0x400b0000) // タイマー0レジスタのベースアドレス

// タイマー上位16ビット (書き込み)
#define TIMER0_TIMEHW (*(volatile unsigned long *)(TIMER0_BASE + 0x00))
// タイマー下位16ビット (書き込み)
#define TIMER0_TIMELW (*(volatile unsigned long *)(TIMER0_BASE + 0x04))
// タイマー上位32ビット (読み取り)
#define TIMER0_TIMEHR (*(volatile unsigned long *)(TIMER0_BASE + 0x08))
// タイマー下位32ビット (読み取り)
#define TIMER0_TIMELR (*(volatile unsigned long *)(TIMER0_BASE + 0x0C))
// アラーム0設定値
#define TIMER0_ALARM0 (*(volatile unsigned long *)(TIMER0_BASE + 0x10))
// アラーム1設定値
#define TIMER0_ALARM1 (*(volatile unsigned long *)(TIMER0_BASE + 0x14))
// アラーム2設定値
#define TIMER0_ALARM2 (*(volatile unsigned long *)(TIMER0_BASE + 0x18))
// アラーム3設定値
#define TIMER0_ALARM3 (*(volatile unsigned long *)(TIMER0_BASE + 0x1C))
// アラーム有効ビット
#define TIMER0_ARMED (*(volatile unsigned long *)(TIMER0_BASE + 0x20))
// 生タイマー上位32ビット (読み取り、ポーズ中でもカウント)
#define TIMER0_TIMERAWH (*(volatile unsigned long *)(TIMER0_BASE + 0x24))
// 生タイマー下位32ビット (読み取り、ポーズ中でもカウント)
#define TIMER0_TIMERAWL (*(volatile unsigned long *)(TIMER0_BASE + 0x28))
// デバッグポーズ制御
#define TIMER0_DBGPAUSE (*(volatile unsigned long *)(TIMER0_BASE + 0x2C))
// タイマー一時停止制御
#define TIMER0_PAUSE (*(volatile unsigned long *)(TIMER0_BASE + 0x30))
// ロック制御 (書き込み保護)
#define TIMER0_LOCKED (*(volatile unsigned long *)(TIMER0_BASE + 0x34))
// クロックソース選択
#define TIMER0_SOURCE (*(volatile unsigned long *)(TIMER0_BASE + 0x38))
// 割り込み要求フラグ
#define TIMER0_INTR (*(volatile unsigned long *)(TIMER0_BASE + 0x3C))
// 割り込みイネーブルビット
#define TIMER0_INTE (*(volatile unsigned long *)(TIMER0_BASE + 0x40))
// 割り込み強制フラグ
#define TIMER0_INTF (*(volatile unsigned long *)(TIMER0_BASE + 0x44))
// 割り込みステータス (クリア用)
#define TIMER0_INTS (*(volatile unsigned long *)(TIMER0_BASE + 0x48))

// -------------------- タイマー1レジスタ (TIMER1) --------------------
#define TIMER1_BASE (0x400b8000) // タイマー1レジスタのベースアドレス

// タイマー上位16ビット (書き込み用)
#define TIMER1_TIMEHW (*(volatile unsigned long *)(TIMER1_BASE + 0x00))
// タイマー下位16ビット (書き込み用)
#define TIMER1_TIMELW (*(volatile unsigned long *)(TIMER1_BASE + 0x04))
// タイマー上位32ビット (読み取り用)
#define TIMER1_TIMEHR (*(volatile unsigned long *)(TIMER1_BASE + 0x08))
// タイマー下位32ビット (読み取り用)
#define TIMER1_TIMELR (*(volatile unsigned long *)(TIMER1_BASE + 0x0C))
// アラーム0設定値
#define TIMER1_ALARM0 (*(volatile unsigned long *)(TIMER1_BASE + 0x10))
// アラーム1設定値
#define TIMER1_ALARM1 (*(volatile unsigned long *)(TIMER1_BASE + 0x14))
// アラーム2設定値
#define TIMER1_ALARM2 (*(volatile unsigned long *)(TIMER1_BASE + 0x18))
// アラーム3設定値
#define TIMER1_ALARM3 (*(volatile unsigned long *)(TIMER1_BASE + 0x1C))
// アラーム有効ビット (各アラームに対応するビットをセットすると有効になる)
#define TIMER1_ARMED (*(volatile unsigned long *)(TIMER1_BASE + 0x20))
// 生タイマー上位32ビット (読み取り専用、ポーズ中でもカウントが進む)
#define TIMER1_TIMERAWH (*(volatile unsigned long *)(TIMER1_BASE + 0x24))
// 生タイマー下位32ビット (読み取り専用、ポーズ中でもカウントが進む)
#define TIMER1_TIMERAWL (*(volatile unsigned long *)(TIMER1_BASE + 0x28))
// デバッグポーズ制御 (デバッグ時にタイマーの動作を一時停止させる)
#define TIMER1_DBGPAUSE (*(volatile unsigned long *)(TIMER1_BASE + 0x2C))
// タイマー一時停止制御 (通常動作時にタイマーのカウントを一時停止させる)
#define TIMER1_PAUSE (*(volatile unsigned long *)(TIMER1_BASE + 0x30))
// ロック制御 (タイマーの設定を誤って変更しないように保護する)
#define TIMER1_LOCKED (*(volatile unsigned long *)(TIMER1_BASE + 0x34))
// クロックソース選択 (タイマーの動作クロックを選択する)
#define TIMER1_SOURCE (*(volatile unsigned long *)(TIMER1_BASE + 0x38))
// 割り込み要求フラグ (各イベントに対応する割り込み要求が発生するとセットされる)
#define TIMER1_INTR (*(volatile unsigned long *)(TIMER1_BASE + 0x3C))
// 割り込みイネーブルビット (各イベントに対応する割り込みを有効にする)
#define TIMER1_INTE (*(volatile unsigned long *)(TIMER1_BASE + 0x40))
// 割り込み強制フラグ (ソフトウェアから強制的に割り込みを発生させる)
#define TIMER1_INTF (*(volatile unsigned long *)(TIMER1_BASE + 0x44))
// 割り込みステータス (割り込み要求フラグをクリアするために使用)
#define TIMER1_INTS (*(volatile unsigned long *)(TIMER1_BASE + 0x48))
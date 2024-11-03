# M5UnitENVPro_Google_Spreadsheet

M5Stack Unit ENV Pro (BME688) により計測されたデータをスプレッドシートに記録するプログラム。<br><br>

動作確認済みマイコン
* Seeed Studio XIAO ESP32C3

## スプレッドシートとGASの準備

スプレッドシートを開いてURLを記録する。<br>
拡張機能タブから Apps Script へ移動。<br>

以下のコードをコピペする。<br>
2行目の`https://docs.google.com/spreadsheets/********`を先に記録したスプレッドのURLで置き換えておく。<br>

code.gs
```js
function doGet(e) {
  const url = "https://docs.google.com/spreadsheets/********";
  const ss = SpreadsheetApp.openByUrl(url);
  const sheet = ss.getSheets()[0];
  const d = new Date();
  const params = {
    "timestamp": d.getTime(),
    "localtime": d.toLocaleString(),
    "mcu_timestamp": e.parameter.mcu_timestamp,
    "iaq": e.parameter.iaq,
    "iaq_accuracy": e.parameter.iaq_accuracy,
    "temperature": e.parameter.temperature,
    "pressure": e.parameter.pressure,
    "humidity": e.parameter.humidity,
    "gas_resistance": e.parameter.gas_resistance,
    "stabilization_status": e.parameter.stabilization_status,
    "run_in_status": e.parameter.run_in_status
  };
  sheet.appendRow(Object.values(params));
  return ContentService.createTextOutput('sccess');
}
```

デプロイボタンから新しいデプロイを選択し、ウェブアプリとしてデプロイする。<br>
このとき、アクセスできるユーザーは全員にしておく。<br>
デプロイが完了したらウェブアプリのURLを記録しておく。

## Platform IO

マイコンとセンサ (M5Stack Unit ENV Pro) を用意し接続する。<br>

`ssid`、`password`を自分のwifiルーターのものに、`url`をウェブアプリのURLにそれぞれ変更する。<br>
必要に応じて`#define DEBUG_PRINT`のコメントアウトを外す。<br>

path: src/main.cpp
```cpp
// #define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

// Wi-Fi
const char *ssid = "********";
const char *password = "********";

// Google Spreadsheet
const String url = "https://script.google.com/macros/s/********/exec";
```

マイコンとPCを接続しプログラムをuploadする。

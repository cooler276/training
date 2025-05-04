const { SerialPort } = require('serialport');
const { WebSocketServer } = require('ws');

// シリアルポートの設定 (ご自身の環境に合わせてください)
const serialPortPath = 'COM8'; // 例: Linuxの場合
const baudRate = 115200;

// WebSocketサーバーの設定
const wsPort = 8080;
const wss = new WebSocketServer({ port: wsPort });

let wsClient;

// シリアルポートの初期化
const serialPort = new SerialPort({ path: serialPortPath, baudRate: baudRate });

serialPort.on('open', () => {
    console.log('シリアルポートが開きました');
});

serialPort.on('data', (data) => {
    const receivedData = data.toString().trim();
    // "AD Value: XXXX" という形式を想定
    const match = receivedData.match(/AD Value: (\d+)/);
    if (match && match[1]) {
        const adValue = parseInt(match[1]);
        if (wsClient && wsClient.readyState === 1) { // WebSocket接続が開いている場合
            wsClient.send(adValue.toString());
        } else {
            console.log('WebSocketクライアントが存在しないか、接続が閉じられています:', adValue);
        }
    }
});

serialPort.on('error', (err) => {
    console.error('シリアルポートエラー:', err);
});

// WebSocketサーバーの接続処理
wss.on('connection', ws => {
    console.log('WebSocketクライアントが接続しました');
    wsClient = ws; // 最新の接続クライアントを保持

    ws.on('close', () => {
        console.log('WebSocketクライアントが切断しました');
        wsClient = null;
    });

    ws.on('error', error => {
        console.error('WebSocketエラー:', error);
        wsClient = null;
    });
});

console.log(`WebSocketサーバー起動: ws://localhost:${wsPort}`);
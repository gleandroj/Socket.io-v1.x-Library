# Socket.io-v1.x-Library
Socket.io Library for Arduino - Forked from [Socket.io-v1.x-Library](https://github.com/washo4evr/Socket.io-v1.x-Library)

Works with ESP8266, Ethernet Library and Arduino Uno (Be careful with memory usage)

## How To Use This Library

```
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "echo.websocket.org";
int port = 9000;
SocketIOClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  client.connect(hostname, port);
  client.setDataArrivedDelegate(ondata);
  client.emit("arduino connect", "Hello World!");
}

void loop() {
  client.monitor();
}

void ondata(SocketIOClient client, String event, String data) {
  Serial.println(data);
}
```
#include <SocketIOClient.h>

unsigned long lastPing = 0UL;

const char clientHandshakeLine1[] PROGMEM = "GET /socket.io/1/?EIO=3&transport=polling&b64=true HTTP/1.1\r\n";
const char clientHandshakeLine2[] PROGMEM = "Host: ";
const char clientHandshakeLine3[] PROGMEM = "\r\n";
const char clientHandshakeLine4[] PROGMEM = "Origin: Arduino\r\n";
const char clientHandshakeLine5[] PROGMEM = "Connection: keep-alive\r\n\r\n";
const char clientHandshakeLine6[] PROGMEM = "GET /socket.io/1/websocket/?EIO=3&transport=websocket&b64=true&sid=";
const char clientHandshakeLine7[] PROGMEM = " HTTP/1.1\r\n";
const char clientHandshakeLine8[] PROGMEM = "Sec-WebSocket-Key: ";
const char clientHandshakeLine9[] PROGMEM = "Sec-WebSocket-Version: 13\r\n";
const char clientHandshakeLine10[] PROGMEM = "Upgrade: websocket\r\n";
const char clientHandshakeLine11[] PROGMEM = "Connection: Upgrade\r\n";

const char* const WebSocketClientStringTable[] PROGMEM =
{
    clientHandshakeLine1,
    clientHandshakeLine2,
    clientHandshakeLine3,
    clientHandshakeLine4,
    clientHandshakeLine5,
	clientHandshakeLine6,
	clientHandshakeLine7,
	clientHandshakeLine8,
	clientHandshakeLine9,
	clientHandshakeLine10,
	clientHandshakeLine11
};

String SocketIOClient::getStringTableItem(int index) {
    char buffer[70];
    strcpy_P(buffer, (char*)pgm_read_word(&(WebSocketClientStringTable[index])));
    return String(buffer);
}

bool SocketIOClient::connect(char thehostname[], int theport, char thensp[]) {
	return connect(thehostname, thehostname, theport, thensp);
}

bool SocketIOClient::connect(char theserver[], char thehostname[], int theport, char thensp[]) {
	if(!client.connect(theserver, theport)) return 0;
	severhost = theserver;
	hostname = thehostname;
	port = theport;
	nsp = thensp;
	sendHandshake(hostname);
	bool result = readHandshake();
	return result;
}

bool SocketIOClient::connected() {
	return client.connected();
}

void SocketIOClient::disconnect() {
	client.stop();
}

void SocketIOClient::setDataArrivedDelegate(EventDelegate dataArrivedDelegate) {
	  _dataArrivedDelegate = dataArrivedDelegate;
}

void SocketIOClient::parser(int index) {
	String rcvdmsg = "";
	int sizemsg = databuffer[index + 1];   // 0-125 byte, index ok
	if (databuffer[index + 1]>125) {
		sizemsg = databuffer[index + 2];    // 126-255 byte
		index += 1;       // index correction to start
	}
	for (int i = index + 2; i < index + sizemsg + 2; i++) rcvdmsg += (char)databuffer[i];
	//Serial.print("Received message = ");
	//Serial.println(rcvdmsg);
	switch (rcvdmsg[0])
	{
		//Ping recivied
		case '2':
			sendMessage("3");//Pong
			break;
        //Pong recivied
		case '3':
		    //Serial.println("pong recivied.");
			return;
			break;
		case '4':
			switch (rcvdmsg[1])
			{
				case '0':
					Serial.println(F("Conneted to websocket server."));
					break;
				case '2':
					if (_dataArrivedDelegate != NULL) {
						_dataArrivedDelegate(*this, rcvdmsg.substring(rcvdmsg.indexOf("[\"")+2, rcvdmsg.indexOf("\",")), rcvdmsg.substring(rcvdmsg.indexOf("\",") + 2, rcvdmsg.indexOf("]")));
					}
					break;
			}
	}
}

bool SocketIOClient::monitor() {
	int index = -1;
	int index2 = -1;
	String tmp = "";
	*databuffer = 0;
	
	if (!client.connected()) {
		return 0;
	}

	if (!client.available()) {
		//Validar
		if(((millis() - lastPing) >= 20000UL)){
			sendMessage("2");//Ping
			lastPing = millis();
		}
	}
	
	char which;

	while (client.available()) {
		readLine();
		tmp = databuffer;
		dataptr = databuffer;
		index = tmp.indexOf((char)129);	//129 DEC = 0x81 HEX = sent for proper communication
		index2 = tmp.indexOf((char)129, index + 1);
		
		if (index != -1) parser(index);
		if (index2 != -1) parser(index2);
	}
	
	return 1;
}

void SocketIOClient::sendHandshake(char hostname[]) {
	String request = "";
	request +=	getStringTableItem(0);//"GET /socket.io/1/?EIO=3&transport=polling&b64=true HTTP/1.1\r\n";
	request +=	getStringTableItem(1);//"Host: ";
	request += hostname;
	request += getStringTableItem(2);//"\r\n";
	request += getStringTableItem(3);//"Origin: Arduino\r\n";
	request += getStringTableItem(4);//"Connexion: keep-alive\r\n\r\n";

	client.print(request);
}

bool SocketIOClient::waitForInput(void) {
	unsigned long now = millis();
	while (!client.available() && ((millis() - now) < 30000UL)) { ; }
	return client.available();
}

void SocketIOClient::eatHeader(void) {
	while (client.available()) {	// consume the header
		readLine();
		if (strlen(databuffer) == 0) break;
	}
}

bool SocketIOClient::readHandshake() {

	if (!waitForInput()) return false;

	// check for happy "HTTP/1.1 200" response
	readLine();
	if (atoi(&databuffer[9]) != 200) {
		while (client.available()) readLine();
		client.stop();
		return false;
	}
	eatHeader();
	readLine();
	String tmp = databuffer;

	int sidindex = tmp.indexOf("sid");
	int sidendindex = tmp.indexOf("\"", sidindex + 6);
	int count = sidendindex - sidindex - 6;

	for (int i = 0; i < count; i++) sid[i] = databuffer[i + sidindex + 6];

	while (client.available()) readLine();
	client.stop();
	delay(200);
	
	if (!client.connect(hostname, port)) return false;
	
	String request = "";
	request += getStringTableItem(5);//"GET /socket.io/1/websocket/?EIO=3&transport=websocket&b64=true&sid=" ;
	request += sid ;
	request += getStringTableItem(6);//" HTTP/1.1\r\n" ;
	request += getStringTableItem(1);//"Host: " ;
	request += hostname;
	request += getStringTableItem(2);//"\r\n" ;
	request += getStringTableItem(3);//"Origin: Arduino\r\n" ;
	request += getStringTableItem(7);//"Sec-WebSocket-Key: " ;
	request += sid ;
	request += getStringTableItem(2);//"\r\n" ;
	request += getStringTableItem(8);//"Sec-WebSocket-Version: 13\r\n" ;
	request += getStringTableItem(9);//"Upgrade: websocket\r\n" ;
	request += getStringTableItem(10);//"Connection: Upgrade\r\n" ;
	request += getStringTableItem(2);//"\r\n" ;

	client.print(request);

	if (!waitForInput()) return false;
	readLine();
	if (atoi(&databuffer[9]) != 101) {	// check for "HTTP/1.1 101 response, means Updrage to Websocket OK
		while (client.available()) readLine();

		client.stop();
		return false;
	}
	readLine();
	readLine();
	readLine();
	for (int i = 0; i < 28; i++) key[i] = databuffer[i + 22];


	eatHeader();
	
	sendMessage("52");
	monitor();
	if(nsp != "") sendNSP();

	return true;
}

void SocketIOClient::readLine() {
	for (int i = 0; i < DATA_BUFFER_LEN; i++)
		databuffer[i] = ' ';
	dataptr = databuffer;
	
	while (client.available() && (dataptr < &databuffer[DATA_BUFFER_LEN - 2])){
		char c = client.read();
		//Serial.print(c);			//Can be used for debugging
		if (c == 0) Serial.print("");
		else if (c == 255) Serial.println("");
		else if (c == '\r') { ; }
		else if (c == '\n') break;
		else *dataptr++ = c;
	}
	*dataptr = 0;
}

void SocketIOClient::sendMessage(const char* message){
	byte frame[10];
	int indexStartRawData = -1;
	int length = strlen(message);
	frame[0] = (byte)129;
	
	if (length <= 125)
	{
		frame[1] = (byte)length;
		indexStartRawData = 2;
	}
	else if (length >= 126 && length <= 65535)
	{
		frame[1] = (byte)126;
		frame[2] = (byte)((length >> 8) & 255);
		frame[3] = (byte)(length & 255);
		indexStartRawData = 4;
	}
	
	byte response[indexStartRawData + length];

	int i, reponseIdx = 0;

	//Add the frame bytes to the reponse
	for (i = 0; i < indexStartRawData; i++){
		response[reponseIdx] = frame[i];
		reponseIdx++;
	}

	//Add the data bytes to the response
	for (i = 0; i < length; i++){
		response[reponseIdx] = *(message+i);
		reponseIdx++;
	}
	
	client.write(response, indexStartRawData + length);
	client.flush();
}

void SocketIOClient::sendNSP() {
	char* buffer = (char*) malloc(strlen(nsp)+4);
	sprintf(buffer, "40/%s", nsp);
	sendMessage(buffer);
	free(buffer);
}

void SocketIOClient::emit(const char* RID, const char* JSON){
	char* buffer = (char*) malloc(strlen(RID)+strlen(JSON)+15);
	sprintf(buffer, "42/%s,[\"%s\",%s]", nsp, RID, JSON);
	sendMessage(buffer);
	free(buffer);
}


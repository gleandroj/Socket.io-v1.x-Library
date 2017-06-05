#include "Arduino.h"

#include <Ethernet.h>
#include "SPI.h"	

#define DATA_BUFFER_LEN 70
#define SID_LEN 24

class SocketIOClient {
public:
	typedef void (*EventDelegate)(SocketIOClient client, String event, String data);
	void setDataArrivedDelegate(EventDelegate dataArrivedDelegate);
	bool connect(char server[], char hostname[], int port = 80, char nsp[] = "");
	bool connect(char hostname[], int port = 80, char nsp[] = "");
	bool connected();
	void disconnect();
	bool monitor();
	void sendNSP();
	void emit(const char* RID, const char* JSON);
private:
	void parser(int index);
	void sendHandshake(char hostname[]);
	void sendMessage(const char* message);
	EthernetClient client;
	bool readHandshake();
	void readLine();
	char *dataptr;
	char databuffer[DATA_BUFFER_LEN];
	char sid[SID_LEN];
	char key[28];
	char *severhost;
	char *hostname;
	char *nsp;
	int port;
	
	EventDelegate _dataArrivedDelegate;
	
	String getStringTableItem(int index);
	bool waitForInput(void);
	void eatHeader(void);
};

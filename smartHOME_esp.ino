#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

const int EEPROM_SIZE = 100;

const int LIGHTS = 4;
const int PLUG = 5;

char* EEPROMs;
char* SSID = NULL;
char* password = NULL;

char* uri;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Setup begin");

  pinMode(LIGHTS, OUTPUT);
  pinMode(PLUG, OUTPUT);

  EEPROM.begin(EEPROM_SIZE);

  Serial.print("EEPROM: ");
  printEEPROM();
  Serial.println();

  //saveNetwork(" ", " ");
  getNetwork();
  Serial.println("Getting network...");
  Serial.printf("SSID: %s\nPassword: %s\n", SSID, password);
  

  if (SSID == NULL && password == NULL) {
    Serial.println("Makeing network...");
    makeNetwork();
  } else {
    Serial.println("Connecting to network...");
    connectToNetwork();
  }
}



void loop() {
  server.handleClient();
}



void saveNetwork(char* SSID, char* password) {
  int i = 0;

  writeToAddress(i, "SSID:$");
  writeToAddress(i, SSID);
  writeToAddress(i, "$;Password:$");
  writeToAddress(i, password);
  writeToAddress(i, "\";");
}



void getNetwork() {
  EEPROMs = readEEPROM();

  if (indexOf("SSID:$", EEPROMs, 0) != -1 && indexOf("$;Password:$", EEPROMs, 0) != -1) {
    SSID = subString(EEPROMs, indexOf("SSID:$", EEPROMs, 0) + 6, indexOf("$;", EEPROMs, 0) - 1);
    password = subString(EEPROMs, indexOf("$;Password:", EEPROMs, 0) + 12, indexOf("$;", EEPROMs, indexOf("$;", EEPROMs, 0) + 2) - 1);
  
    Serial.println(SSID);
    Serial.println(password);
  }
}



void writeToAddress(int& address, char* data) {
  char i;
  
  for (i = 0; *(data + i) != 0; i++) {
    EEPROM.write(address + i, *(data + i));
  }

  address += i;

  EEPROM.commit();
}



void printEEPROM() {
  char c = EEPROM.read(0);
  
  for (int i = 1; i < EEPROM_SIZE; i++) {
    Serial.printf("At %d: %c\n", i - 1, c);

    c = EEPROM.read(i);
  }
}


char* readEEPROM() {
  char* s = new char[EEPROM_SIZE];

  for (int i = 0; i < EEPROM_SIZE; i++) {
    *(s + i) = EEPROM.read(i);
  }

  return s;
}


void clearEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 255);
  }

  EEPROM.commit();
}


int indexOf(char* c, char* s, int startIndex) {
  bool b = true; 

  for (int i = startIndex; *(s + i) != 0; i++) {
    b = true;
    
    for (int j = 0; *(c + j) != 0; j++) {
      if (*(s + i + j) != *(c + j)) {
        b = false;
        break;
      }
    }

    if (b)
      return i;
  }

  return -1;
}

char* subString(char* s, int beginIndex, int endIndex) {
  char* newString = new char[endIndex - beginIndex + 3];

  int i;
  for (i = beginIndex; *(s + i) != 0 && i <= endIndex; i++) {
    *(newString + i - beginIndex) = *(s + i);
  }

  *(newString + i - beginIndex) = 0;
  
  return newString;
}

void makeNetwork() {
  WiFi.softAPConfig(IPAddress(192, 168, 24, 64), IPAddress(192, 168, 4, 9), IPAddress(255, 255, 255, 0));
  Serial.println("IP address configured");
  
  WiFi.softAP("esp8266", "espPassowrd");
  Serial.println("Setup WiFi done");

  

  server.on("/", []() {
    server.send(200, "text/html", "<html><body><p>Hello</p></body></html>");
  });
  
  server.onNotFound([]() {
    Serial.print("Page not found: ");

    char* c = new char[EEPROM_SIZE];

    server.uri().toCharArray(c, EEPROM_SIZE);
    
    uri = c;

    Serial.printf("URI: %s\n", uri);
    
    if (indexOf("SSID:$", uri, 0) != -1 && indexOf("$;Password:$", uri, 0) != -1) {

      Serial.println("Uso u if!");
      int x = 0;
      writeToAddress(x, uri);


      server.stop();
      server.close();

      WiFi.mode(WIFI_OFF);

      ESP.restart();
    }
  });

  server.begin();

  
}

void connectToNetwork() {
  
  Serial.println("Connecting to network!!!");

  WiFi.hostname("smarthome");
  WiFi.begin(SSID, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to ");
    Serial.print("ssid");
  }
  Serial.println("ssid");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", "<html><body><p>I am on your network</p></body></html>");
  });

  server.on("/0", []() {
    server.send(200, "text/html", "<html><body><p>I am on your network</p></body></html>");

    digitalWrite(LIGHTS, LOW);
    digitalWrite(PLUG, LOW);
  });

  server.on("/1", []() {
    server.send(200, "text/html", "<html><body><p>I am on your network</p></body></html>");

    digitalWrite(LIGHTS, LOW);
    digitalWrite(PLUG, HIGH);
  });

  server.on("/2", []() {
    server.send(200, "text/html", "<html><body><p>I am on your network</p></body></html>");

    digitalWrite(LIGHTS, HIGH);
    digitalWrite(PLUG, LOW);
  });

  server.on("/3", []() {
    server.send(200, "text/html", "<html><body><p>I am on your network</p></body></html>");

    digitalWrite(LIGHTS, HIGH);
    digitalWrite(PLUG, HIGH);
  });

  
  server.begin();
}

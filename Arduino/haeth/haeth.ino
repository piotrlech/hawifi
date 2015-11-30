/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <RCSwitch.h>
#include <Wire.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "sha256.h"
#include "piotr.h"
#include "pass.h"


unsigned int localPort = 8888;      // local port to listen for UDP packets

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
//IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
//IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
//IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server
IPAddress timeServer(91, 189, 89, 199); // ntp.ubuntu.com NTP server

int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
unsigned long unixTime;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(83);

#define START_CMD_CHAR '*'
#define END_CMD_CHAR '#'
#define DIV_CMD_CHAR '|'

#define CMD_TURN_IT_ON_OFF 10
#define CMD_READ_STATUS 11
#define CMD_SEND_TRIGGERS 12
#define CMD_SET_TIME 13

#define MAX_COMMAND 20  // max command number code. used for error checking.
#define MIN_COMMAND 10  // minimum command number code. used for error checking. 
#define IN_STRING_LENGHT 40
#define MAX_ANALOGWRITE 255
#define PIN_HIGH 3
#define PIN_LOW 2
#define MY_1ST    "10000"
#define MY_2ND    "01000"
#define MY_3RD    "00100"
#define MY_4TH    "00010"
#define MY_5TH    "00001"
#define MY_6TH    "10001"
#define MAX_CMD  20

String inText;
RCSwitch mySwitch = RCSwitch();
// the current address in the EEPROM (i.e. which byte we're going to write to next)
AlarmId onTimer = 255;
AlarmId offTimer = 255;
tmElements_t tm;
unsigned long prevNonce = 179124;
unsigned long currNonce = 0;
int nonceAddr = 0;

#define MAX_ACTION            10      // Maximum length of the HTTP action that can be parsed.

#define MAX_PATH              100      // Maximum length of the HTTP request path that can be parsed.
                                      // There isn't much memory available so keep this short!

#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20  // Size of buffer for incoming request data.
                                                          // Since only the first line is parsed this
                                                          // needs to be as large as the maximum action
                                                          // and path plus a little for whitespace and
                                                          // HTTP version.

#define TIMEOUT_MS            500    // Amount of time in milliseconds to wait for
                                     // an incoming request to finish.  Don't set this
                                     // too high or your server could be slow to respond.

uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];
char command[MAX_ACTION+1];
char nonce[MAX_PATH+1];
char hmac[MAX_PATH+1];
int resetLevel = 0;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  // Transmitter is connected to Arduino Pin #8
  mySwitch.enableTransmit(8);
  //mySwitch.setPulseLength(700);
  mySwitch.setRepeatTransmit(20);

  //Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  //eepromPutLng(prevNonce);

  // Enter a MAC address and IP address for your controller below.
  // The IP address will be dependent on your local network:
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  //IPAddress ip(192, 168, 100, 149);
  /*byte ip[] =      { 192, 168, 100, 149 };
  byte dns[] =     { 192, 168, 100, 253 };
  byte gateway[] = { 192, 168, 100, 253 };
  byte subnet[] =  { 255, 255, 255, 128 };*/
    
  byte ip[] =      { 192, 168, 2,  42 };
  byte dns[] =     { 192, 168, 1,   1 };
  byte gateway[] = { 192, 168, 2,   1 };
  byte subnet[] =  { 255, 255, 255, 0 };
    
  /*byte ip[] =      { 192, 168, 1, 142 };
  byte dns[] =     { 192, 168, 1,   1 };
  byte gateway[] = { 192, 168, 1,   1 };
  byte subnet[] =  { 255, 255, 255, 0 };*/
    
  // start the Ethernet connection and the server:
  //Ethernet.begin(mac, ip, dns, gateway, subnet);
  Ethernet.begin(mac);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }

  int i;
  i = Udp.begin(localPort);
  //Serial.println(i);
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  testNtp();
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  digitalClockDisplay();

  setAlarms();
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
          // Clear the incoming data buffer and point to the beginning of it.
      bufindex = 0;
      memset(&buffer, 0, sizeof(buffer));
      
      // Clear action and path strings.
      memset(&action,  0, sizeof(action));
      memset(&path,    0, sizeof(path));
      memset(&command, 0, sizeof(command));
      memset(&nonce,   0, sizeof(nonce));
      memset(&hmac,    0, sizeof(hmac));
  
      // Set a timeout for reading all the incoming data.
      unsigned long endtime = millis() + TIMEOUT_MS;
      
      // Read all the incoming data until it can be parsed or the timeout expires.
      bool parsed = false;
      while (!parsed && (millis() < endtime) && (bufindex < BUFFER_SIZE)) {
        if (client.available()) {
          buffer[bufindex++] = client.read();
        }
        parsed = parseRequest(buffer, bufindex, action, path);
        Alarm.delay(0);
      }
  
      if (parsed) {
        resetLevel = -1;
        Serial.println(F("Processing request"));
        Serial.print(F("Action: ")); Serial.println(action);
        Serial.print(F("Path: ")); Serial.println(path);
        if (strcmp(action, "GET") == 0) {
          // Respond with the path that was accessed.
          // First send the success response code.
          client.println(F("HTTP/1.1 200 OK"));
          Alarm.delay(1);
          // Then send a few headers to identify the type of data returned and that
          // the connection will not be held open.
          client.println(F("Content-Type: text/plain"));
          client.println(F("Connection: close"));
          client.println(F("Server: W5100"));
          // Send an empty line to signal start of body.
          client.println(F(""));
          // Now send the response data.
          client.print(F("cmd "));
          Alarm.delay(1);
          parsed = parsePath(path, command, nonce, hmac);
          if(parsed) {
            client.print(command);
            String beforeHmac = String();
            uint8_t *hash;
            // HMAC_SHA256("key", "The quick brown fox jumps over the lazy dog") = 
            // 0xf7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8
            /*beforeHmac = beforeHmac + "The quick brown fox jumps over the lazy dog";
            Serial.print("beforeHmac: "); Serial.println(beforeHmac);
            Serial.print("Hmac key: "); Serial.println("key");
            Sha256.initHmac((uint8_t*)"key", 3); // key, and length of key in bytes
            Sha256.print(beforeHmac);
            hash = Sha256.resultHmac();
            Serial.print("afterHmac: "); Serial.println(bin2hex(hash));
            beforeHmac = "";*/
            
            beforeHmac = beforeHmac + "/" + HMAC_PASS + "/" + command + "/" + nonce + "/";
            //Serial.print("beforeHmac: "); Serial.println(beforeHmac);
            //Serial.print("Hmac key: "); Serial.println(HMAC_KEY);
            Sha256.initHmac((uint8_t*)HMAC_KEY, 13); // key, and length of key in bytes
            Sha256.print(beforeHmac);
            hash = Sha256.resultHmac();
            //Serial.print("afterHmac: "); Serial.println(bin2hex(hash));
  
            String strNonce = String();
            strNonce = nonce;
            unsigned long lngNonce = strNonce.toInt();
            //Serial.print("nonces "); Serial.print(eepromGetLng()); Serial.print(" vs "); Serial.println(lngNonce);
  
            if ((strcmp(hmac, bin2hex(hash)) == 0) && (eepromGetLng() < lngNonce))
            //if(true)
            {
              eepromPutLng(lngNonce);
              //Serial.print("eepromGetLng "); Serial.println(eepromGetLng());
            
              client.print(F(" ok"));
              Serial.println(F(" ok"));
              if (strlen(command) > 3)
                prgCmd();
              else
                exeCmd();
              if (strcmp(command, "stm") == 0)
                client.println(getSystemDate());
              else if (strcmp(command, "rst") == 0) {
                eepromPutLng(prevNonce);
                for (int i = 10; i < 10*MAX_CMD; i++) {
                  EEPROM.write(i, 0);
                }
              }
              else if (strcmp(command, "lst") == 0) {
                Serial.println();
                client.println();
                for (int i = 10; i < 10*MAX_CMD; i += 10) {
                  if (EEPROM.read(i) > 0) {
                    for (int j = 0; (j < 3) && (EEPROM.read(i+j) > 0); j++) {
                      hmac[j] = (char)EEPROM.read(i+j);
                    }
                    hmac[3] = ' ';
                    int c = EEPROM.read(i+3);  // hour
                    if (c >= 10)
                      hmac[4] = c/10 + '0';
                    else
                      hmac[4] = '0';
                    hmac[5] = c%10 + '0';
                    hmac[6] = ':';
                    c = EEPROM.read(i+4);      // minute
                    if (c >= 10)
                      hmac[7] = c/10 + '0';
                    else
                      hmac[7] = '0';
                    hmac[8] = c%10 + '0';
                    hmac[9] = ' ';
                    c = EEPROM.read(i+5);      // dow
                    if (c >= 10)
                      hmac[10] = '#';
                    else
                      hmac[10] = '0';
                    hmac[11] = c%10 + '0';
                    hmac[12] = 0;
                    Serial.println(hmac);
                    client.println(hmac);
                  }
                }
              }
            }
            else
            {
              client.print(F(" nok"));
              if (strcmp(hmac, bin2hex(hash)) != 0)
              {
                client.print(F(", bad hash"));
                Serial.println("Bad hash:");
                Serial.println(hmac);
                Serial.println(bin2hex(hash));
              }
              if (eepromGetLng() >= lngNonce)
              {
                client.print(F(", bad nonce"));
                Serial.println("Bad nonce:");
                Serial.println(eepromGetLng());
                Serial.println(lngNonce);
              }
              client.println(F(""));
            }
          }
        }
      }
      // give the web browser time to receive the data
      Alarm.delay(1);
      // close the connection:
      client.stop();
      Serial.println("client disconnected");
    }
  }
  Alarm.delay(0);
}

bool prgCmd()
{
  //if (strcmp(command, "aglf2300") == 0) {
  if (command[0] == 'a') {
    int bl = 0;
    for (int i = 10; i < 10*MAX_CMD; i += 10) {
      if (EEPROM.read(i) == 0) {
        bl = i;
        break;
      }
    }
    if ((bl > 0) && (strlen(command) >= 8)) {
      for (int j = 0; j < 3; j++)
        EEPROM.write(bl+j, command[j+1]);
      int h = 10 * (command[4] - '0') + command[5] - '0';
      int m = 10 * (command[6] - '0') + command[7] - '0';
      EEPROM.write(bl+3, h);
      EEPROM.write(bl+4, m);
      if (command[8] > 0)
        EEPROM.write(bl+5, command[8] - '0');
      else
        EEPROM.write(bl+5, 255);
    }
    setAlarms();
  }
  else if (command[0] == 'd') {
    Serial.println("deleting");
    for (int i = 10; i < 10*MAX_CMD; i += 10) {
      bool match = true;
      //Serial.println(strlen(command));
      for (int j = 1; j < strlen(command); j++) {
        Serial.print(i+j);
        Serial.print(" - ");
        Serial.print(command[j]);
        Serial.print(" - ");
        Serial.print((char)EEPROM.read(i+j-1));
        Serial.print(" - ");
        Serial.print(command[j] != EEPROM.read(i+j-1));
        Serial.println();
        if (command[j] != EEPROM.read(i+j-1)) {
          match = false;
          break;
        }
      }
      if (match) {
        Serial.println("match");
        for (int j = 0; j < 10; j++)
          EEPROM.write(i+j, 0);
      }
    }
    setAlarms();
  }
}

void setAlarms() {
  for (int i = 0; i < dtNBR_ALARMS; i++) {
    Alarm.free(i);
  }
  for (int i = 10; i < 10*MAX_CMD; i += 10) {
    if (EEPROM.read(i) != 0) {
      int h = EEPROM.read(i+3);
      int m = EEPROM.read(i+4);
      timeDayOfWeek_t d = static_cast<timeDayOfWeek_t>(EEPROM.read(i+5));
      Serial.print(h);
      Serial.print(":");
      Serial.println(m);
      Serial.print("-");
      Serial.println(d);
      if (d == 255)
        Alarm.alarmRepeat(h, m, 0, cmdAlarm);
      else
        Alarm.alarmRepeat(d, h, m, 0, cmdAlarm);
    }
  }
  //Alarm.timerRepeat(12, onWDT);
  Alarm.alarmRepeat(2, 0, 0, chkTz);
  /*AlarmID_t a;
  a = Alarm.timerRepeat(12, onWDT);
  Serial.print(a);
  int h = 16;
  int m = 33;
  Serial.print(" Alarm at ");
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  a = Alarm.alarmRepeat(h, m, 10, cmdAlarm);
  Serial.print(" a=");
  Serial.print(a);
  Serial.print(" count()=");
  Serial.print(Alarm.count());
  Serial.println();*/
}

void cmdAlarm()
{
  Serial.println("cmdAlarm");
  /*command[0] = 'g';
  command[1] = 'l';
  command[2] = 'n';
  command[3] = 0;
  exeCmd();*/
  for (int i = 10; i < 10*MAX_CMD; i += 10) {
    if (EEPROM.read(i) != 0) {
      int h = EEPROM.read(i+3);
      int m = EEPROM.read(i+4);
      int d = EEPROM.read(i+5);
      if (d == 255 || d == weekday()) {
        if (h == hour() && m == minute()) {
          Serial.print("Got ");
          Serial.print(h);
          Serial.print(":");
          Serial.println(m);
          command[0] = (char)EEPROM.read(i+0);
          command[1] = (char)EEPROM.read(i+1);
          command[2] = (char)EEPROM.read(i+2);
          command[3] = 0;
          exeCmd();
        }
      }
    }
  }
}

bool exeCmd()
{
  Serial.print("exeCmd ");
  Serial.print(command);
  if (strcmp(command, "gln") == 0)
    setSwitch(GROUP_STR, 3, HIGH);
  else if (strcmp(command, "glf") == 0)
    setSwitch(GROUP_STR, 3, LOW);
  else if (strcmp(command, "dln") == 0)
    setSwitch(GROUP_STR, 4, HIGH);
  else if (strcmp(command, "dlf") == 0)
    setSwitch(GROUP_STR, 4, LOW);
  else if (strcmp(command, "gun") == 0)
    setSwitch(GROUP_OKN, 1, HIGH);
  else if (strcmp(command, "guf") == 0)
    setSwitch(GROUP_OKN, 1, LOW);
  else if (strcmp(command, "gdn") == 0)
    setSwitch(GROUP_OKN, 2, HIGH);
  else if (strcmp(command, "gdf") == 0)
    setSwitch(GROUP_OKN, 2, LOW);
  else if (strcmp(command, "dun") == 0)
    setSwitch(GROUP_OKN, 3, HIGH);
  else if (strcmp(command, "duf") == 0)
    setSwitch(GROUP_OKN, 3, LOW);
  else if (strcmp(command, "ddn") == 0)
    setSwitch(GROUP_OKN, 4, HIGH);
  else if (strcmp(command, "ddf") == 0)
    setSwitch(GROUP_OKN, 4, LOW);
  else if (strcmp(command, "tvn") == 0)
    setSwitch(GROUP_STR, 2, HIGH);
  else if (strcmp(command, "tvf") == 0)
    setSwitch(GROUP_STR, 2, LOW);
  else if (strcmp(command, "pcn") == 0)
    setSwitch(GROUP_STR, 1, HIGH);
  else if (strcmp(command, "pcf") == 0)
    setSwitch(GROUP_STR, 1, LOW);
  else if (strcmp(command, "pln") == 0)
    setSwitch(GROUP_PAR, 1, HIGH);
  else if (strcmp(command, "plf") == 0)
    setSwitch(GROUP_PAR, 1, LOW);
  else if (strcmp(command, "pkn") == 0)
    setSwitch(GROUP_PAR, 2, HIGH);
  else if (strcmp(command, "pkf") == 0)
    setSwitch(GROUP_PAR, 2, LOW);
}

void onWDT(){
  Serial.print("onWDT ");
  digitalClockDisplay();

  /*if (resetLevel < 0)
    resetLevel = 0;
  else {
    uint32_t ip = cc3000.IP2U32(192, 168, 1, 1);;
    Serial.print(F("WDT: pinging ... "));
    uint8_t replies = cc3000.ping(ip, 5);
    Serial.print(replies); Serial.println(F(" replies"));
    if (replies < 5) {
      if (resetLevel > 10) {
        Serial.print(F("\n\nResetting Arduino. The reset level is ")); Serial.println(resetLevel);
        resetFunc(); //call reset
      }
      else {
        Serial.print(F("\n\nClosing the connection. The reset level is ")); Serial.println(resetLevel);
        cc3000.disconnect();
        cc3000.reboot();
        serverStart();
        resetLevel++;
      }
    }
    else
      resetLevel = 0;
  }*/
}

void chkTz(){
  Serial.println("chkTz");
  digitalClockDisplay();
  testNtp();
  digitalClockDisplay();
}

String getSystemDate()
{
  String h = String(hour());
  if (h.length() < 2)
    h = "0" + h;
  String m = String(minute());
  if (m.length() < 2)
    m = "0" + m;
  String s = String(second());
  if (s.length() < 2)
    s = "0" + s;
  String y =  String(year());
  String mm = String(month());
  String d =  String(day());
  String dd = String(weekday());
  String tz = String(timeZone);

  return (" " + tz + " " + dd + " " + y + "-" + mm + "-" + d + " " + h + ":" + m + ":" + s);
}

// Return true if the buffer contains an HTTP request.  Also returns the request
// path and action strings if the request was parsed.  This does not attempt to
// parse any HTTP headers because there really isn't enough memory to process
// them all.
// HTTP request looks like:
//  [method] [path] [version] \r\n
//  Header_key_1: Header_value_1 \r\n
//  ...
//  Header_key_n: Header_value_n \r\n
//  \r\n

bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) 
{
  // Check if the request ends with \r\n to signal end of first line.
  if (bufSize < 2)
    return false;
  if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') {
    parseFirstLine((char*)buf, action, path);
    return true;
  }
  return false;
}

// Parse the action and path from the first line of an HTTP request.
void parseFirstLine(char* line, char* action, char* path) {
  // Parse first word up to whitespace as action.
  char* lineaction = strtok(line, " ");
  if (lineaction != NULL)
    strncpy(action, lineaction, MAX_ACTION);
  // Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, " ");
  if (linepath != NULL)
    strncpy(path, linepath, MAX_PATH);
}

bool parsePath(char* path, char* command, char* nonce, char* hmac) {
  // Parse first word up to whitespace as action.
  char* lineaction = strtok(path, "/");
  if (lineaction != NULL)
    strncpy(command, lineaction, MAX_ACTION);
  // Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, "/");
  if (linepath != NULL)
    strncpy(nonce, linepath, MAX_PATH);
  char* linehmac = strtok(NULL, "/");
  if (linehmac != NULL)
    strncpy(hmac, linehmac, MAX_PATH);
  else
    return false;
  return true;
}

void setSwitch(char* group, int pin_num, int pin_value)
{
  Serial.print("setSwitch ");
  Serial.print(group);
  Serial.print(pin_num);
  Serial.println(pin_value);
  switch (pin_num) {
  case 1:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_1ST);
    else
      mySwitch.switchOff(group, MY_1ST);
    break;
  case 2:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_2ND);
    else
      mySwitch.switchOff(group, MY_2ND);
    break;
  case 3:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_3RD);
    else
      mySwitch.switchOff(group, MY_3RD);
    break;
  case 4:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_4TH);
    else
      mySwitch.switchOff(group, MY_4TH);
    break;
  case 5:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_5TH);
    else
      mySwitch.switchOff(group, MY_5TH);
    break;
  case 6:
    if(pin_value == HIGH)
      mySwitch.switchOn(group, MY_6TH);
    else
      mySwitch.switchOff(group, MY_6TH);
    break;
  }
}

void resetFunc()
{
  Serial.print("Reset now ");
  wdt_enable(WDTO_15MS);
  while(1){
    Serial.print(".");
  }
}

void eepromPutLng(unsigned long nonce) {
  uint8_t* bAddr = (uint8_t*) &nonce;
  
  EEPROM.write( nonceAddr+0, *(bAddr+0) );
  EEPROM.write( nonceAddr+1, *(bAddr+1) );
  EEPROM.write( nonceAddr+2, *(bAddr+2) );
  EEPROM.write( nonceAddr+3, *(bAddr+3) );
}

long eepromGetLng() {
  long ret = EEPROM.read(nonceAddr);
  long mltp = 256;
  ret = ret + mltp * EEPROM.read(nonceAddr+1);
  mltp *= 256;
  ret = ret + mltp * EEPROM.read(nonceAddr+2);
  mltp *= 256;
  ret = ret + mltp * EEPROM.read(nonceAddr+3);
  return ret;
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void testNtp()
{
  Serial.println("testNtp");
  sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  Alarm.delay(1000);  
  Serial.println("testNtp 2");
  if ( Udp.parsePacket() ) {  
    Serial.println("packet parsed");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);               

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  
    // print Unix time:
    Serial.println(epoch);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':'); 
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch %60); // print the second
    timeZone = tZone(epoch);
    
    /*if (timeZone == 1)
      timeZone = 2;
    else
      timeZone = 1;*/
    
    epoch += timeZone * SECS_PER_HOUR;

    setTime(epoch);
  }
  else {
      Serial.println("packet not parsed");
      setTime(16, 33, 0, 22, 9, 2015);
  }
}

int tZone(time_t t)
{
  int tz = 1;
  int y = year(t);
  int m = month(t);
  int d = day(t);

  if (m > 3 && m < 10)
    tz = 2;
  else if (m == 3) {
    int b = 31 - ((((5 * y) / 4) + 4) % 7);
    if (d >= b)
      tz = 2;
  }
  else if (m == 10) {
    int e = 31 - ((((5 * y) / 4) + 1) % 7);
    if (d < e)
      tz = 2;
  }
  /*Serial.println("timeZone================");
  Serial.println(y);
  Serial.println(m);
  Serial.println(d);
  Serial.println(tz);
  Serial.println("timeZone================");*/
  return tz;
}

/*-------- NTP code ----------*/

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


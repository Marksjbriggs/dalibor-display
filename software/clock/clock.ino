
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <time.h>
#include "secrets.h"

#define GMT_TIME (-8)


time_t t_of_day;
time_t last_checked;

int status = WL_IDLE_STATUS;

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

bool stale = true;


#define CLOCK 8
#define DATA  9
#define LATCH 10


#define BUFFER_SIZE      96
#define DISPLAY_SIZE     6
#define NUMBER_OF_DIGITS 10

///////////////////////////////////////////////////////////////////////////
//
// THIS IS A BUFFER POINTER MAP TO ACCESS INDIVIDUAL DIGIT ELEMENT FLAGS
//
// INDEX INTO THIS BY DIGIT AND THEN BY ELEMENT
//
///////////////////////////////////////////////////////////////////////////

uint16_t DISPLAY_MAP[DISPLAY_SIZE][NUMBER_OF_DIGITS] = {
  // CARD 3 pointers
  { 69,  79,  77,  73,  78,  74,  70,  72,  76,  75},
  { 95,  86,  88,  93,  87,  91,  94,  92,  89,  90},
  
  // CARD 2 pointers
  { 37,  47,  45,  41,  46,  42,  38,  40,  44,  43}, // ML
  { 63,  54,  56,  61,  55,  59,  62,  60,  57,  58}, // MR
  
  // CARD 1 pointers
  {  5,  15,  13,   9,  14,  10,   6,   8,  12,  11}, // SL
  { 31,  22,  24,  29,  23,  27,  30,  28,  25,  26}, // SR
};

///////////////////////////////////////////////////////////////////////////
//
// THIS IS A BUFFER POLARITY MAP
//
// SOME REGISTERS HAVE INVERSE POLARITY
//
///////////////////////////////////////////////////////////////////////////

char BUFFER_POLARITY[] = {
  // card 1 (seconds)
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  
  // card 2 (minutes)
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 3 (hours)
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

///////////////////////////////////////////////////////////////////////////
//
// THIS IS THE DEFAULT DISPLAY BUFFER
//
///////////////////////////////////////////////////////////////////////////

char BUFFER[] = {
  // card 1
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 2
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  // card 3
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void setup() {
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(LATCH, OUTPUT);

  writeTime(99,99,99);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }

  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);

  while (RTC.STATUS > 0) {} // attend que tous les registre soit synchronisée

  RTC.PER = 1024*1; //1 secs timer.
  RTC.INTCTRL = 0 << RTC_CMP_bp
  | 1 << RTC_OVF_bp; //Overflow interrupt.
 
  RTC.CTRLA = RTC_PRESCALER_DIV1_gc //NO Prescaler
  | 1 << RTC_RTCEN_bp         //active RTC
  | 1 << RTC_RUNSTDBY_bp;     //fonctionne en standby

  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc; // 32KHz divisé par 32, fonctionne à 1.024kHz
  sei(); //active l'interruption
}

void clearMap() {
  unsigned idx;
  
  for (idx = 0; idx < BUFFER_SIZE; idx++) {
    BUFFER[idx] = BUFFER_POLARITY[idx]?1:0;
  }
}

void setMap(unsigned digit, unsigned num) { 
  unsigned idx;
  
  idx = DISPLAY_MAP[digit][num];
  BUFFER[idx] = BUFFER_POLARITY[idx]?0:1;
}

void pushMap() {
  unsigned idx;
  for (idx = 0; idx < BUFFER_SIZE; idx++) {
    digitalWrite(CLOCK, LOW);
    delayMicroseconds(10);
    digitalWrite(DATA, BUFFER[BUFFER_SIZE - idx]?HIGH:LOW);
    delayMicroseconds(10);
    digitalWrite(CLOCK, HIGH);
    delayMicroseconds(20);
  }
}

int p = 0;

void writeTime(unsigned h, unsigned m, unsigned s) {
  unsigned i;
    
  // deassert latch
  digitalWrite(LATCH, HIGH);
  delayMicroseconds(10);

  // clear display buffer
  clearMap();

  //  write digits to buffer
  setMap(0, h/10); // HL
  setMap(1, h%10); // HR
  setMap(2, m/10); // ML
  setMap(3, m%10); // MR
  setMap(4, s/10); // SL
  setMap(5, s%10); // SR

  // push buffer to display registers
  pushMap();

  // assert latch (copy registers to latch)
  digitalWrite(LATCH, LOW);
  delayMicroseconds(10);
}

void ntp_packet_check()
{
  struct tm t;
  
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
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
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    t.tm_year = 2019-1900;
    t.tm_mon = 1;
    t.tm_mday = 1;
    t.tm_hour = (epoch  % 86400L) / 3600 + GMT_TIME;
    t.tm_min = (epoch  % 3600) / 60;
    t.tm_sec = epoch % 60;
    t.tm_isdst = 0;        // Is DST on? 1 = yes, 0 = no, -1 = unknown

    t_of_day = mktime(&t);
    last_checked = t_of_day;

    stale = false;
  }
}


ISR(RTC_CNT_vect)
{
  RTC.INTFLAGS = RTC_OVF_bm;
  t_of_day++;
}



void loop() {
  struct tm* info;

  if( true == stale ) sendNTPpacket(timeServer);
  
  // see if there is an NTP UDP packet waiting for us
  ntp_packet_check();

  // write the time
  info = gmtime(&t_of_day);
  writeTime(info->tm_hour, info->tm_min, info->tm_sec);

  // if more than 15 minutes has gone by then mark time as stale
  if( t_of_day > (last_checked + 15 * 60) ) stale = true;

  // delay to prevent UDP packet flood
  delay(50);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

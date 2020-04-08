//
//  Texecom ESP WiFi Controller/Reader
//  MQTT commands and status
//
//  Author Trevor Steyn <trevor@webon.co.za>
//


#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

#define LED 2
#define ZONES 32

//Global Vars

// Configs
const char* wifiSsid        = "AP_NAME";
const char* wifiPassword    = "*************";
const char* mqttServer      = "10.0.0.1";
const char* mqttUser        = "mqtt_user";
const char* mqttPass        = "mqtt_pass";
const int mqttPort          = 1883;

IPAddress server(10, 0, 0, 1);

const char alarmPin[5] = "1234"; 

//Zone Vars

char ZONE_INFO_HEALTH[ZONES];     // Zone health
char C_ZONE_INFO_HEALTH[ZONES];   // Zone health compare for changes
char ZONE_STATUS[ZONES];          // Zone Status Clear or Motion etc
char C_ZONE_STATUS[ZONES];        // Zone Status compare for changes
char ZONE_BYPASS[ZONES];          // Zone Bypass Status
char C_ZONE_BYPASS[ZONES];        // Zone Bypass compare for changes
char ZONE_ABYPASS[ZONES];         // Zone AutBypass Status
char C_ZONE_ABYPASS[ZONES];       // Zone AutBypass Status
char ZONE_ALARMED[ZONES];         // Zone Alarmed
char C_ZONE_ALARMED[ZONES];       // Zone Alarmed

// Alarm Vars

char ALARM_DURESS[4];
char C_ALARM_DURESS[4];
char ALARM_BURGLAR[4];
char C_ALARM_BURGLAR[4];
char ALARM_ARMED[4];
char C_ALARM_ARMED[4];
char ALARM_ALARMED[4];
char C_ALARM_ALARMED[4];
char ALARM_RESET[4];
char C_ALARM_RESET[4];
char ALARM_READY[4];
char C_ALARM_READY[4];

// Alarm Commands

const byte ALARM_DURESS_COMMAND[1][4] = {
  {'\\', 'A', 0x01, '/'}
};

const byte ALARM_ARM_AWAY_COMMAND[4][4] = {
    {'\\', 'A', 0x01, '/'},
    {'\\', 'A', 0x02, '/'},
    {'\\', 'A', 0x04, '/'},
    {'\\', 'A', 0x08, '/'}
};

const byte ALARM_ARM_STAY_COMMAND[4][4] = {
    {'\\', 'Y', 0x01, '/'},
    {'\\', 'Y', 0x02, '/'},
    {'\\', 'Y', 0x04, '/'},
    {'\\', 'Y', 0x08, '/'}
};

const byte ALARM_RESET_COMMAND[4][4] = {
    {'\\', 'R', 0x01, '/'},
    {'\\', 'R', 0x02, '/'},
    {'\\', 'R', 0x04, '/'},
    {'\\', 'R', 0x08, '/'}
};

const byte ALARM_DISARM_COMMAND[4][4] = {
    {'\\', 'D', 0x01, '/'},
    {'\\', 'D', 0x02, '/'},
    {'\\', 'D', 0x04, '/'},
    {'\\', 'D', 0x08, '/'}
};

// ZONE Commands

const byte ZONE_BYPASS_COMMAND[32][4] = {
  {'\\', 'B', 0x01, '/'},
  {'\\', 'B', 0x02, '/'},
  {'\\', 'B', 0x03, '/'},
  {'\\', 'B', 0x04, '/'},
  {'\\', 'B', 0x05, '/'},
  {'\\', 'B', 0x06, '/'},
  {'\\', 'B', 0x07, '/'},
  {'\\', 'B', 0x08, '/'},
  {'\\', 'B', 0x09, '/'},
  {'\\', 'B', 0x0A, '/'},
  {'\\', 'B', 0x0B, '/'},
  {'\\', 'B', 0x0C, '/'},
  {'\\', 'B', 0x0D, '/'},
  {'\\', 'B', 0x0E, '/'},
  {'\\', 'B', 0x0F, '/'},
  {'\\', 'B', 0x10, '/'},
  {'\\', 'B', 0x11, '/'},
  {'\\', 'B', 0x12, '/'},
  {'\\', 'B', 0x13, '/'},
  {'\\', 'B', 0x14, '/'},
  {'\\', 'B', 0x15, '/'},
  {'\\', 'B', 0x16, '/'},
  {'\\', 'B', 0x17, '/'},
  {'\\', 'B', 0x18, '/'},
  {'\\', 'B', 0x19, '/'},
  {'\\', 'B', 0x1A, '/'},
  {'\\', 'B', 0x1B, '/'},
  {'\\', 'B', 0x1C, '/'},
  {'\\', 'B', 0x1D, '/'},
  {'\\', 'B', 0x1E, '/'},
  {'\\', 'B', 0x1F, '/'},
  {'\\', 'B', 0x20, '/'}  
};

const byte ZONE_UNBYPASS_COMMAND[32][4] = {
  {'\\', 'U', 0x01, '/'},
  {'\\', 'U', 0x02, '/'},
  {'\\', 'U', 0x03, '/'},
  {'\\', 'U', 0x04, '/'},
  {'\\', 'U', 0x05, '/'},
  {'\\', 'U', 0x06, '/'},
  {'\\', 'U', 0x07, '/'},
  {'\\', 'U', 0x08, '/'},
  {'\\', 'U', 0x09, '/'},
  {'\\', 'U', 0x0A, '/'},
  {'\\', 'U', 0x0B, '/'},
  {'\\', 'U', 0x0C, '/'},
  {'\\', 'U', 0x0D, '/'},
  {'\\', 'U', 0x0E, '/'},
  {'\\', 'U', 0x0F, '/'},
  {'\\', 'U', 0x10, '/'},
  {'\\', 'U', 0x11, '/'},
  {'\\', 'U', 0x12, '/'},
  {'\\', 'U', 0x13, '/'},
  {'\\', 'U', 0x14, '/'},
  {'\\', 'U', 0x15, '/'},
  {'\\', 'U', 0x16, '/'},
  {'\\', 'U', 0x17, '/'},
  {'\\', 'U', 0x18, '/'},
  {'\\', 'U', 0x19, '/'},
  {'\\', 'U', 0x1A, '/'},
  {'\\', 'U', 0x1B, '/'},
  {'\\', 'U', 0x1C, '/'},
  {'\\', 'U', 0x1D, '/'},
  {'\\', 'U', 0x1E, '/'},
  {'\\', 'U', 0x1F, '/'},
  {'\\', 'U', 0x20, '/'}
};

// Timer for pushing all data to mqtt

unsigned long timer;

// Get some modules ready
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  timer = millis();                     // initialise timer
  pinMode(LED, OUTPUT);                     // LED pin as output.
  digitalWrite(LED, HIGH);
  delay(2000);
  digitalWrite(LED, LOW);
  delay(2000);
  digitalWrite(LED, HIGH);
  //swSer.begin(19200);
  Serial.begin(19200,SERIAL_8N1,18,20);                      // Srial for Debugging
  Serial2.begin(19200,SERIAL_8N2);            // Initialize the Serial interface 
  Serial.println("Serial Initialised...");
  client.setServer(server, 1883);
  client.setCallback(callback);
  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(LED, LOW);
    delay(500);
        digitalWrite(LED, HIGH);
  }
  // Wifi Connected
    Serial.println("WIFI CONNECTE");
    Serial2.write("\\W1234/");       // Gain Access to Alarm Panel
    Serial2.readStringUntil('\n');     // Remove the OK from buffer
    Serial.println("Setup Loop completed");
}

// HA Send Commands as such
// /home/alarm/set/AA10 AA11 AW01 
// Z01B Z01U 

void callback(char* topic, byte* payload, unsigned int length) {
  char command_sent[length];
  strncpy(command_sent, (char*)payload, length);
  int INDEX = atoi(command_sent +2) -1;              // This is our partition or zone number
  
  if (payload[0] == 'A') {   // Alarm Panel Command Recieved
  
    if (payload[1] == 'A') {
  //    Serial2.write(ALARM_RESET_COMMAND[INDEX], sizeof(ALARM_RESET_COMMAND[INDEX]));
  //    Serial2.readStringUntil('\n');    
      if (C_ALARM_RESET[INDEX] == 'Y') {
        Serial2.write(ALARM_RESET_COMMAND[INDEX], sizeof(ALARM_RESET_COMMAND[INDEX]));
        Serial2.readStringUntil('\n');    
      }
      Serial2.write(ALARM_ARM_AWAY_COMMAND[INDEX], sizeof(ALARM_ARM_AWAY_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');
    } else if (payload[1] == 'S') {
   //   Serial2.write(ALARM_RESET_COMMAND[INDEX], sizeof(ALARM_RESET_COMMAND[INDEX]));
   //   Serial2.readStringUntil('\n');    
      if (C_ALARM_RESET[INDEX] == 'Y') {
        Serial2.write(ALARM_RESET_COMMAND[INDEX], sizeof(ALARM_RESET_COMMAND[INDEX]));
        Serial2.readStringUntil('\n');    
      }
      Serial2.write(ALARM_ARM_STAY_COMMAND[INDEX], sizeof(ALARM_ARM_STAY_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');
      
    } else if (payload[1] == 'D') {
      Serial2.write(ALARM_DISARM_COMMAND[INDEX], sizeof(ALARM_DISARM_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');
    } else if (payload[1] == 'R') {
      Serial2.write(ALARM_RESET_COMMAND[INDEX], sizeof(ALARM_RESET_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');      
      
    } else if (payload[1] == 'P') {
      //TODO GET CODE ETC
    }
    
  // Zone Code
  }else if (payload[0] == 'Z') {
    Serial.println("Zone Command Recived");
     if (payload[1] == 'B') {
      Serial2.write(ZONE_BYPASS_COMMAND[INDEX], sizeof(ZONE_BYPASS_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');     
     }else if (payload[1] == 'U') {
      Serial2.write(ZONE_UNBYPASS_COMMAND[INDEX], sizeof(ZONE_UNBYPASS_COMMAND[INDEX]));
      Serial2.readStringUntil('\n');
     }
  }
}


void check_alarm_panel(byte alarm_array[50],int array_size) {
  if (array_size != 20){
      Serial2.write("\\W1234/");           // Gain Access to Alarm Panel
      Serial2.readStringUntil('\n');       // Remove the OK from buffer
      Serial.print(array_size);
      Serial.println("Does not equal 20 for Alarm Info");
      return;
  }
  // Loop through 4 Partitions
  for ( int i = 0 ; i < 4; i++ ) {
    //  Duress first byte upper nibble
    if (bitRead(alarm_array[0], 0 + i) == 1) {
      ALARM_DURESS[i] = 'Y';    
    } else {
      ALARM_DURESS[i] = 'N';
    }
    if (ALARM_DURESS[i] != C_ALARM_DURESS[i]){
      C_ALARM_DURESS[i] = ALARM_DURESS[i];
      mqttPublish(i + 1, "AD", ALARM_DURESS[i]);
    }    
    // Burglar Alarm
    if (bitRead(alarm_array[1], 0 + i) == 1) {
      ALARM_BURGLAR[i] = 'Y';    
    } else {
      ALARM_BURGLAR[i] = 'N';
    }
    if (ALARM_BURGLAR[i] != C_ALARM_BURGLAR[i]){
      C_ALARM_BURGLAR[i] = ALARM_BURGLAR[i];
      mqttPublish(i + 1, "AB", ALARM_BURGLAR[i]);
    }
    // Alarm Armed/Armed Away
    if (bitRead(alarm_array[8], 4 + i) == 1) {
      ALARM_ARMED[i] = 'S';   // Stay Armed
    } else if (bitRead(alarm_array[8], 0 + i) == 1) {
      ALARM_ARMED[i] = 'A';   // Away Armed
    } else {
      ALARM_ARMED[i] = 'R';   // Ready
    }
    if (ALARM_ARMED[i] != C_ALARM_ARMED[i]){
      C_ALARM_ARMED[i] = ALARM_ARMED[i];
      mqttPublish(i + 1, "AA", ALARM_ARMED[i]);
    }

    // Confirmed Alarm
    if (bitRead(alarm_array[15], 0 + i) == 1) {
      ALARM_ALARMED[i] = 'Y';
    } else {
      ALARM_ALARMED[i] = 'N';
    }
    if (ALARM_ALARMED[i] != C_ALARM_ALARMED[i]){
      C_ALARM_ALARMED[i] = ALARM_ALARMED[i];
      mqttPublish(i + 1, "AC", ALARM_ALARMED[i]);
    }
    // Reset Required
    if (bitRead(alarm_array[14], 0 + i) == 1) {
      ALARM_RESET[i] = 'Y';
    } else {
      ALARM_RESET[i] = 'N';
    }
    if (ALARM_RESET[i] != C_ALARM_RESET[i]){
      C_ALARM_RESET[i] = ALARM_RESET[i];
      mqttPublish(i + 1, "AR", ALARM_RESET[i]);
    }
    if (bitRead(alarm_array[9], 0 + i) == 1) {
      ALARM_READY[i] = 'Y';   // Alarm Ready
    } else {
      ALARM_READY[i] = 'N';  
    }
    if (ALARM_READY[i] != C_ALARM_READY[i]){
      C_ALARM_READY[i] = ALARM_READY[i];
      mqttPublish(i + 1, "AL", ALARM_READY[i]);
    }
  }
}

void check_zone_info(byte zone_array[50],int array_size) {
    if (array_size - 1 != ZONES) {
      //Serial.println("Does not matvh zones");
      // Data Incorrect Lets reconnect with panel
      Serial2.write("\\W1234/");           // Gain Access to Alarm Panel
      Serial2.readStringUntil('\n');       // Remove the OK from buffer
      return;
    }
    for ( int i = 0 ; i <= ZONES-1; i++ ) {
    // This section checks the health of the Zones
      if ((bitRead(zone_array[i], 0) == 0) && (bitRead(zone_array[i], 1) == 0) ){
        ZONE_INFO_HEALTH[i] = 'H';
      } else if ((bitRead(zone_array[i], 0) == 1) && (bitRead(zone_array[i], 1) == 0) ){
          ZONE_INFO_HEALTH[i] = 'A';
      } else if ((bitRead(zone_array[i], 0) == 0) && (bitRead(zone_array[i], 1) == 1) ){
          ZONE_INFO_HEALTH[i] = 'T';
      }  else if ((bitRead(zone_array[i], 0) == 1) && (bitRead(zone_array[i], 1) == 1) ){
          ZONE_INFO_HEALTH[i] = 'S';
      }
      if (ZONE_INFO_HEALTH[i] != C_ZONE_INFO_HEALTH[i] ) {
          C_ZONE_INFO_HEALTH[i] = ZONE_INFO_HEALTH[i];
          mqttPublish(i + 1, "ZH", ZONE_INFO_HEALTH[i]); // TODO Lets change health to an int to save memory
      }

      // This section checks each zone for bypass
      //if ((bitRead(zone_array[i], 5) == 1) or (bitRead(zone_array[i], 6) == 1))  {
      if (bitRead(zone_array[i], 5) == 1)  {
          ZONE_BYPASS[i] = 'Y';
      } else {
        ZONE_BYPASS[i] = 'N';
      }
      if (ZONE_BYPASS[i] != C_ZONE_BYPASS[i]) {
        C_ZONE_BYPASS[i] = ZONE_BYPASS[i];
          mqttPublish(i + 1, "ZB", ZONE_BYPASS[i]);
      }

      // This section check each zone for auto bypass
      if (bitRead(zone_array[i], 6) == 1) {
          ZONE_ABYPASS[i] = 'Y';
      } else {
        ZONE_ABYPASS[i] = 'N';
      }
      if (ZONE_ABYPASS[i] != C_ZONE_ABYPASS[i]) {
          C_ZONE_ABYPASS[i] = ZONE_ABYPASS[i];
          mqttPublish(i + 1, "ZA", ZONE_ABYPASS[i]);
      }

      // This section checks if zone is alarmed
      if (bitRead(zone_array[i], 4) == 1) {
          ZONE_ALARMED[i] = 'Y';
      } else {
          ZONE_ALARMED[i] = 'N';
      }
      if (ZONE_ALARMED[i] != C_ZONE_ALARMED[i]) {
          C_ZONE_ALARMED[i] = ZONE_ALARMED[i];
          mqttPublish(i + 1, "ZZ", ZONE_ALARMED[i]);
      }
    }
}

void mqttPublish( int INDEX, char CMD[2], char STATE) {
  int ZONE_GG = INDEX;
  char STATUS[2] = {STATE};
  STATUS[1] = '\0';
  char BASE[] = "/home/alarm/";
  char TOPIC[50] = "/home/alarm/";
  int LENGTH = sizeof(BASE);
  char C_INDEX = INDEX + '0'; 
  String BIGZONES = String(INDEX);
  Serial.print("Change Discovered: Command:");
  Serial.print(CMD);
  Serial.print(" Zone/Partition: ");
  Serial.print(INDEX);
  Serial.print(" and STATE: ");
  Serial.println(STATE);  
  if (CMD[0] == 'A') {
    TOPIC[LENGTH-1] = 'A';
    if  (CMD[1] == 'D') {
      TOPIC[LENGTH] = C_INDEX;
      LENGTH = LENGTH + 1;
      char bypass[] = "/duress";
      int bypass_l = sizeof(bypass);
      Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      } 
      client.publish(TOPIC,STATUS,true);
      return;    
    } else if (CMD[1] == 'B') {
      TOPIC[LENGTH] = C_INDEX;
      LENGTH = LENGTH + 1;
      char bypass[] = "/burglar";
      int bypass_l = sizeof(bypass);
      Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      } 
      client.publish(TOPIC,STATUS,true);
      return;   
    } else if (CMD[1] == 'L') {
      TOPIC[LENGTH] = C_INDEX;
      LENGTH = LENGTH + 1;
      char topic[] = "/ready";
      int topic_l = sizeof(topic);
      int COUNT = LENGTH;
      for (int i=0; i < topic_l; i++) {
        COUNT = LENGTH + i;
        TOPIC[COUNT] = topic[i];
      }
      client.publish(TOPIC,STATUS,true);
      return;     
    } else if (CMD[1] == 'A') {
      TOPIC[LENGTH] = C_INDEX;
      LENGTH = LENGTH + 1;
      //char bypass[] = "/burglar";
      //int bypass_l = sizeof(bypass);
      //Serial.println(bypass_l);
      //int COUNT = LENGTH;
      //for (int i=0; i < bypass_l; i++) {
      //  COUNT = LENGTH + i;     
      //  TOPIC[COUNT] = bypass[i];
      //} 
      if ( STATUS[0] == 'A' ) {
        client.publish(TOPIC,"armed_away",true);
      } else if ( STATUS[0] == 'S' ) {
        client.publish(TOPIC,"armed_home",true);
      } else if ( STATUS[0] == 'R' ) {
        client.publish(TOPIC,"disarmed",true);
      }
      return;  
    }     
  }else if (CMD[0] == 'Z') {
    //Set Topic
    Serial.println("Getting into the zone hahaha");
    TOPIC[LENGTH-1] = 'Z';
     TOPIC[LENGTH] = BIGZONES[0];
     LENGTH = LENGTH + 1;
     if (INDEX > 9) {    
      TOPIC[LENGTH] = BIGZONES[1];
      LENGTH = LENGTH + 1;
      
     }
    Serial.println("Zone Update will be sent");
    
    if  (CMD[1] == 'B') {     
//      TOPIC[LENGTH] = C_INDEX;
//      LENGTH = LENGTH + 1;
      char bypass[] = "/bypass";
      int bypass_l = sizeof(bypass);
      Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      }
      client.publish(TOPIC,STATUS,true);
      return;
    }  else if (CMD[1] == 'A') {
//      TOPIC[LENGTH] = C_INDEX;
//      LENGTH = LENGTH + 1;
      char bypass[] = "/abypass";
      int bypass_l = sizeof(bypass);
      //Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      }
      client.publish(TOPIC,STATUS,true);
      return;
      
    } else if (CMD[1] == 'H') {
//      TOPIC[LENGTH] = C_INDEX;
//      LENGTH = LENGTH + 1;
      char bypass[] = "/health";
      int bypass_l = sizeof(bypass);
      //Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      }
      client.publish(TOPIC,STATUS,true);
      return;
    } else if (CMD[1] == 'Z') { 
//      TOPIC[LENGTH] = C_INDEX;
//      LENGTH = LENGTH + 1;
      char bypass[] = "/alarmed";
      int bypass_l = sizeof(bypass);
      //Serial.println(bypass_l);
      int COUNT = LENGTH;
      for (int i=0; i < bypass_l; i++) {
        COUNT = LENGTH + i;     
        TOPIC[COUNT] = bypass[i];
      }
      client.publish(TOPIC,STATUS,true);
      return;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "ESP8266TexecomClient-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (client.connect("texecom-alarm",mqttUser,mqttPass)) {
      //if (client.connect(clientId.c_str())) {
          Serial.println("connected");
          // Once connected, publish an announcement...
          client.publish("/home/alarm", "hello world");
          // ... and resubscribe
          client.subscribe("/home/alarm/set");
      } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
      }
    }
}
// the loop function runs over and over again forever
void loop() {  
  if (WiFi.status() == WL_CONNECTED) {
    // Reconnect to Mqtt if not connected
    if (!client.connected()) {
        reconnect();
    }
    digitalWrite(LED, LOW);
    client.loop();
    digitalWrite(LED, HIGH);
    // Resend all data every 5 minutes
    if (millis() - timer >= 300000) {
      
      memset(C_ZONE_INFO_HEALTH,'$', strlen(C_ZONE_INFO_HEALTH));
      memset(C_ZONE_STATUS,'$', strlen(C_ZONE_STATUS));
      memset(C_ZONE_BYPASS,'$', strlen(C_ZONE_BYPASS));
      memset(C_ZONE_ALARMED,'$', strlen(C_ZONE_ALARMED));
      memset(C_ALARM_DURESS,'$', strlen(C_ALARM_DURESS));
      memset(C_ALARM_BURGLAR,'$', strlen(C_ALARM_BURGLAR));
      memset(C_ALARM_ARMED,'$', strlen(C_ALARM_ARMED));
      memset(C_ALARM_ALARMED,'$', strlen(C_ALARM_ALARMED));
      memset(C_ALARM_RESET,'$', strlen(C_ALARM_RESET));  
      memset(C_ALARM_READY,'$', strlen(C_ALARM_READY));  
      timer = millis();   
    }
    // Get Zone status
    Serial2.write("\\Z");
    Serial2.write(0x00);
    Serial2.write(0x20);
    Serial2.write("/");
    byte zone_binary[50];  
    int ZoneSize = Serial2.readBytesUntil('\n',zone_binary, 50);
    check_zone_info(zone_binary,ZoneSize);

    client.loop();
      
    // Check Alarm Status
    Serial2.write("\\P");
    Serial2.write(0x00);
    Serial2.write(0x13);
    Serial2.write("/");
    byte alarm_binary[50];
    int AlarmSize = Serial2.readBytesUntil('\n',alarm_binary, 50);
    check_alarm_panel(alarm_binary, AlarmSize);
  
    // mqtt get commands etc
    } else {
      Serial.println("Wifi Failed");
    }
}

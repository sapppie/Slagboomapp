// Include files.
#include <SPI.h>                  // Ethernet shield uses SPI-interface
#include <Ethernet.h>             // Ethernet library (use Ethernet2.h for new ethernet shield v2)
#include <Servo.h>
Servo Slagboom;
// Set Ethernet Shield MAC address  (check yours)
byte mac[] = { 0x40, 0x6c, 0x8f, 0x36, 0x84, 0x8a }; // Ethernet adapter shield S. Oosterhaven
IPAddress ip(192, 168, 0, 106);
int ethPort = 3300;                                  // Take a free port (check your router)

EthernetServer server(ethPort);              // EthernetServer instance (listening on port <ethPort>).

//byte actionDevice = 0;                    // Variable to store Action Device id (0, 1, 2)
bool pinState[3] = {false, false, false}; // Variable to store actual on/off state
bool pinChange = false;                   // Variable to store actual pin change
int sensorValue0 = 0;                    // Variable to store actual sensor0 value
int sensorValue1 = 0;                    // Variable to store actual sensor1 value
const int trig = 4;
const int echo = 3;
int Boomopen = 500;
int Boomdicht = 0;
long duration;
int distance, sum;

void setup()
{
   Slagboom.attach(2);
   Serial.begin(9600);
   pinMode(trig, OUTPUT);
   pinMode(echo, INPUT);
   Slagboom.writeMicroseconds(0);
   Ethernet.begin(mac, ip);
   Serial.println("Ethernetboard connected (pins 10, 11, 12, 13 and SPI)");
   server.begin();

   // Print IP-address and led indication of server state
   Serial.print("Listening address: ");
   Serial.print(Ethernet.localIP());
   
   // for hardware debug: LED indication of server state: blinking = waiting for connection
   int IPnr = getIPComputerNumber(Ethernet.localIP());   // Get computernumber in local network 192.168.1.3 -> 3)
   Serial.print(" ["); Serial.print(IPnr); Serial.print("] "); 
   Serial.print("  [Testcase: telnet "); Serial.print(Ethernet.localIP()); Serial.print(" "); Serial.print(ethPort); Serial.println("]");
   //signalNumber(ledPin, IPnr);
}

void slagBoom(long duration,int distance)
{
  
int avgrange = 4;

for (int i =  1; i < avgrange; i++){
    digitalWrite(trig,LOW);
delayMicroseconds(2);

digitalWrite(trig,HIGH);
delayMicroseconds(10);
digitalWrite(trig,LOW);

duration = pulseIn(echo,HIGH);
distance = duration*0.034/2;
    sum += distance;
    delay(10);
                                   }
  int gemiddelde = sum / 4;
  Serial.print("Distance ");
Serial.println(gemiddelde);

sum = 0;
if (gemiddelde <= 10)
{
  
  Slagboom.writeMicroseconds(0);  
}
else
{
  Slagboom.writeMicroseconds(1500);
  
}
}

void Afstand(long duration, int distance)
{
         digitalWrite(trig,LOW);
         delayMicroseconds(2);

         digitalWrite(trig,HIGH);
         delayMicroseconds(10);
         digitalWrite(trig,LOW);

         duration = pulseIn(echo,HIGH);
         distance = duration*0.034/2;
}

void loop()
{
  slagBoom(duration,distance);
   EthernetClient ethernetClient = server.available();
   if (!ethernetClient) {
      return; // wait for connection and blink LED
   }

   Serial.println("Application connected");
   while (ethernetClient.connected()) 
   {
     while (ethernetClient.available())
      {
         char inByte = ethernetClient.read();   // Get byte from the client.
         executeCommand(inByte);  // Wait for command to execute
         inByte = NULL;                         // Reset the read byte.
      } 
   }
   Serial.println("Application disonnected");
}

// Choose and switch your Kaku device, state is true/false (HIGH/LOW)
void switchDefault(byte actionDevice, bool state)
{   
   pinState[actionDevice] = state;    
   if(state){server.write(" ON\n");}
   else{server.write("OFF\n");}
}

// Implementation of (simple) protocol between app and Arduino
// Request (from app) is single char ('a', 's', 't', 'i' etc.)
// Response (to app) is 4 chars  (not all commands demand a response)
void executeCommand(char cmd)
{     
         char buf[4] = {'\0', '\0', '\0', '\0'};

         // Command protocol
         Serial.print("["); Serial.print(cmd); Serial.print("] -> ");
         switch (cmd) {
         case 'a': // Report sensor value to the app  
         digitalWrite(trig,LOW);
         delayMicroseconds(2);

         digitalWrite(trig,HIGH);
         delayMicroseconds(10);
         digitalWrite(trig,LOW);

         duration = pulseIn(echo,HIGH);
         distance = duration*0.034/2;
         sensorValue0 = distance;                          // update sensor0 value
         intToCharBuf(sensorValue0, buf, 4);               // convert to charbuffer
         server.write(buf, 4);                             // response is always 4 chars (\n included)
         Serial.print("Sensor0: "); Serial.println(sensorValue0);
            
         break;
         
         case 'b': // Report sensor value to the app  
            sensorValue1 = readSensor(1, 100);                // update sensor1 value
            intToCharBuf(sensorValue1, buf, 4);               // convert to charbuffer
            server.write(buf, 4);                             // response is always 4 chars (\n included)
            Serial.print("Sensor1: "); Serial.println(buf);
            digitalWrite(trig,LOW);
         delayMicroseconds(2);

         digitalWrite(trig,HIGH);
         delayMicroseconds(10);
         digitalWrite(trig,LOW);

         duration = pulseIn(echo,HIGH);
         distance = duration*0.034/2;
            if(distance <= 10){
            switchDefault(0,true); Serial.println("Set 0 state to \"ON\"");Slagboom.writeMicroseconds(1500);
            }
            break;
         /*case 's': // Report switch state to the app
            if (pinState[actionDevice]) { server.write(" ON\n"); Serial.println("Pin state is ON"); }  // always send 4 chars
            else { server.write("OFF\n"); Serial.println("Pin state is OFF"); }
            break;*/
         case 'i':    
            break;
            
         case 'x': //toggle device 0
            if (pinState[0]) { switchDefault(0,false); Serial.println("Set 0 state to \"CLOSED\""); Slagboom.writeMicroseconds(1500); }
            else { switchDefault(0,true); Serial.println("Set 0 state to \"OPEN\""); Slagboom.writeMicroseconds(0);}             
            break;
            
         case 'y': //toggle device 1
            if (pinState[1]) { switchDefault(1,false); Serial.println("Set 1 state to \"OFF\"");}
            else { switchDefault(1,true); Serial.println("Set 1 state to \"ON\"");}             
            break;
            
         case 'z': //toggle device 2
            if (pinState[2]) { switchDefault(2,false); Serial.println("Set 2 state to \"OFF\"");}
            else { switchDefault(2,true); Serial.println("Set 2 state to \"ON\"");}              
            break;
            
         }
}
void distanceAv(int trig,int echo,long duration,int distance){
  int avgrange = 6;
  for (int i =  1; i < avgrange; i++){
    Afstand(duration,distance);
    

sum += distance;
delay(10);
  }
  int gemiddelde = sum / 6;
  Serial.print("Distance ");
Serial.println(gemiddelde);

sum = 0;
}
// read value from pin pn, return value is mapped between 0 and mx-1
int readSensor(byte pn, int mx)
{
  return map(analogRead(pn), 0, 1023, 0, mx);    
}

// Convert int <val> char buffer with length <len>
void intToCharBuf(int val, char buf[], byte len)
{
   String s;
   s = String(val);                        // convert tot string
   if (s.length() == 1) s = "0" + s;       // prefix redundant "0" 
   if (s.length() == 2) s = "0" + s;  
   s = s + "\n";                           // add newline
   s.toCharArray(buf, len);                // convert string to char-buffer
}

// Check switch level and determine if an event has happend
// event: low -> high or high -> low
void checkEvent(byte p, bool &state)
{
   static bool swLevel = false;       // Variable to store the switch level (Low or High)
   static bool prevswLevel = false;   // Variable to store the previous switch level

   swLevel = digitalRead(p);
   if (swLevel)
      if (prevswLevel) delay(1);
      else {               
         prevswLevel = true;   // Low -> High transition
         state = true;
         pinChange = true;
      } 
   else // swLevel == Low
      if (!prevswLevel) delay(1);
      else {
         prevswLevel = false;  // High -> Low transition
         state = false;
         pinChange = true;
      }
}

// blink led on pin <pn>
void blink(byte pn)
{
  digitalWrite(pn, HIGH); 
  delay(100); 
  digitalWrite(pn, LOW); 
  delay(100);
}

// Visual feedback on pin, based on IP number, used for debug only
// Blink ledpin for a short burst, then blink N times, where N is (related to) IP-number
/*void signalNumber(byte pin, byte n)
{
   byte i;
   for (i = 0; i < 30; i++)
       { digitalWrite(pin, HIGH); delay(20); digitalWrite(pin, LOW); delay(20); }
   delay(1000);
   for (i = 0; i < n; i++)
       { digitalWrite(pin, HIGH); delay(300); digitalWrite(pin, LOW); delay(300); }
    delay(1000);
}*/

// Convert IPAddress tot String (e.g. "192.168.1.105")
String IPAddressToString(IPAddress address)
{
    return String(address[0]) + "." + 
           String(address[1]) + "." + 
           String(address[2]) + "." + 
           String(address[3]);
}

// Returns B-class network-id: 192.168.1.3 -> 1)
/*int getIPClassB(IPAddress address)
{
    return address[2];
}*/

// Returns computernumber in local network: 192.168.1.3 -> 3)
int getIPComputerNumber(IPAddress address)
{
    return address[3];
}

// Returns computernumber in local network: 192.168.1.105 -> 5)
/*int getIPComputerNumberOffset(IPAddress address, int offset)
{
    return getIPComputerNumber(address) - offset;
}*/


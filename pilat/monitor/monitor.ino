#include <Wire.h>

/* I2C master must use this address to communicate
 * Connect the Arduino SCL pin directly to RPi GPIO 3
 * and SDA pin to RPi GPIO 2. No voltage divider is required.
 */
int SLAVE_ADDR = 0x4;

/* hlen is the number of bins in the histogram.
 * The micros() clock has a resolution of 4us,
 * and use this value as the bin width. Using
 * 400 bins allows to span 1600us total. Outliers
 * increments the last bin.
 */
const int hlen = 400;
int hist[hlen];
unsigned long lat_max = 0;

// connected pins
int pin_trg = 13; // connect to RPi GPIO 23 (with voltage divider 10k/30k)
int pin_ack = 3;  // connect to RPi GPIO 24 (with pull-down 10k)

// internal program state
int running = 0;
int do_dump = 0;
int pos = 0;

/* I2C Commands */
char CMD_START  = 0x42; // start interrupt signal
char CMD_STOP   = 0x43; // stop interrupt signal
char CMD_RESET  = 0x44; // reset histogram
char CMD_DUMP   = 0x45; // display report on the serial console

void resetHist()
{
  for (int i = 0; i < hlen; i++) {
    hist[i] = 0;
  }  
}

void setup()
{
  Serial.begin(9600);
  resetHist();
  
  Wire.begin(SLAVE_ADDR);
  Wire.onRequest(reqHist);
  Wire.onReceive(handleCommand);
  
  pinMode(pin_trg, OUTPUT);
  pinMode(pin_ack, INPUT);
}

void loop()
{
  int val;
  int retry = 0;
  
  if (running) {
    
    // set trigger signal
    unsigned long t1 = micros();
    digitalWrite(pin_trg, HIGH);
    
    // wait for ack
    while(digitalRead(pin_ack) == 0) {
      continue;
    }
  
    // compute response time
    unsigned long t2 = micros();  
    
    // unset trigger pin
    digitalWrite(pin_trg, LOW);
  
    unsigned long us = (t2 - t1);
    unsigned long bin = us / 4; // 4us per bin (micros() resolution)
    if (bin >= hlen) {
      bin = hlen - 1;
    }
    hist[bin]++;

    if (us > lat_max) {
      lat_max = us;
    }
    Serial.print("sample:");
    Serial.println(us);
    Serial.flush();
  }
  
  if (do_dump) {
    do_dump = 0;
    for (int i = 0; i < hlen; i++) {
      Serial.print(hist[i]);
      if (i < (hlen - 1)) {
        Serial.print(", ");
      } else {
        Serial.println("");
      }
    }
    Serial.print("latency max:")
    Serial.println(lat_max);
    Serial.println("done");
    Serial.flush();
  }
  
  delay(100);
}

void handleCommand(int count)
{
  char cmd = 0;
  while(Wire.available()) {
    cmd = Wire.read();
    if (cmd == CMD_START) {
      Serial.println("start");
      running = 1;
    } else if (cmd == CMD_STOP) {
      Serial.println("stop");
      running = 0;      
    } else if (cmd == CMD_RESET) {
      Serial.println("reset");
      pos = 0;
      resetHist();
    } else if (cmd == CMD_DUMP) {
      do_dump = 1;
      Serial.println("dump");
    }
  }
}

/* The histogram cannot be sent all at once with I2C.
 * Bins are sent one at a time. This is why we need to 
 * request the exact amount of data that the histogram contains.
 */
void reqHist()
{
  if (running)
    return;
  Wire.write((uint8_t *)&hist[pos], sizeof(int));
  pos = (pos + 1) % hlen;
}


/*William Garrido
 * - 06-28-2015  Added Serial JSON input from ESP8266 with MQTT
 * - 
 */
#include "U8glib.h"

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <TimerOne.h>
#include <ArduinoJson.h>

#define TMP_PIN A7
#define PIN 6
#define NUMPIXELS 16
#define PIR 3

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
U8GLIB_SSD1306_128X64 u8g(4, 7, 5);		// HW SPI Com: CS = 10, A0 = 9 (Hardware Pins are  SCK = 13 and MOSI = 11)

unsigned long timer = millis();
long motionTime = 20000;
bool motion = false;

// Serial input buffer
#define INPUT_BUFFER_SIZE 100
char input_Buffer[INPUT_BUFFER_SIZE];
uint8_t input_Buffer_Index;

// Multiple screen support
uint8_t current_screen = 0;
#define MAX_SCREENS  6;
#define SCREENTIME 1000

void drawColorBox(void)
{
  u8g_uint_t w,h;
  u8g_uint_t r, g, b;
  
  w = u8g.getWidth()/32;
  h = u8g.getHeight()/8;
  for( b = 0; b < 4; b++ )
    for( g = 0; g < 8; g++ )
      for( r = 0; r < 8; r++ )
      {
        u8g.setColorIndex((r<<5) |  (g<<2) | b );
        u8g.drawBox(g*w + b*w*8, r*h, w, h);
      }
}

void drawLogo(uint8_t d)
{
  u8g.setFont(u8g_font_gdr25r);
  u8g.drawStr(0+d, 30+d, "T");
  u8g.drawStr(23+d,30+d,"E");
  u8g.drawStr(43+d,30+d,"M");
  u8g.drawStr(73+d,30+d,"P");

  
  u8g.drawHLine(2+d, 35+d, 47);
  u8g.drawVLine(45+d, 32+d, 12);
}

void drawURL(void)
{
  u8g.setFont(u8g_font_4x6);
  if ( u8g.getHeight() < 59 )
  {
    u8g.drawStr(53,9,"code.google.com");
    u8g.drawStr(77,18,"/p/u8glib");
  }
  else
  {
    u8g.drawStr(1,54,"code.google.com/p/u8glib");
  }
}


void draw(void) {
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    drawColorBox();
  }
  u8g.setColorIndex(1);
  if ( U8G_MODE_GET_BITS_PER_PIXEL(u8g.getMode()) > 1 ) {
    drawLogo(2);
    u8g.setColorIndex(2);
    drawLogo(1);
    u8g.setColorIndex(3);
  }
  drawLogo(0);
  //drawURL();
  
}

void checkPIR(){
  
  motion = true;
  timer = millis();
  
}

void setup(void) {
  // flip screen, if required
  //u8g.setRot180();
  Serial.begin(115200);
  Serial1.begin(115200);
  strip.begin();
  pinMode(PIR, INPUT);
  attachInterrupt(1, checkPIR, HIGH);
    
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(strip.Color(10, 0, 0), 20); // Red
  colorWipe(strip.Color(0, 10, 0), 20); // Green
  colorWipe(strip.Color(0, 0, 10), 20); // Blue

  Timer1.initialize(SCREENTIME); // 100ms reading interval
  Timer1.attachInterrupt(switchMode); 
}

void switchMode() {
  current_screen = (current_screen + 1) % MAX_SCREENS;
}

void loop(void) {
  
  // picture loop
  u8g.firstPage();  
  do {
    draw();
    u8g.setPrintPos(1,64);
    u8g.print(getTemp());
    u8g.print("°F");
  u8g.setColorIndex(1);

  switch (current_screen) {
      case 0:

        break;
    }    
    
  } while( u8g.nextPage() );
  
  // rebuild the picture after some delay
  //delay(50);  
  if(motion) setNeo(getTemp());
  else colorWipe(strip.Color(0, 0, 0), 0);
  
  
  if(timer + motionTime <= millis()) motion = false;

  // Check if we have serial input
  while (Serial1.available()) {
    input_Buffer[input_Buffer_Index] = Serial1.read();
    if (input_Buffer[input_Buffer_Index] == '\n') {
      input_Buffer[input_Buffer_Index] = '\0';
      processInput();
      input_Buffer_Index = 0;
    } else 
      input_Buffer_Index = (input_Buffer_Index+1)%INPUT_BUFFER_SIZE;
  }
  
  //Serial.println("Heartbeat");
}


void processInput() {

  if (input_Buffer[4] != ':')
    return;

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input_Buffer);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  const char* sensorL = root["L"];
  const char* sensorV = root["V"];
  const char* sensorD = root["D"];

  Serial.println(sensorL);
  Serial.println(sensorV);
  Serial.println(sensorD);

  switch (sensorL) {
    case 'L':


      break;

    case 'V':


      break;

    case 'D':


      break;
    
  }
}

    
void setNeo(double tmp){
 int mTmp;
 
 if(tmp > 59 && tmp <= 72){mTmp = mapf(tmp, 59, 72, 40, 1); colorWipe(strip.Color(0, 0, mTmp), 10); // Blue
   }
 else if(tmp >= 71 && tmp < 85){mTmp = mapf(tmp, 71, 85, 1, 40); colorWipe(strip.Color(mTmp, 0, 0), 10); // Red
   }
  
  
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

float toF(float degreesC){
  return 1.8 * degreesC + 32.0;
}

static int readVcc (byte us =250){
  analogRead(6);
  bitSet(ADMUX, 3);
  delayMicroseconds(us);
  bitSet(ADCSRA, ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  word x = ADC;
  return x ? (1100L * 1023) / x : -1; 
}


double getTemp(){
  const int NUMSAMPLES = 5;
   int RawTemp = 0;
    for (int i = 0; i < NUMSAMPLES; i++)
    {
      RawTemp += analogRead(TMP_PIN);
    }
    RawTemp /= NUMSAMPLES;
    long Resistance;	
    double Temp;
    Resistance=((10240000/RawTemp) - 10000); 
    Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
    Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp)); 
    Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
    //Temp = (Temp * -1);
    Temp = toF(Temp); // Convert Celcius to Fahrenheit
      //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
    return Temp;

}

//Copy of Arduino map function converted for floats, from a forum post
float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#include <EEPROM.h>
#include <Espalexa.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN D3       // LED Pin
#define NUMPIXELS 16 // Number of LED's in your strip
#define WICK_PIN PIN
#define CANDLEPIXELS 6

// The LED can be in only one of these states at any given time
#define ESPALEXA_A 1SYNC
#define BRIGHT 0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// START OF CANDLE MODE RELATED STUFF ////////////////////////////////////////////////
Adafruit_NeoPixel *wick;
byte state;
byte index_start;
byte index_end;

long red_int = 0;
long green_int = 0;
long blue_int = 255;
int brightness = 255;

// Change this!!
const char *ssid = "YOUR WIFI NAME";                // Enter WiFi Name
const char *password = "YOUR WIFI PASSWORD"; // Enter WiFi Password

Espalexa espalexa;

void handleSwitchOff();
void handleSwitchOn();

// callback function from esp8266 alexa
void handleToggleLight(uint8_t brightness, uint32_t rgb);

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  // TODO: WIFI function start from here
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  EEPROM.begin(5); // set up EEPROM storage space for config values

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // WIFI function close here

  // TODO: Read last value from memort while bootup

  // EEPROM Memory //
  // Mem Location ||--0--||--1--||--2--||--3--||--4--||--5--||--6--||
  //                 red   green  blue   bright  mode

  red_int = EEPROM.read(0); // restore colour to last used value. Ensures RGB lamp is same colour as when last switched off
  green_int = EEPROM.read(1);
  blue_int = EEPROM.read(2);
  brightness = EEPROM.read(3);

  strip.begin();

  wick = new Adafruit_NeoPixel(CANDLEPIXELS, WICK_PIN, NEO_GRB + NEO_KHZ800);
  wick->begin();
  wick->show();

  index_start = 255;
  index_end = 255;
  state = BRIGHT;

  if (WiFi.status() == WL_CONNECTED)
  {
    espalexa.addDevice("NightLamp", handleToggleLight); // Device Name
    espalexa.begin();
  }
  else
  {
    while (1)
    {
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }
}

void loop()
{
  espalexa.loop();
  delay(1);
}

// the color device callback function has two parameters
void handleToggleLight(uint8_t brightness, uint32_t rgb)
{

  // TODO: Read the brightness level and rgb value then set the value to EEPROM and switch on the light
  float hell = brightness / 255.0;
  red_int = ((rgb >> 16) & 0xFF) * hell;
  green_int = ((rgb >> 8) & 0xFF) * hell;
  blue_int = (rgb & 0xFF) * hell;

  EEPROM.write(0, red_int); // write the colour values to EEPROM to be restored on start-up
  EEPROM.write(1, green_int);
  EEPROM.write(2, blue_int);
  EEPROM.commit();

  if (brightness == 0)
  {
    handleSwitchOff();
  }
  else
  {
    handleSwitchOn();
  }
}

// trun on the lights
void handleSwitchOn()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    strip.setPixelColor(i, strip.Color(brightness * red_int / 255, brightness * green_int / 255, brightness * blue_int / 255)); // Set colour with gamma correction and brightness adjust value.
    strip.show();
  }
}

void handleSwitchOff()
{ // go to non-existent mode and turn off all pixels
  for (int i = 0; i < NUMPIXELS; i++)
  {                                               // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off led strip
    strip.show();
  } // This sends the updated pixel color to the hardware.
}

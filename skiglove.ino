#include <U8glib.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

/* NEOPIXEL */
#define NEOPIN          6
#define NUMPIXELS      16

U8GLIB_SSD1306_128X64 ui(U8G_I2C_OPT_NONE);	// I2C / TWI
SoftwareSerial gpsSerial(5, 4);
Adafruit_GPS GPS(&gpsSerial);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);
uint8_t satLEDs = 0;



uint8_t satLight(uint8_t oldSats, uint8_t newSats) {
  Serial.print(oldSats);Serial.print(":");Serial.println(newSats);
  if (oldSats < newSats) {
    for (int i= 15 - oldSats; i > 15 - newSats; i--) {
      Serial.println(i);
      pixels.setPixelColor(i, pixels.Color(0,15,0));
      pixels.show();
      delay(100);
    }
  }
  else if (oldSats > newSats) {
    for (int i=15 - oldSats; i <= 15 - newSats; i++) {
      pixels.setPixelColor(i, 0);
      pixels.show();
      Serial.print(i);
      delay(100);
    }
  }
  return newSats;
}

void draw() {
  static char str[20];
  // graphic commands to redraw the complete screen should be placed here  
  ui.setFont(u8g_font_unifont);
  //ui.setFont(u8g_font_osb21);
  ui.setPrintPos(0, 10); 
  ui.print("Sats: ");
  ui.print(GPS.satellites);
  ui.setPrintPos(0, 10+ ui.getFontLineSpacing());
  ui.print("Got fix? ");
  ui.print(GPS.fix); 
  ui.setPrintPos(0, 23 + ui.getFontLineSpacing());
  ui.print("Quality: ");
  ui.print(GPS.fixquality);
  sprintf(str, "Date: %d/%d/%d", GPS.day, GPS.month, GPS.year);
  ui.setPrintPos(0, 36 + ui.getFontLineSpacing());
  ui.print(str);
  sprintf(str, "Time: %d:%d:%d", GPS.hour, GPS.minute, GPS.seconds);
  ui.setPrintPos(0, 50 + ui.getFontLineSpacing());
  ui.print(str);
}

void timerIsr() {
  (void)GPS.read();
}

void setup(void) {
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  Timer1.initialize(10000);
  Timer1.attachInterrupt(timerIsr); 
  // assign default color value
  if ( ui.getMode() == U8G_MODE_R3G3B2 ) {
    ui.setColorIndex(255);     // white
  }
  else if ( ui.getMode() == U8G_MODE_GRAY2BIT ) {
    ui.setColorIndex(3);         // max intensity
  }
  else if ( ui.getMode() == U8G_MODE_BW ) {
    ui.setColorIndex(1);         // pixel on
  }
  else if ( ui.getMode() == U8G_MODE_HICOLOR ) {
    ui.setHiColorByRGB(255,255,255);
  }
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.println("SkiGlove Initialised!");
}

void loop(void) {
  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
    Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    }
  // picture loop
  ui.firstPage();  
  do {
    draw();
  } while( ui.nextPage() );
  // rebuild the picture after some delay
  satLEDs = satLight(satLEDs, GPS.satellites);
}


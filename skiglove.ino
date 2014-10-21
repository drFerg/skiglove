#include <U8glib.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <TimerOne.h>
#include <Adafruit_NeoPixel.h>

/* NEOPIXEL */
#define NEOPIN          6
#define NUMPIXELS      16
#define REFRESH 500
#define TICKS 4
#define SCREEN_LINES 5
#define LINE_CHARS 20
U8GLIB_SSD1306_128X64 ui(U8G_I2C_OPT_NONE);	// I2C / TWI
SoftwareSerial gpsSerial(5, 4);
Adafruit_GPS GPS(&gpsSerial);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);
uint8_t satLEDs = 0;
uint8_t tick = 0;
uint32_t elapsed = 0;
char ticks[TICKS] = {'\\' , '|', '/', '-'};
char text[SCREEN_LINES][LINE_CHARS];



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
  ui.setPrintPos(0, 12); 
  ui.print(text[0]);
  ui.setPrintPos(0, 10 + ui.getFontLineSpacing());
  ui.print(text[1]);
  ui.setPrintPos(0, 23 + ui.getFontLineSpacing());
  ui.print(text[2]);
  ui.setPrintPos(0, 36 + ui.getFontLineSpacing());
  ui.print(text[3]);
  ui.setPrintPos(0, 50 + ui.getFontLineSpacing());
  ui.print(text[4]);
}

void setup(void) {
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_BAUD_9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_1HZ);
  GPS.sendCommand(PGCMD_NOANTENNA);

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

char str[32];
void loop(void) {
  /*GPS Update phase */
  GPS.read();
  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
    sprintf(str, "LL: %ld.%ld%c|%ld.%ld%c", GPS.latitude_fixed / 10000000, GPS.lat_min /1000,
            GPS.lat, GPS.longitude_fixed / 10000000, GPS.long_min/1000, GPS.lon);
    Serial.println(str);
    }
  /* Draw phase (refresh rate ~2HZ)*/
  if (millis() - elapsed >= REFRESH){
    sprintf(text[0], (GPS.fix ? "Sats:[%d]| Q:%d %c" : 
                          "Sats: %d | Q:%d %c"), GPS.satellites, GPS.fixquality, ticks[tick]);
    sprintf(text[1], "Lat : %ld.%ld%c", GPS.latitude_fixed / 10000000, GPS.lat_min/1000, GPS.lat);
    sprintf(text[2], "Long: %ld.%ld%c", GPS.longitude_fixed / 10000000, GPS.long_min/ 1000, GPS.lon);
    sprintf(text[3], "Date: %d/%d/%d", GPS.day, GPS.month, GPS.year);
    sprintf(text[4], "Time: %d:%d:%d", GPS.hour, GPS.minute, GPS.seconds);
    ui.firstPage();  
    do {
      draw();
    } while( ui.nextPage());
    tick = (tick + 1) % TICKS;
    elapsed = millis();
  }
  /* LED Update Phase */
  satLEDs = satLight(satLEDs, GPS.satellites);
}


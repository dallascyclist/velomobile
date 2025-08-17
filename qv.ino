
//  This is the arduno code for operating the lights for a Mango Velomobile,  it should be adaptable to any other velomobile or bicycle.
//  I am using an Arduino DUE for the current processor, mainly because I wanted a lot of pins to experiment with over time, and the
//  energy draw just isn't all that much compared to the draw from the LEDs themselves
//
//  This code is Copyright  2025 Doug Davis of Murphy Texas dougd@dldavis.com  under the terms of the GPL V3 licnese as cited below
//

/*   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// If you need help, you can reach me on the email above or on BROL as MrWizard


#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 3
Adafruit_SSD1306 display(OLED_RESET);

#ifdef __AVR__
#include <avr/power.h>
#endif
#include <SPI.h>
#define NUM_PIXELS  200


#define DUE_LEFT_STRIP_PIN 2
#define DUE_RIGHT_STRIP_PIN 3

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel left_strip = Adafruit_NeoPixel(NUM_PIXELS, DUE_LEFT_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel right_strip = Adafruit_NeoPixel(NUM_PIXELS, DUE_RIGHT_STRIP_PIN, NEO_GRBW + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
uint16_t left_red_pos = 0;
uint16_t left_blue_pos = 0;
uint16_t left_green_pos = 0;
uint16_t left_purple_pos = 0;
uint16_t left_yellow_pos = 0;
uint16_t left_white_pos = 0;


uint16_t right_red_pos = 0;
uint16_t right_blue_pos = 0;
uint16_t right_green_pos = 0;
uint16_t right_purple_pos = 0;
uint16_t right_yellow_pos = 0;
uint16_t right_white_pos = 0;



uint8_t fire_latch = 0;

uint8_t theater_latch = 0;
uint8_t theater_index = 0;

uint8_t warp_latch = 0;
uint8_t warp_index = 0;

void setup() {
  uint16_t left_spacing = left_strip.numPixels() / 6;
  uint16_t right_spacing = right_strip.numPixels() / 6;

  Serial.begin(9600);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)]
  #ifdef DISPLAY
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  #endif // DISPLAY

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  #ifdef DISPLAY
  display.display();
  #endif // DISPLAY
  
  delay(2000); // 2 seconds settle time

  // Clear the buffer.
  #ifdef DISPLAY
  display.clearDisplay();
  #endif // DISPLAY

  left_strip.begin();
  right_strip.begin();
  left_strip.show(); // Initialize all pixels to 'off'
  right_strip.show(); // Initialize all pixels to 'off'

  left_red_pos   = left_spacing * 1;
  left_blue_pos  = left_spacing * 2;
  left_green_pos = left_spacing * 3;
  left_purple_pos = left_spacing * 4;
  left_yellow_pos = left_spacing * 5;
  left_white_pos = left_spacing * 6;

  right_red_pos   = right_spacing * 1;
  right_blue_pos  = right_spacing * 2;
  right_green_pos = right_spacing * 3;
  right_purple_pos = right_spacing * 4;
  right_yellow_pos = right_spacing * 5;
  right_white_pos = right_spacing * 6;
}

// modified for RGBW

#define P_RED     0xff0000
#define P_BLUE    0x0000ff
#define P_GREEN   0x00ff00
#define P_YELLOW  0xffff00
#define P_PURPLE  0x00ffff
#define P_WHITE   0xffffffff
#define P_OFF     0x000000


#define READ_LIMIT  200 // threashold of a pin going high to trigger an event

#define LEFT_PIN  1
#define STOP_PIN  2
#define RIGHT_PIN 3
#define NIGHT_PIN 4
#define DAY_PIN   5


#define THEATER_PIN 0
#define OFF_PIN 6
#define LIGHTNING_PIN 9
#define FIRE_PIN 8
#define WARP_PIN 7

#define STOP      (uint8_t) 1
#define NO_STOP   (uint8_t) 0

#define IDLE_OFF  0
#define IDLE_WARP 1
#define IDLE_FIRE 2
#define IDLE_THEATER 3
#define IDLE_DAY  10

#define FLASH_DELAY 4

#define MIN_GENERATION 2
#define MAX_GENERATIONS 8
#define RIPPLE_FRAMES 8         // Number of animation frames per ripple
#define RIPPLE_INTERVAL 0      // Time between frames (ms)
#define RIPPLE_LENGTH 10      // how far the ripple expands on each side



uint8_t day_generation = MIN_GENERATION;
uint8_t ripple_frame = 0;
uint8_t day_latch = 0; 
uint32_t lastRippleTime = 0;
bool vehicle_controls = true;
uint8_t off_latch = 0;



int first_time = 0;
int32_t flash_delay             = 0;
int32_t flash_delay_day         = 0;
int32_t flash_delay_left        = 0;
int32_t flash_delay_left_stop   = 0;
int32_t flash_delay_right       = 0;
int32_t flash_delay_right_stop  = 0;
int32_t flash_delay_night       = 0;
int32_t flash_delay_stop        = 0;
int32_t flash_delay_hazards     = 0;



#define THEATERS 4 // hard code this for now, build an array funcitons later 

uint32_t warparr[][6] = {
  { P_OFF, P_OFF, P_OFF, P_OFF, P_OFF, P_OFF, },
  { P_RED, P_WHITE, P_RED, P_WHITE, P_RED, P_WHITE, },
  { P_RED, P_RED, P_RED, P_RED, P_RED, P_RED, },
  { P_RED, P_YELLOW, P_RED, P_YELLOW, P_RED, P_YELLOW, },
  { P_RED, P_RED, P_YELLOW, P_RED, P_RED, P_YELLOW, },
  { P_RED, P_RED, P_YELLOW, P_RED, P_RED, P_RED, },
  { P_RED, P_RED, P_BLUE, P_BLUE, P_RED, P_RED, },
  { P_GREEN, P_GREEN, P_GREEN, P_GREEN, P_GREEN, P_GREEN, },
  { P_YELLOW, P_YELLOW, P_YELLOW, P_YELLOW, P_YELLOW, P_YELLOW, },
  { P_BLUE, P_BLUE, P_BLUE, P_BLUE, P_BLUE, P_BLUE, },
  { P_WHITE, P_WHITE, P_WHITE, P_WHITE, P_WHITE, P_WHITE, },
  { P_RED, P_WHITE, P_RED, P_WHITE, P_RED, P_WHITE, },
  { P_RED, P_GREEN, P_BLUE, P_PURPLE, P_YELLOW, P_WHITE, },

  { P_OFF, P_OFF, P_OFF, P_OFF, P_OFF, P_OFF, },
};

uint32_t fires[][3] = {
  { 0, 0, 0 },
  { 255, 215, 40 },  // Yellow FIre
  { 215, 255, 40 },
  { 40, 215, 255 },
  { 40, 255, 215 },
  { 255, 40, 215 },
  { 215, 40, 255 },
  { 0, 0, 0 },
};
uint8_t fire_index = 0;

uint32_t idle_mode = IDLE_OFF;
//#define DEBUG ON 
#ifdef DEBUG
  int D;
#endif
// #define DEBUG ON
void loop() {
#ifdef DEBUG
  if (D++ % 100 == 0) {
  Serial.print("first_time: ");
  Serial.print(first_time, DEC);
  Serial.print(" idle: ");
  Serial.print(idle_mode, DEC);
  Serial.print(" flash_delay_day: ");
  Serial.print(flash_delay_day, DEC);
  Serial.print(" flash_delay_left: ");
  Serial.print(flash_delay_left, DEC);
  Serial.print(" flash_delay_right: ");
  Serial.print(flash_delay_right, DEC);
  Serial.print(" flash_delay_stop: ");
  Serial.print(flash_delay_stop, DEC);

  Serial.print(" flash_delay_left_stop: ");
  Serial.print(flash_delay_left_stop, DEC);
  Serial.print(" flash_delay_right_stop: ");
  Serial.print(flash_delay_right_stop, DEC);
  Serial.print(" flash_delay_hazard: ");
  Serial.print(flash_delay_hazards, DEC);
  Serial.print(" Off: ");
  Serial.print(analogRead(OFF_PIN), DEC);
  Serial.print(" STOP: ");
  Serial.print(analogRead(STOP_PIN), DEC);
  Serial.print(" Left: ");
  Serial.print(analogRead(LEFT_PIN), DEC);
  Serial.print(" Right: ");
  Serial.print(analogRead(RIGHT_PIN), DEC);
  Serial.print(" Night: ");
  Serial.print(analogRead(NIGHT_PIN), DEC);
  Serial.print(" Day: ");
  Serial.print(analogRead(DAY_PIN), DEC);
  Serial.print(" Warp: ");
  Serial.print(analogRead(WARP_PIN), DEC);
  Serial.print(" Thea: ");
  Serial.print(analogRead(THEATER_PIN), DEC);
  Serial.print(" Light: ");
  Serial.print(analogRead(LIGHTNING_PIN), DEC);

  Serial.print("\r\n");

  }
#endif
if (analogRead(DAY_PIN) > READ_LIMIT) {
  if (day_latch == 0) {
    day_generation++;
    if (day_generation > MAX_GENERATIONS) day_generation = MIN_GENERATION;
    ripple_frame = 0;
    day_latch = 1;
  }
  idle_mode = IDLE_DAY;
} else {
  day_latch = 0;
}




  if (analogRead(FIRE_PIN) > READ_LIMIT) {
    if (fire_latch == 0) {
      fire_index++;
      fire_latch++;
    }
    idle_mode = IDLE_FIRE;
    if (fires[fire_index][0] == 0)     fire_index = 0;
    #ifdef DISPLAY

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.println("FIRE: ");
    display.println(fire_index, DEC);
    display.display();
    #endif // DISPLAY




  } else {
    fire_latch = 0;

  }
  if (analogRead(WARP_PIN) > READ_LIMIT) {

    if (warp_latch == 0) {
      warp_index++;
      warp_latch++;

      if (warparr[warp_index][1] == (uint32_t) P_OFF) warp_index = 0; // set it back to the beginning of the sequence list
    }
    idle_mode = IDLE_WARP;
    #ifdef DISPLAY
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.println("WARP: ");
    display.println(warp_index, DEC);
    display.display();
    #endif 
  } else {
    warp_latch = 0;
  }
if (analogRead(OFF_PIN) > READ_LIMIT) {
  idle_mode = IDLE_OFF;
  if (off_latch == 0) {
    vehicle_controls = !vehicle_controls; // toggle between true/false
    off_latch = 1; // lock until released
    #ifdef DISPLAY
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(10, 0);
      display.clearDisplay();
      if (vehicle_controls) {
        display.println("Controls ON");
      } else {
        display.println("Controls OFF");
      }
      display.display();
    #endif
  }
} else {
  off_latch = 0; // allow re-trigger
}

  if (analogRead(THEATER_PIN) > READ_LIMIT) {
    if (theater_latch == 0) {
      theater_index++;
      theater_latch++;
      if (theater_index == THEATERS) theater_index = 0 ;
    }
    first_time = 0;
    idle_mode = IDLE_THEATER;
    #ifdef DISPLAY
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.println("THEATER MODE :" );
    display.println(theater_index, DEC);
    display.display();
    #endif



  } else  {
    theater_latch = 0;
  }

  // if (flash_delay_right_stop < 0) flash_delay_right_stop = 0 ;
  // if (flash_delay_left_stop < 0 ) flash_delay_left_stop = 0; 
  if (vehicle_controls) { 
  if ((analogRead(LEFT_PIN) > READ_LIMIT  && analogRead(RIGHT_PIN) > READ_LIMIT ) ||   flash_delay_hazards  != 0) {
    first_time = 0;

    if (flash_delay_hazards == 0) oledDisplay((char *) "HAZARD");
    if (analogRead(LEFT_PIN) > READ_LIMIT && analogRead(RIGHT_PIN) > READ_LIMIT )
      flash_delay_hazards = FLASH_DELAY;
    else
      flash_delay_hazards -- ;


    return  Hazard();

  }
  if ((analogRead(LEFT_PIN) > READ_LIMIT  && analogRead(STOP_PIN) > READ_LIMIT ) ||   flash_delay_left_stop  != 0) {
    first_time = 0;
    if (flash_delay_left_stop == 0)  oledDisplay((char *) "LEFT + STOP");
    if (analogRead(STOP_PIN) > READ_LIMIT )
      flash_delay_left_stop = FLASH_DELAY;
    else
      flash_delay_left_stop -- ;


    return Left_Turn(STOP);

  }


  if ((analogRead(RIGHT_PIN) > READ_LIMIT && analogRead(STOP_PIN) > READ_LIMIT ) || flash_delay_right_stop != 0 )  {
    first_time = 0;
    if (flash_delay_right_stop == 0) oledDisplay ((char *) "RIGHT + STOP");
    if (analogRead(STOP_PIN) > READ_LIMIT)
      flash_delay_right_stop = FLASH_DELAY;
    else
      flash_delay_right_stop -- ;

    return Right_Turn(STOP);

  }
  if (analogRead(LEFT_PIN) > READ_LIMIT  ||  flash_delay_left != 0) {
    first_time = 0;
    flash_delay_left_stop = 0;  // protection racket
    if (flash_delay_left == 0) oledDisplay((char *) "LEFT");
    if (analogRead(LEFT_PIN) > READ_LIMIT)
      flash_delay_left = FLASH_DELAY;
    else
      flash_delay_left -- ;



    return Left_Turn(NO_STOP);

  }

  if (analogRead(RIGHT_PIN) > READ_LIMIT || flash_delay_right != 0 )  {
    first_time = 0;
    flash_delay_right_stop = 0;  // protection racket
    if (flash_delay_right == 0) oledDisplay ((char *) "RIGHT");

    if (analogRead(RIGHT_PIN) > READ_LIMIT)
      flash_delay_right = FLASH_DELAY;
    else
      flash_delay_right -- ;

    return Right_Turn(NO_STOP);

  }
  if (analogRead(STOP_PIN) > READ_LIMIT || flash_delay_stop != 0 ) {
    first_time = 0;
    if (flash_delay_stop == 0)   oledDisplay((char *) "STOP");


    if (analogRead(STOP_PIN) > READ_LIMIT)
      flash_delay_stop = FLASH_DELAY;
    else
      flash_delay_stop -- ;

    return Stop();

  }
  } // vehicle_controls
  clearDelays();

  switch (idle_mode)  {

case IDLE_DAY: {
  playDayRippleEffect(day_generation);
  break;
}



    case IDLE_OFF: {
        clearStrip(&left_strip);
        clearStrip(&right_strip);

        theater_index - 0;
        fire_index = 0;
        warp_index = 0;
        clearDelays();

        if (first_time == 0) {

        #ifdef DISPLAY
          display.stopscroll();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(10, 0);
          display.clearDisplay();
          display.println("Doug Davis's Mango");
          display.println("   +1 972-900-2902");
          display.println("  dougd@dldavis.com");
          display.display();
        #endif

          first_time = 1;

        }
        break ;
      }
    case IDLE_WARP: {
        first_time = 1;
        left_strip.setPixelColor(left_red_pos, warparr[warp_index][0]);
        left_strip.setPixelColor(left_blue_pos, warparr[warp_index][1]);
        left_strip.setPixelColor(left_green_pos, warparr[warp_index][2]);
        left_strip.setPixelColor(left_purple_pos, warparr[warp_index][3]);
        left_strip.setPixelColor(left_yellow_pos, warparr[warp_index][4]);
        left_strip.setPixelColor(left_white_pos, warparr[warp_index][5]);
        left_strip.show();

        // take care of the clean up for next loop
        left_strip.setPixelColor(left_red_pos, P_OFF);
        left_strip.setPixelColor(left_blue_pos, P_OFF);
        left_strip.setPixelColor(left_green_pos, P_OFF);
        left_strip.setPixelColor(left_purple_pos, P_OFF);
        left_strip.setPixelColor(left_yellow_pos, P_OFF);;
        left_strip.setPixelColor(left_white_pos, P_OFF);

        if (left_red_pos--    <= 1) left_red_pos      = left_strip.numPixels();
        if (left_blue_pos--   <= 1) left_blue_pos     = left_strip.numPixels();
        if (left_green_pos--  <= 1) left_green_pos    = left_strip.numPixels();
        if (left_purple_pos-- <= 1) left_purple_pos   = left_strip.numPixels();
        if (left_yellow_pos-- <= 1) left_yellow_pos   = left_strip.numPixels();
        if (left_white_pos--  <= 1) left_white_pos    = left_strip.numPixels();


        right_strip.setPixelColor(right_red_pos, warparr[warp_index][0]);
        right_strip.setPixelColor(right_blue_pos, warparr[warp_index][1]);
        right_strip.setPixelColor(right_green_pos, warparr[warp_index][2]);
        right_strip.setPixelColor(right_purple_pos, warparr[warp_index][3]);
        right_strip.setPixelColor(right_yellow_pos, warparr[warp_index][4]);
        right_strip.setPixelColor(right_white_pos, warparr[warp_index][5]);

        right_strip.show();

        // take care of the clean up for next loop
        right_strip.setPixelColor(right_red_pos, P_OFF);
        right_strip.setPixelColor(right_blue_pos, P_OFF);
        right_strip.setPixelColor(right_green_pos, P_OFF);
        right_strip.setPixelColor(right_purple_pos, P_OFF);
        right_strip.setPixelColor(right_yellow_pos, P_OFF);;
        right_strip.setPixelColor(right_white_pos, P_OFF);

        if (right_red_pos--    <= 1) right_red_pos      = right_strip.numPixels();
        if (right_blue_pos--   <= 1) right_blue_pos     = right_strip.numPixels();
        if (right_green_pos--  <= 1) right_green_pos    = right_strip.numPixels();
        if (right_purple_pos-- <= 1) right_purple_pos   = right_strip.numPixels();
        if (right_yellow_pos-- <= 1) right_yellow_pos   = right_strip.numPixels();
        if (right_white_pos--  <= 1) right_white_pos    = right_strip.numPixels();
        break;
      }
    case IDLE_FIRE : {
        int r = fires[fire_index][0];
        int g = fires[fire_index][1];
        int b = fires[fire_index][2];

        for (int x = 8; x < 240; x++)
        {
          int flicker = random(0, 200);
          int r1 = r - flicker;
          int g1 = g - flicker;
          int b1 = b - flicker;
          if (g1 < 0) g1 = 0;
          if (r1 < 0) r1 = 0;
          if (b1 < 0) b1 = 0;
          left_strip.setPixelColor(x, r1, g1, b1);
          right_strip.setPixelColor(x, r1, g1, b1);
        }
        left_strip.show();
        right_strip.show();
        delay(random(50, 150));
        break;

      }
    case IDLE_THEATER: {
        switch (theater_index) {
          case 0: {
              amber_blink(250);
              break;
            }
          case 1: {
              red_amber_blink(100);
              break;
            }
          case 2: {
              red_green_blink(100);
              break;
            }
          case 3: {
              red_green_random(100);
              break;
            }
        }

        break;
      }

  }

}

void Stop()
{
  uint32_t amber = left_strip.Color(127, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);
  uint16_t offset = left_strip.numPixels() / 2 + left_strip.numPixels() / 4;
  // blink(&left_strip, amber, 0, offset, offset*2, 0);
  // blink(&right_strip, amber, 0, offset, offset*2, 0);
  solid(&left_strip, red, 0, left_strip.numPixels(), 0);
  solid(&right_strip, red, 0, right_strip.numPixels(), 0);
  //  solid(&left_strip, red, offset*2, left_strip.numPixels()/2, 0);
  // solid(&right_strip, red, offset*2, left_strip.numPixels()/2, 0);
}

void Hazard()
{

  uint16_t i = 0, j = 0;
  uint32_t amber = left_strip.Color(127, 127, 0);

  blink((Adafruit_NeoPixel *) &right_strip, amber,  0, 0, right_strip.numPixels(), right_strip.numPixels());

  blink((Adafruit_NeoPixel *) &left_strip, amber,  0, 0, left_strip.numPixels(), left_strip.numPixels());
  return ;
}
void Right_Turn(uint8_t stop)
{
  uint16_t i = 0, j = 0;
  uint32_t amber = left_strip.Color(127, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);


  if (stop == STOP)
    solid((Adafruit_NeoPixel *) &left_strip, red, 0, left_strip.numPixels(), 0);
  else
    clearStrip(&left_strip);

  blink((Adafruit_NeoPixel *) &right_strip, amber,  0, 0, right_strip.numPixels(), 240);

}

void Left_Turn(uint8_t stop)
{
  uint16_t i = 0, j = 0;
  uint32_t amber = left_strip.Color(127, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);

  if (stop == STOP)
    solid((Adafruit_NeoPixel *) &right_strip, red, 0, right_strip.numPixels(), 0);
  else
    clearStrip((Adafruit_NeoPixel *) &right_strip);

  blink((Adafruit_NeoPixel *) &left_strip, amber,  0, 0, left_strip.numPixels(), 240);

}


void blink(Adafruit_NeoPixel *strip, uint32_t first_color, uint32_t second_color, uint16_t start_pixel, uint16_t end_pixel, uint8_t wait)
{
  uint16_t i = 0;
  for (i = start_pixel; i < ( end_pixel < strip->numPixels() ?  end_pixel : strip->numPixels()); i++)
    strip->setPixelColor(i, first_color);
  strip->show();
  delay(wait);
  for (i = start_pixel; i < ( end_pixel < strip->numPixels() ?  end_pixel : strip->numPixels()); i++)
    strip->setPixelColor(i, second_color);
  strip->show();
  delay(wait);
}


void solid(Adafruit_NeoPixel *strip, uint32_t solid_color,  uint16_t start_pixel, uint16_t end_pixel, uint8_t wait)
{
  uint16_t i = 0;
  for (i = start_pixel; i < ( end_pixel < strip->numPixels() ?  end_pixel : strip->numPixels()); i++)
    strip->setPixelColor(i, solid_color);
  strip->show();
  delay(wait);
}

void amber_blink(int8_t wait)
{
  uint16_t i = 0, j = 0;
  uint32_t amber = left_strip.Color(200, 200, 0);

  theaterChaseX2(amber,  amber, 0, left_strip.numPixels() / 2, wait);

}

void red_amber_blink(int8_t wait)
{
  uint16_t i = 0, j = 0;
  uint32_t amber = left_strip.Color(127, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);
  theaterChaseX2(red,  amber, 0, left_strip.numPixels() / 2, wait);

}


void red_green_blink(int8_t wait)
{
  uint16_t i = 0, j = 0;
  uint32_t green = left_strip.Color(0, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);
  theaterChaseX2(red,  green, 0, left_strip.numPixels() / 2, wait);

}


void red_green_random(int8_t wait)
{
  uint16_t i = 0, j = 0;
  uint32_t green = left_strip.Color(0, 127, 0);
  uint32_t red = left_strip.Color(127, 0, 0);
  theaterChaseXXYY(red,  green, 0, left_strip.numPixels() / 2, wait);

}

//Theatre-style crawling lights.
void theaterChaseX2(uint32_t front, uint32_t rear, int x, int y, uint8_t wait) {
  for (int j = 0; j < 2; j++) { //do 10 cycles of chasing
    for (int q = 3; q > 0; q--) {
      for (int i = y; i > x; i = i - 3) {
        left_strip.setPixelColor(i + q, front);  //turn every third pixel on
        right_strip.setPixelColor(i + q, front);  //turn every third pixel on
        left_strip.setPixelColor(i + q + y, rear); //turn every third pixel on
        right_strip.setPixelColor(i + q + y, rear); //turn every third pixel on
      }
      left_strip.show();
      right_strip.show();

      delay(wait);

      for (int i = y; i > x; i = i - 3) {
        left_strip.setPixelColor(i + q, 0);      //turn every third pixel off
        right_strip.setPixelColor(i + q, 0);      //turn every third pixel off
        left_strip.setPixelColor(i + q + y, 0);    //turn every third pixel off
        right_strip.setPixelColor(i + q + y, 0);    //turn every third pixel off
      }
    }
  }
}
//Theatre-style crawling lights.
void theaterChaseXXYY(uint32_t front, uint32_t rear, int x, int y, uint8_t wait) {
  for (int j = 0; j < 2; j++) { //do 10 cycles of chasing
    for (int q = 3; q > 0; q--) {
      for (int i = y; i > x; i = i - 3) {
        int c = random (0, 2) == 1 ? front : rear ;
        left_strip.setPixelColor(i + q, c);  //turn every third pixel on
        right_strip.setPixelColor(i + q, c);  //turn every third pixel on
        left_strip.setPixelColor(i + q + y, c); //turn every third pixel on
        right_strip.setPixelColor(i + q + y, c); //turn every third pixel on
      }
      left_strip.show();
      right_strip.show();

      delay(wait);

      for (int i = y; i > x; i = i - 3) {
        left_strip.setPixelColor(i + q, 0);      //turn every third pixel off
        right_strip.setPixelColor(i + q, 0);      //turn every third pixel off
        left_strip.setPixelColor(i + q + y, 0);    //turn every third pixel off
        right_strip.setPixelColor(i + q + y, 0);    //turn every third pixel off
      }
    }
  }
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < left_strip.numPixels(); i = i + 3) {
        left_strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
        right_strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      left_strip.show();
      right_strip.show();

      delay(wait);

      for (int i = 0; i < left_strip.numPixels(); i = i + 3) {
        left_strip.setPixelColor(i + q, 0);      //turn every third pixel off
        right_strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

uint32_t dimColor(uint32_t color, uint8_t width) {
  return (((color & 0xFF0000) / width) & 0xFF0000) + (((color & 0x00FF00) / width) & 0x00FF00) + (((color & 0x0000FF) / width) & 0x0000FF);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {

    return left_strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;

    return left_strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return left_strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void clearStrip(Adafruit_NeoPixel *strip) {
  for ( int i = 0; i < strip->numPixels(); i++) {
    strip->setPixelColor(i, 0x000000);
  }
  strip->show();
}
void oledDisplay (char *disp)
{
  #ifdef DISPLAY
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.clearDisplay();
  display.println(disp);
  display.display();
  display.startscrollleft(0x00, 0x0F);
  #endif
}

void clearDelays()
{
  flash_delay             = 0;
  flash_delay_day         = 0;
  flash_delay_left        = 0;
  flash_delay_left_stop   = 0;
  flash_delay_right       = 0;
  flash_delay_right_stop  = 0;
  flash_delay_night       = 0;
  flash_delay_stop        = 0;
  flash_delay_hazards     = 0;
  return ;
}


#define MAX_TRAIL_LENGTH 5   // Red falls off here


void playDayRippleEffect(uint8_t generation) {
  uint16_t numPixels = left_strip.numPixels();
  int ripple_zone = numPixels / generation;
  int ripple_max_offset = ripple_zone / 2;

  // Expand/collapse animation phase
  int cycle_offset = ripple_frame;
  if (ripple_frame > ripple_max_offset) {
    cycle_offset = (ripple_max_offset * 2) - ripple_frame;
  }

  clearStrip(&left_strip);
  clearStrip(&right_strip);

  for (uint8_t g = 0; g < generation; g++) {
    int zone_start = g * ripple_zone;
    int zone_end   = zone_start + ripple_zone;
    int center     = zone_start + ripple_zone / 2;

    // Draw center pulse at beginning and end of ripple cycle
    if (ripple_frame == 0 || ripple_frame == ripple_max_offset * 2) {
      left_strip.setPixelColor(center, left_strip.Color(255, 255, 255)); // white

      if (center - 1 >= zone_start) {
        left_strip.setPixelColor(center - 1, left_strip.Color(255, 180, 0)); // amber
      }
      if (center + 1 < zone_end) {
        left_strip.setPixelColor(center + 1, left_strip.Color(255, 180, 0)); // amber
      }

      right_strip.setPixelColor(center, right_strip.Color(255, 255, 255)); // white

      if (center - 1 >= zone_start) {
        right_strip.setPixelColor(center - 1, right_strip.Color(255, 180, 0)); // amber
      }
      if (center + 1 < zone_end) {
        right_strip.setPixelColor(center + 1, right_strip.Color(255, 180, 0)); // amber
      }

      continue; // skip trail drawing this frame
    }

    // Standard trailing ripple logic
    for (int offset = 0; offset <= cycle_offset; offset++) {
      int trailIndex = cycle_offset - offset;
      if (trailIndex >= MAX_TRAIL_LENGTH) continue;

      uint32_t color = getTrailColor(trailIndex);

      int leftPos  = center - offset;
      int rightPos = center + offset;

      if (leftPos >= zone_start && leftPos < zone_end) {
        left_strip.setPixelColor(leftPos, color);
        right_strip.setPixelColor(leftPos, color);
      }
      if (rightPos >= zone_start && rightPos < zone_end) {
        left_strip.setPixelColor(rightPos, color);
        right_strip.setPixelColor(rightPos, color);
      }
    }
  }

  left_strip.show();
  right_strip.show();

  // Advance animation frame
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate >= RIPPLE_INTERVAL) {
    ripple_frame++;
    if (ripple_frame > ripple_max_offset * 2) {
      ripple_frame = 0;
    }
    lastUpdate = millis();
  }
}

uint32_t getTrailColor(uint8_t offset) {
  switch (offset) {
    case 0: return left_strip.Color(180, 0, 0);   // red
    case 1: return left_strip.Color(100, 0, 0);   // darker red
    case 2: return left_strip.Color(50, 0, 0);    // even darker
    case 3: return left_strip.Color(20, 0, 0);    // fading out
    default: return 0;                            // off
  }
}

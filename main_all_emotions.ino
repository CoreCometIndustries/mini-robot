// Existing includes and definitions remain the same
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const int max_animation_index = 13; // updated for new emotions
int current_animation_index = 0;
 
#define RIGHT_PIN D5  // GPIO5
#define LEFT_PIN  D6  // GPIO4
#define HEAD_PIN  D7  // GPIO14
#define BACK_PIN  D8  // GPIO12

volatile bool inputChanged = false;

enum Mode {
  IDLE,
  MOVE_RIGHT,
  MOVE_LEFT,
  SLEEP,
  ANGRY,
  HAPPY_CONFUSED,
  SURPRISED,
  WINK,
  WAKEUP
};

Mode currentMode = IDLE;

void ICACHE_RAM_ATTR anyTouchChange() {
  inputChanged = true;
}

// Reference state
int ref_eye_height = 40;
int ref_eye_width = 40;
int ref_space_between_eye = 10;
int ref_corner_radius = 10;

// Current eye positions
int left_eye_height = ref_eye_height;
int left_eye_width = ref_eye_width;
int left_eye_x = 32;
int left_eye_y = 32;
int right_eye_x = 32 + ref_eye_width + ref_space_between_eye;
int right_eye_y = 32;
int right_eye_height = ref_eye_height;
int right_eye_width = ref_eye_width;

//--------------------------------------------
// Helper functions (draw_eyes, center_eyes, blink, etc.)
//--------------------------------------------
void draw_eyes(bool update = true) {
  display.clearDisplay();
  int x = int(left_eye_x - left_eye_width / 2);
  int y = int(left_eye_y - left_eye_height / 2);
  display.fillRoundRect(x, y, left_eye_width, left_eye_height, ref_corner_radius, SSD1306_WHITE);
  x = int(right_eye_x - right_eye_width / 2);
  y = int(right_eye_y - right_eye_height / 2);
  display.fillRoundRect(x, y, right_eye_width, right_eye_height, ref_corner_radius, SSD1306_WHITE);
  if (update) display.display();
}

void center_eyes(bool update = true) {
  left_eye_height = ref_eye_height;
  left_eye_width = ref_eye_width;
  right_eye_height = ref_eye_height;
  right_eye_width = ref_eye_width;
  left_eye_x = SCREEN_WIDTH / 2 - ref_eye_width / 2 - ref_space_between_eye / 2;
  left_eye_y = SCREEN_HEIGHT / 2;
  right_eye_x = SCREEN_WIDTH / 2 + ref_eye_width / 2 + ref_space_between_eye / 2;
  right_eye_y = SCREEN_HEIGHT / 2;
  draw_eyes(update);
}

void blink(int speed = 12) {
  draw_eyes();
  for (int i = 0; i < 3; i++) {
    left_eye_height -= speed;
    right_eye_height -= speed;
    draw_eyes();
    delay(1);
  }
  for (int i = 0; i < 3; i++) {
    left_eye_height += speed;
    right_eye_height += speed;
    draw_eyes();
    delay(1);
  }
}

void sleep() {
  left_eye_height = 2;
  right_eye_height = 2;
  draw_eyes(true);
}

void wakeup() {
  sleep();
  for (int h = 0; h <= ref_eye_height; h += 2) {
    left_eye_height = h;
    right_eye_height = h;
    draw_eyes(true);
  }
}

void happy_eye() {
  center_eyes(false);
  int offset = ref_eye_height / 2;
  for (int i = 0; i < 10; i++) {
    display.fillTriangle(left_eye_x - left_eye_width / 2 - 1, left_eye_y + offset,
                         left_eye_x + left_eye_width / 2 + 1, left_eye_y + 5 + offset,
                         left_eye_x - left_eye_width / 2 - 1, left_eye_y + left_eye_height + offset, SSD1306_BLACK);
    display.fillTriangle(right_eye_x + right_eye_width / 2 + 1, right_eye_y + offset,
                         right_eye_x - left_eye_width / 2 - 1, right_eye_y + 5 + offset,
                         right_eye_x + right_eye_width / 2 + 1, right_eye_y + right_eye_height + offset, SSD1306_BLACK);
    offset -= 2;
    display.display();
    delay(1);
  }
  delay(1000);
}

void saccade(int direction_x, int direction_y) {
  int dx = 8, dy = 6, blink_amt = 8;
  for (int i = 0; i < 1; i++) {
    left_eye_x += dx * direction_x;
    right_eye_x += dx * direction_x;
    left_eye_y += dy * direction_y;
    right_eye_y += dy * direction_y;
    right_eye_height -= blink_amt;
    left_eye_height -= blink_amt;
    draw_eyes();
    delay(1);
  }
  for (int i = 0; i < 1; i++) {
    left_eye_x += dx * direction_x;
    right_eye_x += dx * direction_x;
    left_eye_y += dy * direction_y;
    right_eye_y += dy * direction_y;
    right_eye_height += blink_amt;
    left_eye_height += blink_amt;
    draw_eyes();
    delay(1);
  }
}

void move_big_eye(int direction) {
  int oversize = 1, move_amp = 2, blink_amt = 5;
  for (int i = 0; i < 3; i++) {
    left_eye_x += move_amp * direction;
    right_eye_x += move_amp * direction;
    right_eye_height -= blink_amt;
    left_eye_height -= blink_amt;
    if (direction > 0) { right_eye_height += oversize; right_eye_width += oversize; }
    else { left_eye_height += oversize; left_eye_width += oversize; }
    draw_eyes(); delay(1);
  }
  for (int i = 0; i < 3; i++) {
    left_eye_x += move_amp * direction;
    right_eye_x += move_amp * direction;
    right_eye_height += blink_amt;
    left_eye_height += blink_amt;
    if (direction > 0) { right_eye_height += oversize; right_eye_width += oversize; }
    else { left_eye_height += oversize; left_eye_width += oversize; }
    draw_eyes(); delay(1);
  }
  delay(1000);
  for (int i = 0; i < 3; i++) {
    left_eye_x -= move_amp * direction;
    right_eye_x -= move_amp * direction;
    right_eye_height -= blink_amt;
    left_eye_height -= blink_amt;
    if (direction > 0) { right_eye_height -= oversize; right_eye_width -= oversize; }
    else { left_eye_height -= oversize; left_eye_width -= oversize; }
    draw_eyes(); delay(1);
  }
  for (int i = 0; i < 3; i++) {
    left_eye_x -= move_amp * direction;
    right_eye_x -= move_amp * direction;
    right_eye_height += blink_amt;
    left_eye_height += blink_amt;
    if (direction > 0) { right_eye_height -= oversize; right_eye_width -= oversize; }
    else { left_eye_height -= oversize; left_eye_width -= oversize; }
    draw_eyes(); delay(1);
  }
  center_eyes();
}

void move_right_big_eye() { move_big_eye(1); }
void move_left_big_eye() { move_big_eye(-1); }

//--------------------------------------------
// New emotions
//--------------------------------------------
void angry_eye() {
  center_eyes(false);
  // Draw slanted rectangles
  display.fillTriangle(left_eye_x - left_eye_width/2, left_eye_y - ref_eye_height/2,
                       left_eye_x + left_eye_width/2, left_eye_y - ref_eye_height/2 + 5,
                       left_eye_x + left_eye_width/2, left_eye_y + ref_eye_height/2, SSD1306_WHITE);
  display.fillTriangle(right_eye_x + right_eye_width/2, right_eye_y - ref_eye_height/2,
                       right_eye_x - left_eye_width/2, right_eye_y - ref_eye_height/2 + 5,
                       right_eye_x - left_eye_width/2, right_eye_y + ref_eye_height/2, SSD1306_WHITE);
  display.display();
  delay(1000);
}

void confused_eye() {
  center_eyes(false);
  left_eye_width = 30; left_eye_height = 40;
  right_eye_width = 40; right_eye_height = 30;
  draw_eyes(true);
  delay(1000);
}

void surprised_eye() {
  center_eyes(false);
  left_eye_width = 45; left_eye_height = 45;
  right_eye_width = 45; right_eye_height = 45;
  draw_eyes(true);
  delay(1000);
}

void sad_eye() {
  center_eyes(false);
  int offset = ref_eye_height / 2;
  for (int i = 0; i < 10; i++) {
    display.fillTriangle(left_eye_x - left_eye_width / 2, left_eye_y - offset,
                         left_eye_x + left_eye_width / 2, left_eye_y - 5 - offset,
                         left_eye_x - left_eye_width / 2, left_eye_y - left_eye_height - offset, SSD1306_BLACK);
    display.fillTriangle(right_eye_x + right_eye_width / 2, right_eye_y - offset,
                         right_eye_x - left_eye_width / 2, right_eye_y - 5 - offset,
                         right_eye_x + right_eye_width / 2, right_eye_y - right_eye_height - offset, SSD1306_BLACK);
    offset -= 2;
    display.display();
    delay(1);
  }
  delay(1000);
}

void wink_eye() {
  center_eyes(false);
  left_eye_height = 2;
  draw_eyes(true);
  delay(1000);
}

//--------------------------------------------
// Animation index runner
//--------------------------------------------
void launch_animation_with_index(int animation_index) {
  switch (animation_index) {
    case 0: wakeup(); break;
    case 1: center_eyes(true); break;
    case 2: move_right_big_eye(); break;
    case 3: move_left_big_eye(); break;
    case 4: blink(10); break;
    case 5: blink(20); break;
    case 6: happy_eye(); break;
    case 7: sleep(); break;
    case 8: center_eyes(true); for (int i = 0; i < 20; i++) { int dx = random(-1, 2), dy = random(-1, 2); saccade(dx, dy); delay(1); saccade(-dx, -dy); delay(1); } break;
    case 9: angry_eye(); break;
    case 10: confused_eye(); break;
    case 11: surprised_eye(); break;
    case 12: sad_eye(); break;
    case 13: wink_eye(); break;
  }
}

//--------------------------------------------
// Setup and loop
//--------------------------------------------
void setup() {
 pinMode(RIGHT_PIN, INPUT);
  pinMode(LEFT_PIN, INPUT);
  pinMode(HEAD_PIN, INPUT);
  pinMode(BACK_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(RIGHT_PIN), anyTouchChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(LEFT_PIN), anyTouchChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HEAD_PIN), anyTouchChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BACK_PIN), anyTouchChange, CHANGE);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("hi cheeki !"));
  display.display();
  delay(2000);
 wakeup();
  delay(2000);
}

void loop() {
  // If inputs changed, decide what to do
  if (inputChanged) {
    inputChanged = false;
    updateMode();
  }

  // Run animations based on current mode
  switch (currentMode) {
    case MOVE_RIGHT: move_right_big_eye(); break;
    case MOVE_LEFT:  move_left_big_eye(); break;
    case SLEEP: sleep(); break;
    case ANGRY: angry_eye(); break;
    case HAPPY_CONFUSED: happy_eye(); break;
    case SURPRISED: surprised_eye(); break;
    case WINK: wink_eye(); break;
    case WAKEUP: wakeup(); break;
    case IDLE: center_eyes(true); break; // background blinking
  }
}






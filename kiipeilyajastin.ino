#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 8, 7);

const int DELAY_CONSTANT = 3000;
const int DELAY_RANDOM = 1000;

const int btn_start = 3;
const int btn_stop = 2;
const int buzzer = 6;

volatile long int start = 0;
volatile long int stop = 0;

long int last_lcd_update = millis();

volatile bool timer = false;
volatile bool btn_start_high = false;
volatile bool btn_stop_high = false;

bool armed = false;
bool started = false;
volatile bool stopped = false;

void setup() {
  lcd.begin(16, 2);
  print_time(0);
  print_reaction_time(0);
  print_state("virita ajastin  ");

  Serial.begin(115200);
  Serial.setTimeout(1);

  pinMode(btn_start, INPUT_PULLUP);
  pinMode(btn_stop, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(btn_start), start_change, CHANGE);

  attachInterrupt(digitalPinToInterrupt(btn_stop), stop_change, CHANGE);

  pinMode(buzzer, OUTPUT);
}

void loop() {
  if (timer) {
    if (millis() - last_lcd_update > 100) {
      last_lcd_update = millis();
      print_time(millis() - start);
    }
  } else if (armed) {
    if (btn_start_high && !started) {
      handle_start();

    } else if (stopped) {
      handle_stop();
    }

  } else {
    if (btn_stop_high) {
      armed = true;
      print_state("viritetty       ");
    }
  }
}

void start_change() {
  if (digitalRead(btn_start) == HIGH) start_rising();
  else start_falling();
}

void start_rising() {
  btn_start_high = true;
}

void start_falling() {
  btn_start_high = false;
  if (timer) {
    print_reaction_time(millis() - start);
  }
}

void stop_change() {
  if (digitalRead(btn_stop) == HIGH) stop_rising();
  else stop_falling();
}

void stop_rising() {
  btn_stop_high = true;
  if (timer) {
    stop = millis();
    stopped = true;
    timer = false;
  }
}

void stop_falling() {
  btn_stop_high = false;
}

void print_time(long int time) {
  char seconds_string[7];
  float seconds = (time % 60000) / 1000.;
  dtostrf(seconds, 6, 3, seconds_string);
  char time_string[10];
  int minutes = time / 60000;
  sprintf(time_string, "%02d:%s", minutes, seconds_string);
  lcd.home();
  lcd.print(time_string);
}

void print_state(char state[16]) {
  lcd.setCursor(0, 1);
  lcd.print(state);
}

void print_reaction_time(int reaction_time) {
  lcd.setCursor(10, 0);
  char reaction_time_string[7];
  sprintf(reaction_time_string, "%04dms", reaction_time);
  lcd.print(reaction_time_string);
}

void handle_start() {
  started = true;
  Serial.println(-2);
  print_time(0);
  print_reaction_time(0);
  print_state("aktivoitu       ");

  delay(DELAY_CONSTANT - 200 + random(DELAY_RANDOM));

  if (btn_start_high) {
    start = millis();
    timer = true;
    Serial.println(-1);
    print_state("lahto hyvaksytty");

    digitalWrite(buzzer, HIGH);
    delay(700);
    digitalWrite(buzzer, LOW);
  } else {
    print_state("varaslahto      ");
    started = false;
    for (int i = 0; i < 3; i++) {
      delay(100);
      digitalWrite(buzzer, HIGH);
      delay(300);
      digitalWrite(buzzer, LOW);
    }
  }
}

void handle_stop() {
  stopped = false;
  started = false;
  armed = false;
  Serial.println(stop - start);
  print_time(stop - start);
  print_state("lopetus         ");

  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  delay(1700);
  print_state("virita ajastin  ");
}

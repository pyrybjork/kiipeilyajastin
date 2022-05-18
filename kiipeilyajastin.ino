#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 8, 7);

const int DELAY_CONSTANT = 3000;
const int DELAY_RANDOM = 1000;

//pinnejä
const int btn_start = 3;
const int btn_stop = 2;
const int buzzer = 6;

//ajanoton lopetus ja aloitusajat millis() funktion palauttamina
volatile long int start = 0;
volatile long int stop = 0;

long int last_lcd_update = 0;

volatile bool timer = false;
volatile bool btn_start_high = false;
volatile bool btn_stop_high = false;

bool armed = false; //onko lähtölevy aktivoitu eli alkaako ajanotto jos lähtölevylle mennään seisomaan
bool started = false; //tosi, jos lähtöjutut(handle_start()) suoritettu jo kerran. Estää sen, ettei niitä suoriteta uudestaan. Vaihdetaan lopetuksessa epätodeksi.
volatile bool stopped = false; //Kun lopetus nappia painetaan, arvoksi asetetaan tosi. handle_stop() suoritetaan seuraavalla loopilla, joilloin arvoksi asetetaan taas epätosi.

void setup() {
  lcd.begin(16, 2);
  print_time(0);
  print_reaction_time(0);
  print_state("aktivoi lähtö   ");

  Serial.begin(115200);
  Serial.setTimeout(1);

  pinMode(btn_start, INPUT_PULLUP);
  pinMode(btn_stop, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(btn_start), start_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(btn_stop), stop_change, CHANGE);

  pinMode(buzzer, OUTPUT);
}

void loop() {
  if (timer) { //jos ajanotto käynnissä, päivitetään näyttöä 100ms välein
    
    if (millis() - last_lcd_update > 100) {
      last_lcd_update = millis();
      print_time(millis() - start);
    }
    
  } else if (armed) { //jos lähtölevy on aktivoitu edellisen kiipeilyn jälkeen tarkistetaan onko tapahtunut aloitusta tai lopetusta.
    
    if (btn_start_high && !started) { //aloitus
      handle_start();

    } else if (stopped) { //lopetus
      handle_stop();
    }

  } else {
    
    if (btn_stop_high) { //lähtölevyn aktivointi. Sama pinni käytössä kun lopetukseenkin
      armed = true;
      print_state("lähtö aktivoitu ");
    }
  }
  
}

//keskeytyksien käsittely
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

//tulosta millisekunti aika vasempaan ylänurkkaan muodossa minuutit:sekuntit(kolme desimaalia)
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

//tulosta laitteen tila alariville
void print_state(char state[16]) {
  lcd.setCursor(0, 1);
  lcd.print(state);
}

//tulosta millisekunti aika muodossa millisekuntit ms oikeaan ylänurkkaan. Käytetään ajan, joka menee lähtömerkistä siihen, että lähtölevyltä noustaan, kertomiseen
void print_reaction_time(int reaction_time) {
  lcd.setCursor(10, 0);
  char reaction_time_string[7];
  sprintf(reaction_time_string, "%04dms", reaction_time);
  lcd.print(reaction_time_string);
}

//suoritetaan kun lähtölevy aktivoidaan
void handle_start() {
  started = true;
  Serial.println(-2); //tietokoneen kellolle nollaus käsky
  print_time(0);
  print_reaction_time(0);
  print_state("lahtoon 3s - 4s ");

  delay(DELAY_CONSTANT + random(DELAY_RANDOM)); //odotus

  if (btn_start_high) { //hyväksytty lähtö
    
    start = millis();
    timer = true;
    Serial.println(-1); //tietokoneen kellolle aloitus käsky
    print_state("lahto hyvaksytty");

    digitalWrite(buzzer, HIGH);
    delay(700);
    digitalWrite(buzzer, LOW);
    
  } else { //varaslähtö
    
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

//suoritetaan lopetuksessa
void handle_stop() {
  stopped = false;
  started = false;
  armed = false;
  Serial.println(stop - start); //tietokoneen kellolle kerrotaan lopullinen aika
  print_time(stop - start);
  print_state("lopetus         ");

  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  delay(1700);
  print_state("aktivoi lähtö   ");
}

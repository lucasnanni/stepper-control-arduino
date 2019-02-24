#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <RotaryEncoder.h>

#define STEP_PIN        54
#define DIR_PIN         55
#define ENABLE_PIN      38

#define LCD_PINS_RS     16
#define LCD_PINS_ENABLE 17
#define LCD_PINS_D4     23
#define LCD_PINS_D5     25
#define LCD_PINS_D6     27
#define LCD_PINS_D7     29

#define BTN_STOP        41
#define BTN_ENTER       35
#define BTN_EN1         31
#define BTN_EN2         33

#define SPEED_MIN        0
#define SPEED_MAX      500
#define SPEED_STEP      10

#define ENC_SAME         0
#define ENC_NEXT         1
#define ENC_PREV        -1

#define cycleNext(x,min,max) ((x)>=(max)?(min):(x)+1)
#define cyclePrev(x,min,max) ((x)<=(min)?(max):(x)-1)

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);
RotaryEncoder knob(BTN_EN2, BTN_EN1);

int speed = SPEED_MIN;

int knobRead() {
  static int pos = 0;
  int dir, new_pos;
  knob.tick();
  new_pos = knob.getPosition();
  if (new_pos > pos) {
    dir = ENC_NEXT;
  } else if (new_pos < pos) {
    dir = ENC_PREV;
  } else {
    dir = ENC_SAME;
  }
  pos = new_pos;
  return dir;
}

bool buttonRead(int btn) {
  if (digitalRead(btn) == LOW) {
    while (digitalRead(btn) == LOW);
    delay(10);
    return true;
  }
  return false;
}

void mainMenu() {
  int prev_option=-1, option=0;
  while (true) {
    lcd.clear();
    lcd.print("  Velocidade ("); lcd.print(speed); lcd.print(")"); lcd.setCursor(0,1);
    lcd.print("  Avancar >>>"); lcd.setCursor(0, 2);
    lcd.print("  Retroceder <<<"); lcd.setCursor(0, 3);
    lcd.print("  Ajustar Posicao");
    lcd.setCursor(0, option);
    lcd.write(0x7E);
    while (!buttonRead(BTN_ENTER)) {
      int enc = knobRead();
      if (enc != ENC_SAME) {
        prev_option = option;
        if (enc == ENC_NEXT) {
          option = cycleNext(option, 0, 3);
        } else {
          option = cyclePrev(option, 0, 3);
        }
        lcd.setCursor(0, prev_option);
        lcd.print(" ");
        lcd.setCursor(0, option);
        lcd.write(0x7E);
      }
    }
    switch (option) {
      case 0: optionSpeed(); break;
      case 1: optionForward(); break;
      case 2: optionBackward(); break;
      case 3: optionPosition(); break;
    }
  }
}

void optionSpeed() {
  lcd.clear();
  lcd.print("Velocidade ("); lcd.print(SPEED_MIN); lcd.print("-"); lcd.print(SPEED_MAX); lcd.print(")");
  lcd.setCursor(0, 1);
  lcd.write(0x7E); lcd.print(" "); lcd.print(speed);
  while (!buttonRead(BTN_ENTER)) {
    int enc = knobRead();
    if (enc != ENC_SAME) {
      if (enc == ENC_NEXT) {
        speed = min(speed+SPEED_STEP, SPEED_MAX);
      } else {
        speed = max(speed-SPEED_STEP, SPEED_MIN);
      }
      lcd.setCursor(2, 1); lcd.print("   "); lcd.setCursor(2, 1);
      lcd.print(speed);
      lcd.setCursor(0, 3); lcd.print("[Enter] p/ salvar");
    }
  }
}

void optionForward() {
  lcd.clear();
  lcd.print("Avancando ...");
  lcd.setCursor(0, 1);
  lcd.print("Velocidade = "); lcd.print(speed);
  lcd.setCursor(0, 3);
  lcd.print("[Stop] p/ parar");
  stepper.setSpeed(speed);
  while(!buttonRead(BTN_STOP)) {
    stepper.runSpeed();
  }
  stepper.setSpeed(0);
}

void optionBackward() {
  lcd.clear();
  lcd.print("Retrocedendo ...");
  lcd.setCursor(0, 1);
  lcd.print("Velocidade = "); lcd.print(speed);
  lcd.setCursor(0, 3);
  lcd.print("[Stop] p/ parar");
  stepper.setSpeed(-speed);
  while(!buttonRead(BTN_STOP)) {
    stepper.runSpeed();
  }
  stepper.setSpeed(0);
}

void optionPosition() {
  int dir = 0;
  lcd.clear();
  lcd.print("Posicao:");
  lcd.setCursor(1, 9); lcd.write(0x7F);
  lcd.setCursor(1, 11); lcd.write(0x7E);
  lcd.setCursor(5, 1); lcd.print("[   |   ]");
  lcd.setCursor(0, 3); lcd.print("[Stop] p/ sair");
  while (!buttonRead(BTN_STOP)) {
    int enc = knobRead();
    if (enc != ENC_SAME) {
      if (enc == ENC_NEXT) {
        dir = min(dir+1, 3);
      } else {
        dir = max(dir-1, -3);
      }
      lcd.setCursor(5, 1);
      switch (dir) {
        case  0: lcd.print("[   |   ]"); break;
        case  1: lcd.print("[   |>  ]"); break;
        case  2: lcd.print("[   |>> ]"); break;
        case  3: lcd.print("[   |>>>]"); break;
        case -1: lcd.print("[  <|   ]"); break;
        case -2: lcd.print("[ <<|   ]"); break;
        case -3: lcd.print("[<<<|   ]"); break;
      }
      stepper.setSpeed(dir*20);
    }
    stepper.runSpeed();
  }
  stepper.setSpeed(0);
}

void setup() {  
  stepper.setPinsInverted(false, false, true);
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setMaxSpeed(SPEED_MAX);
  lcd.begin(20, 4);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
}

void loop() {  
  mainMenu();
}

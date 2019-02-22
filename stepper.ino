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

#define VMIN             0
#define VMAX           500
#define VPASSO          10

#define ENC_SAME         0
#define ENC_NEXT         1
#define ENC_PREV        -1

#define proxCiclo(x,min,max) ((x)>=(max)?(min):(x)+1)
#define anteCiclo(x,min,max) ((x)<=(min)?(max):(x)-1)

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);
RotaryEncoder encoder(BTN_EN2, BTN_EN1);

int velocidade = 1;

int lerEncoder() {
  static int pos = 0;
  encoder.tick();
  int pos_enc = encoder.getPosition();
  if (pos_enc > pos) {
    pos = pos_enc;
    return ENC_NEXT;
  } else if (pos_enc < pos) {
    pos = pos_enc;
    return ENC_PREV;
  }
  pos = pos_enc;
  return ENC_SAME;
}

bool botao(int btn) {
  if (digitalRead(btn) == LOW) {
    while (digitalRead(btn) == LOW);
    delay(10);
    return true;
  }
  return false;
}

void menuPrincipal() {
  int opcao_ant=-1, opcao=0;
  while (true) {
    lcd.clear();
    lcd.print("  Velocidade ("); lcd.print(velocidade); lcd.print(")"); lcd.setCursor(0,1);
    lcd.print("  Avancar >>>"); lcd.setCursor(0,2);
    lcd.print("  Retroceder <<<"); lcd.setCursor(0,3);
    lcd.print("  Ajustar Posicao");
    lcd.setCursor(0, opcao);
    lcd.write(0x7E);
    while (!botao(BTN_ENTER)) {
      int enc = lerEncoder();
      if (enc != ENC_SAME) {
        opcao_ant = opcao;
        if (enc == ENC_NEXT) {
          opcao = proxCiclo(opcao, 0, 3);
        } else {
          opcao = anteCiclo(opcao, 0, 3);
        }
        lcd.setCursor(0, opcao_ant);
        lcd.print(" ");
        lcd.setCursor(0, opcao);
        lcd.write(0x7E);
      }
    }
    switch (opcao) {
      case 0: opcaoVelocidade(); break;
      case 1: opcaoAvancar(); break;
      case 2: opcaoRetroceder(); break;
      case 3: opcaoPosicao(); break;
    }
  }
}

void opcaoVelocidade() {
  lcd.clear();
  lcd.print("Velocidade ("); lcd.print(VMIN); lcd.print("-"); lcd.print(VMAX); lcd.print(")");
  lcd.setCursor(0, 1);
  lcd.write(0x7E); lcd.print(" "); lcd.print(velocidade);
  while (!botao(BTN_ENTER)) {
    int enc = lerEncoder();
    if (enc != ENC_SAME) {
      if (enc == ENC_NEXT) {
        velocidade = min(velocidade+VPASSO, VMAX);
      } else {
        velocidade = max(velocidade-VPASSO, VMIN);
      }
      lcd.setCursor(2, 1); lcd.print("   "); lcd.setCursor(2, 1);
      lcd.print(velocidade);
      lcd.setCursor(0, 3); lcd.print("[Enter] p/ salvar");
    }
  }
}

void opcaoAvancar() {
  lcd.clear();
  lcd.print("Avancando ...");
  lcd.setCursor(0, 1);
  lcd.print("Velocidade = "); lcd.print(velocidade);
  lcd.setCursor(0, 3);
  lcd.print("[Stop] p/ parar");
  stepper.setSpeed(velocidade);
  while(!botao(BTN_STOP)) {
    stepper.runSpeed();
  }
  stepper.setSpeed(0);
}

void opcaoRetroceder() {
  lcd.clear();
  lcd.print("Retrocedendo ...");
  lcd.setCursor(0, 1);
  lcd.print("Velocidade = "); lcd.print(velocidade);
  lcd.setCursor(0, 3);
  lcd.print("[Stop] p/ parar");
  stepper.setSpeed(-velocidade);
  while(!botao(BTN_STOP)) {
    stepper.runSpeed();
  }
  stepper.setSpeed(0);
}

void opcaoPosicao() {
  int dir = 0;
  lcd.clear();
  lcd.print("Posicao:");
  lcd.setCursor(1, 9); lcd.write(0x7F);
  lcd.setCursor(1, 11); lcd.write(0x7E);
  lcd.setCursor(5, 1); lcd.print("[   |   ]");
  lcd.setCursor(0, 3); lcd.print("[Stop] p/ sair");
  while (!botao(BTN_STOP)) {
    int enc = lerEncoder();
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
  stepper.setMaxSpeed(VMAX);
  lcd.begin(20, 4);
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
}

void loop() {  
  menuPrincipal();
}

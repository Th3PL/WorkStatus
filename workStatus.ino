#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========== PINOS ==========
const int PIN_LDR = A0;
const int PIN_LED_R = 9;
const int PIN_LED_G = 10;
const int PIN_LED_B = 11;
const int PIN_BUZZER = 8;
const int PIN_BTN_START = 2;
const int PIN_BTN_SELECT = 3;
const int PIN_BTN_MENU = 4;

// ========== LCD ==========
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ajustar endereço se necessário (pode ser 0x3F)

// ========== ESTADOS ==========
enum Status {
  LIVRE = 0,
  OCUPADO = 1,
  FOCO = 2,
  REUNIAO = 3
};

Status estadoAtual = LIVRE;
const char* nomesEstados[] = {"LIVRE", "OCUPADO", "FOCO", "REUNIAO"};

// ========== VARIÁVEIS ==========
unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 5000; // Envia status a cada 5s
unsigned long ultimaLeituraLDR = 0;
int valorLDR = 0;
bool estadoMudou = true;

// Debounce dos botões
unsigned long ultimoDebounceStart = 0;
unsigned long ultimoDebounceSelect = 0;
unsigned long ultimoDebounceMenu = 0;
const unsigned long debounceDelay = 200;

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Configurar pinos
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_BTN_SELECT, INPUT_PULLUP);
  pinMode(PIN_BTN_MENU, INPUT_PULLUP);
  pinMode(PIN_LDR, INPUT);
  
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WorkStatus Light");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  
  delay(2000);
  atualizarLED();
  atualizarLCD();
}

// ========== LOOP ==========
void loop() {
  unsigned long agora = millis();
  
  // Ler botões com debounce
  if (digitalRead(PIN_BTN_START) == LOW && (agora - ultimoDebounceStart) > debounceDelay) {
    ultimoDebounceStart = agora;
    // Alterna entre LIVRE e OCUPADO
    if (estadoAtual == LIVRE) {
      estadoAtual = OCUPADO;
    } else if (estadoAtual == OCUPADO) {
      estadoAtual = LIVRE;
    } else {
      estadoAtual = LIVRE; // Volta para livre se estava em outro estado
    }
    estadoMudou = true;
    beepSimples();
  }
  
  if (digitalRead(PIN_BTN_SELECT) == LOW && (agora - ultimoDebounceSelect) > debounceDelay) {
    ultimoDebounceSelect = agora;
    estadoAtual = FOCO;
    estadoMudou = true;
    beepFoco();
  }
  
  if (digitalRead(PIN_BTN_MENU) == LOW && (agora - ultimoDebounceMenu) > debounceDelay) {
    ultimoDebounceMenu = agora;
    estadoAtual = REUNIAO;
    estadoMudou = true;
    beepSimples();
  }
  
  // Ler LDR periodicamente
  if (agora - ultimaLeituraLDR > 1000) {
    valorLDR = analogRead(PIN_LDR);
    ultimaLeituraLDR = agora;
  }
  
  // Atualizar interface se mudou
  if (estadoMudou) {
    atualizarLED();
    atualizarLCD();
    enviarStatus();
    estadoMudou = false;
    ultimoEnvio = agora; 
  }
  

  if (agora - ultimoEnvio > intervaloEnvio) {
    enviarStatus();
    ultimoEnvio = agora;
  }
  
  // Piscar LED se em reunião
  if (estadoAtual == REUNIAO) {
    piscarVermelho();
  }
  
  // Receber comandos do Python via Serial (opcional)
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    processarComando(comando);
  }
}

// ========== FUNÇÕES ==========
void atualizarLED() {
  switch (estadoAtual) {
    case LIVRE:
      setRGB(0, 255, 0); 
      break;
    case OCUPADO:
      setRGB(255, 200, 0);
      break;
    case FOCO:
      setRGB(255, 0, 0); 
      break;
    case REUNIAO:
      setRGB(255, 0, 0); 
      break;
  }
}

void setRGB(int r, int g, int b) {
  analogWrite(PIN_LED_R, r);
  analogWrite(PIN_LED_G, g);
  analogWrite(PIN_LED_B, b);
}

void atualizarLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status:");
  lcd.setCursor(8, 0);
  lcd.print(nomesEstados[estadoAtual]);
  
  lcd.setCursor(0, 1);
  lcd.print("Luz:");
  lcd.setCursor(6, 1);
  lcd.print(valorLDR);
  
  // Indicador de presença baseado em LDR
  lcd.setCursor(12, 1);
  if (valorLDR < 100) {
    lcd.print("AUS"); // Ausente (muito escuro)
  } else {
    lcd.print("PRE"); // Presente
  }
}

void enviarStatus() {
  // Formato: ID=01;STATE=LIVRE;LDR=523;TS=123456
  Serial.print("ID=01;");
  Serial.print("STATE=");
  Serial.print(nomesEstados[estadoAtual]);
  Serial.print(";LDR=");
  Serial.print(valorLDR);
  Serial.print(";TS=");
  Serial.println(millis());
}

void beepSimples() {
  tone(PIN_BUZZER, 1000, 100);
}

void beepFoco() {
  tone(PIN_BUZZER, 1200, 150);
  delay(200);
  tone(PIN_BUZZER, 1400, 150);
}

void piscarVermelho() {
  static unsigned long ultimoPisca = 0;
  static bool ligado = true;
  
  if (millis() - ultimoPisca > 500) {
    ligado = !ligado;
    if (ligado) {
      setRGB(255, 0, 0);
    } else {
      setRGB(0, 0, 0);
    }
    ultimoPisca = millis();
  }
}

void processarComando(String cmd) {
  // Comandos remotos via MQTT → Python → Serial
  if (cmd == "CMD:LIVRE") {
    estadoAtual = LIVRE;
    estadoMudou = true;
    beepSimples();
  } else if (cmd == "CMD:OCUPADO") {
    estadoAtual = OCUPADO;
    estadoMudou = true;
    beepSimples();
  } else if (cmd == "CMD:FOCO") {
    estadoAtual = FOCO;
    estadoMudou = true;
    beepFoco();
  } else if (cmd == "CMD:REUNIAO") {
    estadoAtual = REUNIAO;
    estadoMudou = true;
    beepSimples();
  }
}

// ============================================================
//  modulo_pot_audio.ino — P4
//  Minijuego: potenciómetro al valor exacto
//  Audio: WTC020 + PAM8403 + altavoz EP-40
// ============================================================

// --- Pin del potenciómetro ---
#define PIN_POT A3

// --- Pines del WTC020 (con resistencias 1kΩ en serie) ---
#define WTC_CLK  9
#define WTC_DATA 10

// --- Variables del minijuego ---
int valorObjetivo  = 0;   // Valor que el jugador debe alcanzar (0-1023)
int margenAcierto  = 50;  // Cuánto margen hay para acertar
int valorActual    = 0;   // Lectura actual del potenciómetro
unsigned long potUltimaLectura = 0;

// ============================================================
//  setupPotAudio() — llamada desde main.ino una sola vez
// ============================================================
void setupPotAudio() {
  pinMode(WTC_CLK,  OUTPUT);
  pinMode(WTC_DATA, OUTPUT);
  digitalWrite(WTC_CLK,  HIGH);
  digitalWrite(WTC_DATA, HIGH);
  delay(100);  // Esperar a que el WTC020 arranque
}

// ============================================================
//  iniciarPotAudio() — llamada cuando empieza este minijuego
// ============================================================
void iniciarPotAudio() {
  // Margen según dificultad
  margenAcierto = (nivel == 1) ? 80 : (nivel == 2) ? 40 : 20;

  // Valor objetivo aleatorio (evitar extremos)
  valorObjetivo = random(100, 900);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== POTENCIOMETRO ==");
  lcd.setCursor(0, 1); lcd.print("Objetivo: ");
  lcd.print(valorObjetivo);
  lcd.setCursor(0, 3); lcd.print("Gira hasta llegar!");
  potUltimaLectura = millis();
}

// ============================================================
//  loopPotAudio() — llamada en cada ciclo del loop() principal
// ============================================================
void loopPotAudio() {
  // Leer cada 100ms para no saturar la LCD
  if (millis() - potUltimaLectura < 100) return;
  potUltimaLectura = millis();

  valorActual = analogRead(PIN_POT);

  // Mostrar valor actual y barra de progreso
  lcd.setCursor(0, 2);
  lcd.print("Actual: ");
  lcd.print(valorActual);
  lcd.print("   ");

  // Barra visual de progreso
  mostrarBarraPot();

  // Comprobar si está en el rango correcto
  int diferencia = abs(valorActual - valorObjetivo);

  if (diferencia <= margenAcierto) {
    lcd.setCursor(0, 3); lcd.print("Perfecto!!          ");
    delay(1500);
    potSuperado = true;
    cambiarEstado(GANASTE);
  } else if (diferencia < 150) {
    lcd.setCursor(0, 3); lcd.print("Muy cerca!          ");
  } else if (valorActual < valorObjetivo) {
    lcd.setCursor(0, 3); lcd.print("Gira a la derecha   ");
  } else {
    lcd.setCursor(0, 3); lcd.print("Gira a la izquierda ");
  }
}

// Barra visual en la LCD — muestra posición relativa
void mostrarBarraPot() {
  int posObjetivo = map(valorObjetivo, 0, 1023, 0, 19);
  int posActual   = map(valorActual,   0, 1023, 0, 19);

  lcd.setCursor(0, 1);
  for (int i = 0; i < 20; i++) {
    if (i == posObjetivo) lcd.print("X");       // Objetivo
    else if (i == posActual) lcd.print("|");    // Posición actual
    else lcd.print("-");
  }
}

// ============================================================
//  AUDIO — WTC020
//  Protocolo: pulsos en CLK y DATA para enviar comandos
// ============================================================

// Reproducir archivo por número (1 = 0001.ad4, 2 = 0002.ad4...)
void reproducirAudio(int numero) {
  // Comando play del WTC020: enviar número de pista
  // El WTC020 en modo serial recibe comandos de 16 bits
  uint16_t cmd = 0xA000 | (numero & 0x00FF);
  enviarComandoWTC(cmd);
}

void enviarComandoWTC(uint16_t cmd) {
  digitalWrite(WTC_DATA, LOW);
  delayMicroseconds(2);

  for (int i = 15; i >= 0; i--) {
    digitalWrite(WTC_CLK, LOW);
    delayMicroseconds(50);
    digitalWrite(WTC_DATA, (cmd >> i) & 1);
    delayMicroseconds(50);
    digitalWrite(WTC_CLK, HIGH);
    delayMicroseconds(50);
  }

  digitalWrite(WTC_DATA, HIGH);
  delayMicroseconds(2);
}

// Reproducir sonido de explosión — llamada desde main.ino
void reproducirExplosion() {
  reproducirAudio(1);  // 0001.ad4 = sonido de explosión
}

// Reproducir sonido de éxito — opcional
void reproducirExito() {
  reproducirAudio(2);  // 0002.ad4 = sonido de victoria
}

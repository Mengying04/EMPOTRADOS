// ============================================================
//  modulo_pot_audio.ino — P4
//  Minijuego: potenciómetro al valor exacto
//
//  Nivel 1 — margen ±80,  valor objetivo zona central (200-800)
//  Nivel 2 — margen ±40,  valor objetivo zona amplia  (100-900)
//  Nivel 3 — margen ±20,  valor objetivo cualquier zona
//  Nivel 4 — margen ±10,  valor objetivo cualquier zona
//
//  Audio: WTC020 + PAM8403 + altavoz EP-40
//  Archivos en SD: 0001.ad4 = explosion, 0002.ad4 = victoria
// ============================================================

#define PIN_POT   A3
#define WTC_CLK    9
#define WTC_DATA  10

// --- Variables del minijuego ---
int  valorObjetivo     = 0;
int  margenAcierto     = 80;
int  valorActual       = 0;
int  ultimoValorActual = -1;  // Para no redibujar si no cambia
unsigned long potUltimaLectura = 0;

// ============================================================
//  Setup e inicio
// ============================================================
void setupPotAudio() {
  pinMode(WTC_CLK,  OUTPUT);
  pinMode(WTC_DATA, OUTPUT);
  digitalWrite(WTC_CLK,  HIGH);
  digitalWrite(WTC_DATA, HIGH);
  delay(100);
}

void iniciarPotAudio() {
  // Configurar margen según nivel
  switch (nivel) {
    case 1: margenAcierto = 80;  valorObjetivo = random(200, 801); break;
    case 2: margenAcierto = 40;  valorObjetivo = random(100, 901); break;
    case 3: margenAcierto = 20;  valorObjetivo = random(50,  974); break;
    case 4: margenAcierto = 10;  valorObjetivo = random(50,  974); break;
  }

  ultimoValorActual = -1;
  potUltimaLectura  = millis();

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("= POTENCIOMETRO =");
  lcd.setCursor(0, 1); lcd.print("Objetivo: ");
  lcd.print(valorObjetivo);
  lcd.print("  (+-");
  lcd.print(margenAcierto);
  lcd.print(")");
  delay(1500);
  lcd.clear();
  mostrarPantallaPot();
}

// ============================================================
//  Loop — llamado desde main cada ciclo
// ============================================================
void loopPotAudio() {
  // Leer cada 80ms
  if (millis() - potUltimaLectura < 80) return;
  potUltimaLectura = millis();

  valorActual = analogRead(PIN_POT);

  // Solo redibujar si el valor cambió más de 3 unidades
  if (abs(valorActual - ultimoValorActual) < 3) return;
  ultimoValorActual = valorActual;

  int diferencia = abs(valorActual - valorObjetivo);

  // Actualizar barra y valor actual
  actualizarBarraPot();

  // Mensaje de pista
  lcd.setCursor(0, 3);
  if (diferencia <= margenAcierto) {
    lcd.print("!! ZONA CORRECTA !! ");
  } else if (diferencia < margenAcierto * 2) {
    lcd.print("Muy cerca!          ");
  } else if (diferencia < margenAcierto * 4) {
    lcd.print("Cerca...            ");
  } else if (valorActual < valorObjetivo) {
    lcd.print("Gira derecha >>     ");
  } else {
    lcd.print("<< Gira izquierda   ");
  }

  // Comprobar acierto
  if (diferencia <= margenAcierto) {
    delay(600);  // Pequeña pausa para confirmar que no es accidental
    valorActual = analogRead(PIN_POT);
    if (abs(valorActual - valorObjetivo) <= margenAcierto) {
      lcd.setCursor(0, 3); lcd.print("Perfecto!! Ganaste! ");
      delay(1500);
      potSuperado = true;
      cambiarEstado(GANASTE);
    }
  }
}

// ============================================================
//  Pantalla principal del minijuego
// ============================================================
void mostrarPantallaPot() {
  lcd.setCursor(0, 0); lcd.print("= POTENCIOMETRO =");
  lcd.setCursor(0, 1); lcd.print("Meta:");
  lcd.print(valorObjetivo);
  lcd.print("  Ahora:");
  // El valor actual se actualiza en actualizarBarraPot()
}

void actualizarBarraPot() {
  // Fila 1: objetivo y valor actual
  lcd.setCursor(0, 1);
  lcd.print("Meta:");
  lcd.print(valorObjetivo);
  lcd.print("  Ahora:");
  lcd.print(valorActual);
  lcd.print("   ");

  // Fila 2: barra de 20 caracteres
  // X = objetivo, | = posición actual, - = vacío
  int posObj    = map(valorObjetivo, 0, 1023, 0, 19);
  int posActual = map(valorActual,   0, 1023, 0, 19);

  lcd.setCursor(0, 2);
  for (int i = 0; i < 20; i++) {
    if (i == posObj && i == posActual) {
      lcd.print("*");   // Coinciden — buen signo
    } else if (i == posObj) {
      lcd.print("X");   // Objetivo
    } else if (i == posActual) {
      lcd.print("|");   // Posición actual
    } else if (i > min(posObj, posActual) && i < max(posObj, posActual)) {
      lcd.print(".");   // Zona entre los dos
    } else {
      lcd.print("-");
    }
  }
}

// ============================================================
//  AUDIO — WTC020
// ============================================================
void reproducirAudio(int numero) {
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

// Llamada desde main.ino cuando la bomba explota
void reproducirExplosion() {
  reproducirAudio(1);  // 0001.ad4
}

// Llamada desde main.ino cuando se gana
void reproducirExito() {
  reproducirAudio(2);  // 0002.ad4
}

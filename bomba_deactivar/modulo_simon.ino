// ============================================================
//  modulo_simon.ino — P2 (Simon Says) y P3 (Serpiente)
//  Los botones y LEDs son compartidos por ambos minijuegos.
//  P2 programa las funciones de Simon Says.
//  P3 programa las funciones de la Serpiente.
// ============================================================

// --- Pines del multiplexor 74HC4051 ---
#define MUX_S0  11   // Bit selección 0
#define MUX_S1  12   // Bit selección 1
#define MUX_S2  13   // Bit selección 2
#define MUX_SIG A2   // Señal compartida entrada/salida

// --- Canales del multiplexor ---
// Botones: Y0=rojo(arriba) Y1=verde(abajo) Y2=azul(izq) Y3=amarillo(der)
// LEDs:    Y4=rojo         Y5=verde        Y6=azul       Y7=amarillo
#define BTN_ROJO    0
#define BTN_VERDE   1
#define BTN_AZUL    2
#define BTN_AMARILLO 3
#define LED_ROJO    4
#define LED_VERDE   5
#define LED_AZUL    6
#define LED_AMARILLO 7

// ============================================================
//  FUNCIONES DEL MULTIPLEXOR
//  Estas las usan tanto Simon como la Serpiente
// ============================================================

// Seleccionar un canal del multiplexor (0-7)
void muxSeleccionar(int canal) {
  digitalWrite(MUX_S0, (canal >> 0) & 1);
  digitalWrite(MUX_S1, (canal >> 1) & 1);
  digitalWrite(MUX_S2, (canal >> 2) & 1);
}

// Encender un LED (canal 4-7)
void encenderLed(int canal) {
  pinMode(MUX_SIG, OUTPUT);
  muxSeleccionar(canal);
  digitalWrite(MUX_SIG, HIGH);
}

// Apagar un LED (canal 4-7)
void apagarLed(int canal) {
  pinMode(MUX_SIG, OUTPUT);
  muxSeleccionar(canal);
  digitalWrite(MUX_SIG, LOW);
}

// Apagar todos los LEDs
void apagarTodosLeds() {
  for (int i = LED_ROJO; i <= LED_AMARILLO; i++) apagarLed(i);
}

// Leer un botón (canal 0-3) — devuelve true si está pulsado
bool leerBoton(int canal) {
  pinMode(MUX_SIG, INPUT);
  muxSeleccionar(canal);
  return digitalRead(MUX_SIG) == HIGH;
}

// Esperar a que se pulse un botón — devuelve cuál (0-3)
int esperarBoton() {
  while (true) {
    for (int i = 0; i < 4; i++) {
      if (leerBoton(i)) {
        delay(50);  // Anti-rebote
        while (leerBoton(i));  // Esperar a que lo suelten
        return i;
      }
    }
  }
}

// ============================================================
//  SIMON SAYS — P2
// ============================================================
#define SIMON_MAX_SECUENCIA 20
int  simonSecuencia[SIMON_MAX_SECUENCIA];
int  simonLongitud  = 0;
int  simonPaso      = 0;
bool simonMostrando = false;
unsigned long simonUltimoTick = 0;

void setupSimon() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  apagarTodosLeds();
}

void iniciarSimon() {
  simonLongitud  = (nivel == 1) ? 3 : (nivel == 2) ? 5 : 7;
  simonPaso      = 0;
  simonMostrando = true;

  // Generar secuencia aleatoria
  for (int i = 0; i < simonLongitud; i++) {
    simonSecuencia[i] = random(0, 4);  // 0=rojo 1=verde 2=azul 3=amarillo
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== SIMON SAYS ==");
  lcd.setCursor(0, 1); lcd.print("Observa la seq...");
  delay(1000);
  mostrarSecuenciaSimon();
}

void loopSimon() {
  if (simonMostrando) return;  // Mientras muestra la secuencia no leer botones

  lcd.setCursor(0, 1); lcd.print("Tu turno!          ");
  lcd.setCursor(0, 2); lcd.print("Paso: ");
  lcd.print(simonPaso + 1);
  lcd.print(" / ");
  lcd.print(simonLongitud);

  int boton = esperarBoton();
  encenderLed(boton + 4);  // Encender LED del color pulsado
  delay(300);
  apagarLed(boton + 4);

  if (boton == simonSecuencia[simonPaso]) {
    simonPaso++;
    if (simonPaso >= simonLongitud) {
      // ¡Secuencia completada!
      lcd.setCursor(0, 3); lcd.print("Correcto!!          ");
      apagarTodosLeds();
      delay(1500);
      simonSuperado = true;
      cambiarEstado(JUEGO_SERPIENTE);
    }
  } else {
    // Error — reintentar
    lcd.setCursor(0, 3); lcd.print("Error! Repite...    ");
    apagarTodosLeds();
    delay(1000);
    simonPaso = 0;
    mostrarSecuenciaSimon();
  }
}

void mostrarSecuenciaSimon() {
  simonMostrando = true;
  int velocidad = (nivel == 1) ? 600 : (nivel == 2) ? 400 : 250;

  for (int i = 0; i < simonLongitud; i++) {
    encenderLed(simonSecuencia[i] + 4);
    delay(velocidad);
    apagarLed(simonSecuencia[i] + 4);
    delay(200);
  }
  simonMostrando = false;
}

// ============================================================
//  SERPIENTE — P3
//  Grid de 20 columnas x 3 filas (fila 1-3 de la LCD, fila 0 = info)
// ============================================================
#define GRID_COLS 20
#define GRID_FILAS 3
#define SERPIENTE_MAX 30

// Posición de cada segmento de la serpiente
int snakeX[SERPIENTE_MAX];
int snakeY[SERPIENTE_MAX];
int snakeLongitud = 3;

// Dirección actual: 0=der 1=izq 2=arr 3=aba
int snakeDirX = 1;  // Movimiento horizontal
int snakeDirY = 0;  // Movimiento vertical

// Posición de la comida
int foodX = 10;
int foodY = 1;

bool snakeViva = true;
unsigned long snakeUltimoMovimiento = 0;
int snakeVelocidad = 500;  // ms entre movimientos

// Caracteres personalizados para la LCD
byte charSnake[8] = { 0x00,0x0E,0x1F,0x1F,0x1F,0x0E,0x00,0x00 }; // cuerpo
byte charFood[8]  = { 0x00,0x04,0x0E,0x1F,0x0E,0x04,0x00,0x00 }; // comida

void setupSerpiente() {
  // Los pines ya los configura setupSimon()
}

void iniciarSerpiente() {
  lcd.createChar(0, charSnake);
  lcd.createChar(1, charFood);

  snakeLongitud = 3;
  snakeDirX = 1;
  snakeDirY = 0;
  snakeViva = true;
  snakeVelocidad = (nivel == 1) ? 600 : (nivel == 2) ? 400 : 250;
  snakeUltimoMovimiento = millis();

  // Posición inicial centrada
  for (int i = 0; i < snakeLongitud; i++) {
    snakeX[i] = 5 - i;
    snakeY[i] = 1;
  }

  // Comida en posición aleatoria
  foodX = random(0, GRID_COLS);
  foodY = random(0, GRID_FILAS);

  dibujarSerpiente();
}

void loopSerpiente() {
  // Leer botones para cambiar dirección
  if (leerBoton(BTN_ROJO)     && snakeDirY != 1)  { snakeDirX = 0;  snakeDirY = -1; } // arriba
  if (leerBoton(BTN_VERDE)    && snakeDirY != -1) { snakeDirX = 0;  snakeDirY = 1;  } // abajo
  if (leerBoton(BTN_AZUL)     && snakeDirX != 1)  { snakeDirX = -1; snakeDirY = 0;  } // izquierda
  if (leerBoton(BTN_AMARILLO) && snakeDirX != -1) { snakeDirX = 1;  snakeDirY = 0;  } // derecha

  // Mover serpiente según velocidad
  if (millis() - snakeUltimoMovimiento < snakeVelocidad) return;
  snakeUltimoMovimiento = millis();

  // Calcular nueva cabeza
  int nuevoCabX = snakeX[0] + snakeDirX;
  int nuevoCabY = snakeY[0] + snakeDirY;

  // Comprobar colisión con paredes
  if (nuevoCabX < 0 || nuevoCabX >= GRID_COLS || nuevoCabY < 0 || nuevoCabY >= GRID_FILAS) {
    snakeViva = false;
    lcd.setCursor(0, 0); lcd.print("Chocaste! Repite...");
    delay(1500);
    iniciarSerpiente();
    return;
  }

  // Comprobar colisión consigo misma
  for (int i = 1; i < snakeLongitud; i++) {
    if (snakeX[i] == nuevoCabX && snakeY[i] == nuevoCabY) {
      snakeViva = false;
      lcd.setCursor(0, 0); lcd.print("Chocaste! Repite...");
      delay(1500);
      iniciarSerpiente();
      return;
    }
  }

  // Mover segmentos hacia atrás
  for (int i = snakeLongitud - 1; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  snakeX[0] = nuevoCabX;
  snakeY[0] = nuevoCabY;

  // Comprobar si comió
  if (nuevoCabX == foodX && nuevoCabY == foodY) {
    snakeLongitud++;
    foodX = random(0, GRID_COLS);
    foodY = random(0, GRID_FILAS);

    // Ganar cuando la serpiente llega a longitud 8
    if (snakeLongitud >= 8) {
      lcd.setCursor(0, 0); lcd.print("Ganaste!!           ");
      delay(1500);
      serpienteSuperado = true;
      cambiarEstado(JUEGO_POT);
      return;
    }
  }

  dibujarSerpiente();
}

void dibujarSerpiente() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Serpiente L:");
  lcd.print(snakeLongitud);

  // Dibujar comida
  lcd.setCursor(foodX, foodY + 1);
  lcd.write(byte(1));

  // Dibujar serpiente
  for (int i = 0; i < snakeLongitud; i++) {
    lcd.setCursor(snakeX[i], snakeY[i] + 1);
    lcd.write(byte(0));
  }
}

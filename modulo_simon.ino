// ============================================================
//  modulo_simon.ino — P2 (Simon Says) + P3 (Serpiente)
//
//  SIMON SAYS:
//  Nivel 1 — secuencia 3 colores, velocidad lenta
//  Nivel 2 — secuencia 5 colores, velocidad media
//  Nivel 3 — secuencia 7 colores, velocidad rápida
//  Nivel 4 — secuencia 9 colores, velocidad muy rápida
//
//  SERPIENTE:
//  Nivel 1 — lenta, ganar con longitud 5
//  Nivel 2 — media, ganar con longitud 7
//  Nivel 3 — rápida, ganar con longitud 9
//  Nivel 4 — muy rápida, ganar con longitud 11
// ============================================================

// --- Pines del multiplexor ---
#define MUX_S0   11
#define MUX_S1   12
#define MUX_S2   13
#define MUX_SIG  A2

// --- Canales multiplexor ---
#define BTN_ROJO      0   // Arriba en serpiente
#define BTN_VERDE     1   // Abajo en serpiente
#define BTN_AZUL      2   // Izquierda en serpiente
#define BTN_AMARILLO  3   // Derecha en serpiente
#define LED_ROJO      4
#define LED_VERDE     5
#define LED_AZUL      6
#define LED_AMARILLO  7

// ============================================================
//  FUNCIONES DEL MULTIPLEXOR — usadas por Simon y Serpiente
// ============================================================
void muxSeleccionar(int canal) {
  digitalWrite(MUX_S0, (canal >> 0) & 1);
  digitalWrite(MUX_S1, (canal >> 1) & 1);
  digitalWrite(MUX_S2, (canal >> 2) & 1);
}

void encenderLed(int canal) {
  pinMode(MUX_SIG, OUTPUT);
  muxSeleccionar(canal);
  digitalWrite(MUX_SIG, HIGH);
}

void apagarLed(int canal) {
  pinMode(MUX_SIG, OUTPUT);
  muxSeleccionar(canal);
  digitalWrite(MUX_SIG, LOW);
}

void apagarTodosLeds() {
  for (int i = LED_ROJO; i <= LED_AMARILLO; i++) apagarLed(i);
}

bool leerBoton(int canal) {
  pinMode(MUX_SIG, INPUT);
  muxSeleccionar(canal);
  delayMicroseconds(10);  // Pequeña espera para estabilizar
  return digitalRead(MUX_SIG) == HIGH;
}

// Esperar a que se pulse algún botón — devuelve cuál (0-3)
int esperarBoton() {
  while (true) {
    for (int i = 0; i < 4; i++) {
      if (leerBoton(i)) {
        delay(50);               // Anti-rebote
        while (leerBoton(i));    // Esperar que lo suelten
        return i;
      }
    }
  }
}

// Leer botón sin bloquear — devuelve -1 si ninguno pulsado
int leerBotonNoBloquear() {
  for (int i = 0; i < 4; i++) {
    if (leerBoton(i)) {
      delay(50);
      while (leerBoton(i));
      return i;
    }
  }
  return -1;
}

// ============================================================
//  SIMON SAYS — P2
// ============================================================
#define SIMON_MAX 20
int  simonSecuencia[SIMON_MAX];
int  simonLongitud  = 3;
int  simonPaso      = 0;
int  simonVelocidad = 600;  // ms por color al mostrar

const char* nombreColores[4] = {"ROJO", "VERDE", "AZUL", "AMAR"};

void setupSimon() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  apagarTodosLeds();
}

void iniciarSimon() {
  // Configurar según nivel
  switch (nivel) {
    case 1: simonLongitud = 3; simonVelocidad = 700; break;
    case 2: simonLongitud = 5; simonVelocidad = 500; break;
    case 3: simonLongitud = 7; simonVelocidad = 350; break;
    case 4: simonLongitud = 9; simonVelocidad = 200; break;
  }
  simonPaso = 0;

  // Generar secuencia aleatoria
  for (int i = 0; i < simonLongitud; i++) {
    simonSecuencia[i] = random(0, 4);
  }

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== SIMON SAYS ==");
  lcd.setCursor(0, 1); lcd.print("Secuencia: ");
  lcd.print(simonLongitud);
  lcd.print(" colores");
  lcd.setCursor(0, 2); lcd.print("Observa bien...");
  delay(1500);

  mostrarSecuenciaSimon();
  mostrarTurnoJugador();
}

void loopSimon() {
  int boton = leerBotonNoBloquear();
  if (boton == -1) return;

  // Parpadear LED del botón pulsado para feedback visual
  encenderLed(boton + LED_ROJO);
  delay(200);
  apagarLed(boton + LED_ROJO);

  if (boton == simonSecuencia[simonPaso]) {
    simonPaso++;
    if (simonPaso >= simonLongitud) {
      // ¡Secuencia completada!
      // Celebración: parpadear todos los LEDs
      for (int v = 0; v < 3; v++) {
        for (int i = LED_ROJO; i <= LED_AMARILLO; i++) encenderLed(i);
        delay(200);
        apagarTodosLeds();
        delay(200);
      }
      lcd.setCursor(0, 3); lcd.print("Correcto!!! Genial! ");
      delay(1500);
      simonSuperado = true;
      cambiarEstado(JUEGO_SERPIENTE);
    } else {
      // Correcto pero faltan más
      lcd.setCursor(0, 3);
      lcd.print("Paso ");
      lcd.print(simonPaso);
      lcd.print("/");
      lcd.print(simonLongitud);
      lcd.print(" OK!      ");
    }
  } else {
    // Error — reiniciar desde el principio
    apagarTodosLeds();
    lcd.setCursor(0, 3); lcd.print("Error! Desde el 1  ");
    delay(1500);
    simonPaso = 0;
    mostrarSecuenciaSimon();
    mostrarTurnoJugador();
  }
}

void mostrarSecuenciaSimon() {
  for (int i = 0; i < simonLongitud; i++) {
    int led = simonSecuencia[i] + LED_ROJO;
    encenderLed(led);

    // Mostrar nombre del color en LCD
    lcd.setCursor(0, 3);
    lcd.print(">> ");
    lcd.print(nombreColores[simonSecuencia[i]]);
    lcd.print("            ");

    delay(simonVelocidad);
    apagarLed(led);
    delay(150);
  }
}

void mostrarTurnoJugador() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== SIMON SAYS ==");
  lcd.setCursor(0, 1); lcd.print("Tu turno!");
  lcd.setCursor(0, 2); lcd.print("Repite la secuencia");
  lcd.setCursor(0, 3); lcd.print("Paso 0/");
  lcd.print(simonLongitud);
}

// ============================================================
//  SERPIENTE — P3
//  Grid: 20 columnas x 3 filas (filas 1-3 de la LCD)
//  Fila 0 = información (longitud y tiempo)
// ============================================================
#define GRID_COLS  20
#define GRID_FILAS  3
#define SNAKE_MAX  40

int snakeX[SNAKE_MAX];
int snakeY[SNAKE_MAX];
int snakeLongitud   = 3;
int snakeLongitudMax = 5;  // Longitud para ganar
int snakeDirX       = 1;
int snakeDirY       = 0;
int foodX = 10, foodY = 1;
unsigned long snakeUltimoMov = 0;
int  snakeVelocidad = 500;

// Caracteres personalizados LCD
byte charCuerpo[8] = {0x00,0x0E,0x1F,0x1F,0x1F,0x0E,0x00,0x00};
byte charComida[8] = {0x04,0x0E,0x1F,0x0E,0x04,0x00,0x00,0x00};
byte charCabeza[8] = {0x00,0x1F,0x15,0x1F,0x1F,0x0E,0x00,0x00};

void setupSerpiente() {
  // Los pines ya los configura setupSimon()
}

void iniciarSerpiente() {
  // Configurar según nivel
  switch (nivel) {
    case 1: snakeVelocidad = 600; snakeLongitudMax = 5;  break;
    case 2: snakeVelocidad = 400; snakeLongitudMax = 7;  break;
    case 3: snakeVelocidad = 250; snakeLongitudMax = 9;  break;
    case 4: snakeVelocidad = 150; snakeLongitudMax = 11; break;
  }

  // Crear caracteres personalizados
  lcd.createChar(0, charCuerpo);
  lcd.createChar(1, charComida);
  lcd.createChar(2, charCabeza);

  // Posición inicial
  snakeLongitud = 3;
  snakeDirX = 1;
  snakeDirY = 0;
  for (int i = 0; i < snakeLongitud; i++) {
    snakeX[i] = 4 - i;
    snakeY[i] = 1;
  }
  snakeUltimoMov = millis();

  // Comida en posición aleatoria (que no coincida con la serpiente)
  generarComida();
  dibujarSerpiente();
}

void loopSerpiente() {
  // Leer dirección — sin bloquear
  if (leerBoton(BTN_ROJO)     && snakeDirY != 1)  { snakeDirX = 0;  snakeDirY = -1; }
  if (leerBoton(BTN_VERDE)    && snakeDirY != -1) { snakeDirX = 0;  snakeDirY = 1;  }
  if (leerBoton(BTN_AZUL)     && snakeDirX != 1)  { snakeDirX = -1; snakeDirY = 0;  }
  if (leerBoton(BTN_AMARILLO) && snakeDirX != -1) { snakeDirX = 1;  snakeDirY = 0;  }

  // Mover según velocidad del nivel
  if (millis() - snakeUltimoMov < (unsigned long)snakeVelocidad) return;
  snakeUltimoMov = millis();

  // Nueva posición de la cabeza
  int nuevoX = snakeX[0] + snakeDirX;
  int nuevoY = snakeY[0] + snakeDirY;

  // Colisión con paredes
  if (nuevoX < 0 || nuevoX >= GRID_COLS || nuevoY < 0 || nuevoY >= GRID_FILAS) {
    lcd.setCursor(0, 0); lcd.print("Chocaste! Repite... ");
    delay(1500);
    iniciarSerpiente();
    return;
  }

  // Colisión consigo misma
  for (int i = 1; i < snakeLongitud; i++) {
    if (snakeX[i] == nuevoX && snakeY[i] == nuevoY) {
      lcd.setCursor(0, 0); lcd.print("Colision! Repite... ");
      delay(1500);
      iniciarSerpiente();
      return;
    }
  }

  // Mover cuerpo
  for (int i = min(snakeLongitud, SNAKE_MAX - 1); i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  snakeX[0] = nuevoX;
  snakeY[0] = nuevoY;

  // ¿Comió?
  if (nuevoX == foodX && nuevoY == foodY) {
    if (snakeLongitud < SNAKE_MAX) snakeLongitud++;
    generarComida();

    if (snakeLongitud >= snakeLongitudMax) {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Serpiente ganada!   ");
      lcd.setCursor(0, 1); lcd.print("Longitud: ");
      lcd.print(snakeLongitud);
      delay(1500);
      serpienteSuperado = true;
      cambiarEstado(JUEGO_POT);
      return;
    }
  }

  dibujarSerpiente();
}

void generarComida() {
  bool valido = false;
  while (!valido) {
    foodX = random(0, GRID_COLS);
    foodY = random(0, GRID_FILAS);
    valido = true;
    for (int i = 0; i < snakeLongitud; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        valido = false;
        break;
      }
    }
  }
}

void dibujarSerpiente() {
  lcd.clear();

  // Fila 0: info
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(snakeLongitud);
  lcd.print("/");
  lcd.print(snakeLongitudMax);
  // El tiempo lo pinta main.ino en setCursor(13,0)

  // Dibujar comida
  lcd.setCursor(foodX, foodY + 1);
  lcd.write(byte(1));

  // Dibujar serpiente (cabeza distinta al cuerpo)
  for (int i = 0; i < snakeLongitud; i++) {
    lcd.setCursor(snakeX[i], snakeY[i] + 1);
    lcd.write(byte(i == 0 ? 2 : 0));
  }
}

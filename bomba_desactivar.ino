// ============================================================
//  DESACTIVAR LA BOMBA — main.ino
//  4 niveles de dificultad. Siempre aparecen los 4 minijuegos.
//  Nivel 1 = fácil, Nivel 4 = muy difícil y menos tiempo.
// ============================================================

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// --- Pantalla LCD compartida ---
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ============================================================
//  ESTADOS DEL JUEGO
// ============================================================
enum Estado {
  MENU,
  JUEGO_MATES,
  JUEGO_SIMON,
  JUEGO_SERPIENTE,
  JUEGO_POT,
  BOMBA_EXPLOTA,
  GANASTE
};

Estado estadoActual = MENU;

// ============================================================
//  NIVEL Y TIEMPO
//  Cada nivel tiene un tiempo distinto
// ============================================================
int nivel = 1;

// Tiempo por nivel en milisegundos
const unsigned long TIEMPOS_NIVEL[4] = {
  180000,  // Nivel 1 — 3 minutos
  150000,  // Nivel 2 — 2:30 minutos
  120000,  // Nivel 3 — 2 minutos
   90000   // Nivel 4 — 1:30 minutos
};

unsigned long tiempoInicio   = 0;
unsigned long duracionTotal  = 180000;
unsigned long tiempoRestante = 0;

// ============================================================
//  PROGRESO
// ============================================================
bool matesSuperado      = false;
bool simonSuperado      = false;
bool serpienteSuperado  = false;
bool potSuperado        = false;

// ============================================================
//  MENÚ — navegación con el teclado
//  Arriba = tecla 2, Abajo = tecla 8, Confirmar = tecla #
// ============================================================
int menuCursor = 0;  // 0-3 = niveles 1-4

// ============================================================
//  setup()
// ============================================================
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  setupTeclado();
  setupSimon();
  setupSerpiente();
  setupPotAudio();

  mostrarMenu();
}

// ============================================================
//  loop()
// ============================================================
void loop() {

  // Actualizar temporizador durante el juego
  if (estadoActual != MENU &&
      estadoActual != BOMBA_EXPLOTA &&
      estadoActual != GANASTE) {

    tiempoRestante = duracionTotal - (millis() - tiempoInicio);

    if ((long)tiempoRestante <= 0) {
      cambiarEstado(BOMBA_EXPLOTA);
      return;
    }
    mostrarTiempo();
  }

  // Delegar en el módulo activo
  switch (estadoActual) {
    case MENU:            loopMenu();        break;
    case JUEGO_MATES:     loopTeclado();     break;
    case JUEGO_SIMON:     loopSimon();       break;
    case JUEGO_SERPIENTE: loopSerpiente();   break;
    case JUEGO_POT:       loopPotAudio();    break;
    case BOMBA_EXPLOTA:   loopBomba();       break;
    case GANASTE:         loopGanaste();     break;
  }
}

// ============================================================
//  MENÚ — selección de nivel con teclado
// ============================================================
void mostrarMenu() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== DESACTIVA BOMBA==");
  lcd.setCursor(0, 1); lcd.print("2=arriba 8=abajo    ");
  lcd.setCursor(0, 2); lcd.print("# para empezar      ");
  actualizarCursorMenu();
}

void actualizarCursorMenu() {
  // Borrar las 4 opciones y redibujar con cursor
  const char* etiquetas[4] = {
    "Nivel 1 - Facil    ",
    "Nivel 2 - Medio    ",
    "Nivel 3 - Dificil  ",
    "Nivel 4 - Extremo  "
  };
  // Solo mostramos la opción seleccionada en fila 3
  lcd.setCursor(0, 3);
  lcd.print(">");
  lcd.print(etiquetas[menuCursor]);
}

void loopMenu() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  if (tecla == '2') {
    // Arriba
    menuCursor = (menuCursor - 1 + 4) % 4;
    actualizarCursorMenu();
  } else if (tecla == '8') {
    // Abajo
    menuCursor = (menuCursor + 1) % 4;
    actualizarCursorMenu();
  } else if (tecla == '#') {
    // Confirmar nivel
    nivel = menuCursor + 1;
    duracionTotal = TIEMPOS_NIVEL[menuCursor];
    tiempoInicio  = millis();

    // Reiniciar progreso
    matesSuperado     = false;
    simonSuperado     = false;
    serpienteSuperado = false;
    potSuperado       = false;

    cambiarEstado(JUEGO_MATES);
  }
}

// ============================================================
//  CAMBIAR DE ESTADO
// ============================================================
void cambiarEstado(Estado nuevoEstado) {
  estadoActual = nuevoEstado;
  lcd.clear();

  switch (nuevoEstado) {
    case JUEGO_MATES:      iniciarTeclado();    break;
    case JUEGO_SIMON:      iniciarSimon();      break;
    case JUEGO_SERPIENTE:  iniciarSerpiente();  break;
    case JUEGO_POT:        iniciarPotAudio();   break;
    case BOMBA_EXPLOTA:    iniciarExplosion();  break;
    case GANASTE:          iniciarGanaste();    break;
    default: break;
  }
}

// ============================================================
//  TEMPORIZADOR — esquina superior derecha
// ============================================================
void mostrarTiempo() {
  int segundos = tiempoRestante / 1000;
  int min = segundos / 60;
  int seg = segundos % 60;
  lcd.setCursor(13, 0);
  if (min < 10) lcd.print("0");
  lcd.print(min);
  lcd.print(":");
  if (seg < 10) lcd.print("0");
  lcd.print(seg);
}

// ============================================================
//  PANTALLAS FINALES
// ============================================================
void iniciarExplosion() {
  reproducirExplosion();
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("*** BOOM ***");
  lcd.setCursor(2, 1); lcd.print("La bomba exploto!");
  lcd.setCursor(0, 3); lcd.print("# para volver menu");
}

void loopBomba() {
  char tecla = teclado.getKey();
  if (tecla == '#') {
    menuCursor = 0;
    cambiarEstado(MENU);
    mostrarMenu();
  }
}

void iniciarGanaste() {
  reproducirExito();
  lcd.clear();
  lcd.setCursor(1, 0); lcd.print("** BOMBA DESACT! **");
  lcd.setCursor(3, 1); lcd.print("Felicidades!!!");
  lcd.setCursor(0, 2); lcd.print("Nivel ");
  lcd.print(nivel);
  lcd.print(" completado!");
  lcd.setCursor(0, 3); lcd.print("# para volver menu");
}

void loopGanaste() {
  char tecla = teclado.getKey();
  if (tecla == '#') {
    menuCursor = 0;
    cambiarEstado(MENU);
    mostrarMenu();
  }
}

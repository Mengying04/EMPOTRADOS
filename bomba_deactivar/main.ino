// ============================================================
//  DESACTIVAR LA BOMBA — main.ino
//  Archivo principal. Coordina todos los módulos.
//  Cada persona tiene su propio archivo .ino aparte.
// ============================================================

// --- Librerías ---
#include <LiquidCrystal_I2C.h>   // Pantalla LCD
#include <Keypad.h>               // Teclado matricial

// --- Incluir los módulos de cada persona ---
// (Arduino compila automáticamente todos los .ino de la misma carpeta)
// modulo_teclado.ino       → P1
// modulo_simon.ino         → P2
// modulo_serpiente.ino     → P3
// modulo_pot_audio.ino     → P4

// ============================================================
//  PANTALLA LCD — compartida por todos
// ============================================================
// Dirección I2C: 0x27 (si no funciona probar 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ============================================================
//  ESTADOS DEL JUEGO
//  El juego siempre está en uno de estos estados.
//  Según el estado, el loop() hace una cosa u otra.
// ============================================================
enum Estado {
  MENU,           // Pantalla de inicio / selección de nivel
  JUEGO_MATES,    // Minijuego 1 — operación matemática
  JUEGO_SIMON,    // Minijuego 2 — Simon Says
  JUEGO_SERPIENTE,// Minijuego 3 — Serpiente
  JUEGO_POT,      // Minijuego 4 — Potenciómetro
  BOMBA_EXPLOTA,  // Tiempo agotado o fallo — game over
  GANASTE         // Todos los minijuegos superados
};

Estado estadoActual = MENU;  // Empezamos en el menú

// ============================================================
//  TEMPORIZADOR GLOBAL
//  Cuenta atrás compartida. Si llega a 0 → BOMBA_EXPLOTA.
//  Usa millis() en vez de delay() para no bloquear el programa.
// ============================================================
unsigned long tiempoInicio    = 0;   // Momento en que arrancó el juego
unsigned long duracionTotal   = 120000; // 2 minutos en milisegundos
unsigned long tiempoRestante  = 0;   // Se actualiza cada ciclo

// ============================================================
//  PROGRESO — qué minijuegos se han superado
// ============================================================
bool matesSuperado     = false;
bool simonSuperado     = false;
bool serpienteSuperado = false;
bool potSuperado       = false;

// ============================================================
//  NIVEL DE DIFICULTAD (elegido en el menú)
// ============================================================
int nivel = 1;  // 1 = fácil, 2 = medio, 3 = difícil

// ============================================================
//  setup() — se ejecuta UNA SOLA VEZ al encender el Arduino
// ============================================================
void setup() {
  Serial.begin(9600);      // Para depuración por consola
  lcd.init();              // Inicializar pantalla
  lcd.backlight();         // Encender luz de fondo

  // Inicializar cada módulo (función definida en su archivo)
  setupTeclado();
  setupSimon();
  setupSerpiente();
  setupPotAudio();

  mostrarMenu();           // Mostrar pantalla de inicio
}

// ============================================================
//  loop() — se ejecuta EN BUCLE infinito
//  Según el estado actual, delega en el módulo correspondiente
// ============================================================
void loop() {

  // --- Actualizar temporizador (solo durante el juego) ---
  if (estadoActual != MENU && estadoActual != BOMBA_EXPLOTA && estadoActual != GANASTE) {
    tiempoRestante = duracionTotal - (millis() - tiempoInicio);

    if (tiempoRestante <= 0) {
      cambiarEstado(BOMBA_EXPLOTA);
      return;
    }

    mostrarTiempo();  // Actualizar tiempo en pantalla
  }

  // --- Delegar en el módulo activo ---
  switch (estadoActual) {
    case MENU:
      loopMenu();
      break;
    case JUEGO_MATES:
      loopTeclado();
      break;
    case JUEGO_SIMON:
      loopSimon();
      break;
    case JUEGO_SERPIENTE:
      loopSerpiente();
      break;
    case JUEGO_POT:
      loopPotAudio();
      break;
    case BOMBA_EXPLOTA:
      loopBomba();
      break;
    case GANASTE:
      loopGanaste();
      break;
  }
}

// ============================================================
//  FUNCIONES COMPARTIDAS
//  Cualquier módulo puede llamar a estas funciones
// ============================================================

// Cambiar de estado — llamar desde cualquier módulo al terminar
void cambiarEstado(Estado nuevoEstado) {
  estadoActual = nuevoEstado;
  lcd.clear();

  if (nuevoEstado == JUEGO_MATES)     iniciarTeclado();
  if (nuevoEstado == JUEGO_SIMON)     iniciarSimon();
  if (nuevoEstado == JUEGO_SERPIENTE) iniciarSerpiente();
  if (nuevoEstado == JUEGO_POT)       iniciarPotAudio();
  if (nuevoEstado == BOMBA_EXPLOTA)   iniciarExplosion();
  if (nuevoEstado == GANASTE)         iniciarGanaste();
}

// Mostrar tiempo restante en la esquina de la LCD
void mostrarTiempo() {
  int segundos = tiempoRestante / 1000;
  int min = segundos / 60;
  int seg = segundos % 60;
  lcd.setCursor(13, 0);  // Esquina superior derecha
  if (min < 10) lcd.print("0");
  lcd.print(min);
  lcd.print(":");
  if (seg < 10) lcd.print("0");
  lcd.print(seg);
}

// Mostrar menú inicial
void mostrarMenu() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== DESACTIVA LA ==");
  lcd.setCursor(0, 1); lcd.print("==    BOMBA     ==");
  lcd.setCursor(0, 2); lcd.print("Nivel: ");
  lcd.print(nivel);
  lcd.setCursor(0, 3); lcd.print("* para empezar");
}

// Loop del menú — espera que el jugador pulse * para empezar
void loopMenu() {
  // Esta función está en modulo_teclado.ino
  // El teclado detecta la tecla y llama a:
  //   tiempoInicio = millis();
  //   cambiarEstado(JUEGO_MATES);
}

// Pantalla de explosión — game over
void loopBomba() {
  // La función de sonido está en modulo_pot_audio.ino
  // reproductorExplosion() ya se llamó en iniciarExplosion()
  lcd.setCursor(0, 0); lcd.print("*** BOOM ***");
  lcd.setCursor(0, 1); lcd.print("La bomba exploto");
  lcd.setCursor(0, 3); lcd.print("# para reiniciar");
  // Esperar tecla # para volver al menú
}

// Pantalla de victoria
void loopGanaste() {
  lcd.setCursor(0, 0); lcd.print("** BOMBA DESACT **");
  lcd.setCursor(0, 1); lcd.print("Felicidades!!!");
  lcd.setCursor(0, 3); lcd.print("# para reiniciar");
}

void iniciarExplosion() {
  reproducirExplosion();  // Definida en modulo_pot_audio.ino
  lcd.clear();
}

void iniciarGanaste() {
  lcd.clear();
}

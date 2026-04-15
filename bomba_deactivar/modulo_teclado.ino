// ============================================================
//  modulo_teclado.ino — P1
//  Minijuego: operación matemática con teclado 4x4
// ============================================================

#include <Keypad.h>

// --- Configuración del teclado 4x4 ---
const byte FILAS = 4;
const byte COLS  = 4;

// Mapa de teclas — coincide con la serigrafía del teclado
char teclas[FILAS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pines según el mapa definitivo del proyecto
byte pinesFilas[FILAS] = {A0, A1, 2, 3};   // R1, R2, R3, R4
byte pinesCols[COLS]   = {4, 5, 6, 7};      // C1, C2, C3, C4

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesCols, FILAS, COLS);

// --- Variables del minijuego ---
int  numA         = 0;    // Primer número de la operación
int  numB         = 0;    // Segundo número de la operación
int  respuestaCorrecta = 0;
String respuestaUsuario = "";
char operacion    = '+';  // Operación actual: +, -, *

// ============================================================
//  setupTeclado() — llamada desde main.ino una sola vez
// ============================================================
void setupTeclado() {
  // El teclado no necesita pinMode() manual,
  // la librería Keypad.h lo configura sola
}

// ============================================================
//  iniciarTeclado() — llamada cuando empieza este minijuego
// ============================================================
void iniciarTeclado() {
  respuestaUsuario = "";
  generarOperacion();
  mostrarOperacionEnLCD();
}

// ============================================================
//  loopTeclado() — llamada en cada ciclo del loop() principal
// ============================================================
void loopTeclado() {
  char tecla = teclado.getKey();  // Devuelve '\0' si no hay tecla

  if (tecla) {
    if (tecla >= '0' && tecla <= '9') {
      // El usuario está escribiendo un número
      respuestaUsuario += tecla;
      actualizarRespuestaEnLCD();

    } else if (tecla == '#') {
      // # = confirmar respuesta
      int respuesta = respuestaUsuario.toInt();

      if (respuesta == respuestaCorrecta) {
        // ¡Correcto!
        lcd.setCursor(0, 3);
        lcd.print("Correcto! Bien!     ");
        delay(1500);
        matesSuperado = true;              // Variable de main.ino
        cambiarEstado(JUEGO_SIMON);        // Pasar al siguiente juego

      } else {
        // Incorrecto — mostrar error y reintentar
        lcd.setCursor(0, 3);
        lcd.print("Mal! Era: ");
        lcd.print(respuestaCorrecta);
        lcd.print("     ");
        delay(1500);
        respuestaUsuario = "";
        generarOperacion();
        mostrarOperacionEnLCD();
      }

    } else if (tecla == '*') {
      // * = borrar último dígito
      if (respuestaUsuario.length() > 0) {
        respuestaUsuario.remove(respuestaUsuario.length() - 1);
        actualizarRespuestaEnLCD();
      }
    }
  }
}

// ============================================================
//  FUNCIONES INTERNAS
// ============================================================

void generarOperacion() {
  // Según el nivel, números más grandes o difíciles
  if (nivel == 1) {
    numA = random(1, 10);
    numB = random(1, 10);
    operacion = '+';
  } else if (nivel == 2) {
    numA = random(1, 20);
    numB = random(1, 20);
    operacion = (random(2) == 0) ? '+' : '-';
    if (operacion == '-' && numB > numA) {
      int tmp = numA; numA = numB; numB = tmp;  // evitar negativos
    }
  } else {
    numA = random(2, 10);
    numB = random(2, 10);
    operacion = (random(2) == 0) ? '+' : '*';
  }

  // Calcular respuesta correcta
  if (operacion == '+') respuestaCorrecta = numA + numB;
  if (operacion == '-') respuestaCorrecta = numA - numB;
  if (operacion == '*') respuestaCorrecta = numA * numB;
}

void mostrarOperacionEnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("== OPERACION ==");
  lcd.setCursor(0, 1);
  lcd.print(numA);
  lcd.print(" ");
  lcd.print(operacion);
  lcd.print(" ");
  lcd.print(numB);
  lcd.print(" = ?");
  lcd.setCursor(0, 2); lcd.print("Tu respuesta: ");
  lcd.setCursor(0, 3); lcd.print("# confirmar  * borrar");
}

void actualizarRespuestaEnLCD() {
  lcd.setCursor(14, 2);
  lcd.print("      ");  // Borrar espacio anterior
  lcd.setCursor(14, 2);
  lcd.print(respuestaUsuario);
}

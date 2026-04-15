// ============================================================
//  modulo_teclado.ino — P1
//  Minijuego: operación matemática con teclado 4x4
//
//  Nivel 1 — sumas simples, números 1-10
//  Nivel 2 — sumas y restas, números 1-20
//  Nivel 3 — multiplicaciones y restas, números 1-20
//  Nivel 4 — operaciones encadenadas (a op b op c), números 1-15
// ============================================================

#include <Keypad.h>

// --- Configuración del teclado ---
const byte FILAS = 4;
const byte COLS  = 4;

char teclas[FILAS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinesFilas[FILAS] = {A0, A1, 2, 3};
byte pinesCols[COLS]   = {4, 5, 6, 7};

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesCols, FILAS, COLS);

// --- Variables del minijuego ---
int    numA = 0, numB = 0, numC = 0;
char   opA = '+', opB = '+';
int    respuestaCorrecta = 0;
String respuestaUsuario  = "";
bool   negativo = false;  // Para nivel 4 con resultado negativo posible

// ============================================================
//  Setup e inicio
// ============================================================
void setupTeclado() {
  // Keypad.h configura los pines solo
}

void iniciarTeclado() {
  respuestaUsuario = "";
  negativo = false;
  generarOperacion();
  mostrarOperacionEnLCD();
}

// ============================================================
//  Loop — llamado desde main cada ciclo
// ============================================================
void loopTeclado() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  if (tecla >= '0' && tecla <= '9') {
    if (respuestaUsuario.length() < 4) {  // Máximo 4 dígitos
      respuestaUsuario += tecla;
      actualizarRespuestaEnLCD();
    }

  } else if (tecla == 'A') {
    // Tecla A = signo negativo (para niveles difíciles)
    if (respuestaUsuario.length() == 0) {
      negativo = !negativo;
      actualizarRespuestaEnLCD();
    }

  } else if (tecla == '*') {
    // Borrar último carácter
    if (respuestaUsuario.length() > 0) {
      respuestaUsuario.remove(respuestaUsuario.length() - 1);
    } else {
      negativo = false;
    }
    actualizarRespuestaEnLCD();

  } else if (tecla == '#') {
    // Confirmar respuesta
    int respuesta = respuestaUsuario.toInt();
    if (negativo) respuesta = -respuesta;

    if (respuesta == respuestaCorrecta) {
      lcd.setCursor(0, 3);
      lcd.print("Correcto! Bien!     ");
      delay(1500);
      matesSuperado = true;
      cambiarEstado(JUEGO_SIMON);
    } else {
      lcd.setCursor(0, 3);
      lcd.print("Mal! Era: ");
      lcd.print(respuestaCorrecta);
      lcd.print("      ");
      delay(2000);
      respuestaUsuario = "";
      negativo = false;
      generarOperacion();
      mostrarOperacionEnLCD();
    }
  }
}

// ============================================================
//  Generar operación según nivel
// ============================================================
void generarOperacion() {
  switch (nivel) {

    case 1:
      // Solo sumas, números 1-10
      numA = random(1, 11);
      numB = random(1, 11);
      opA  = '+';
      respuestaCorrecta = numA + numB;
      break;

    case 2:
      // Sumas y restas, números 1-20, resultado siempre positivo
      numA = random(1, 21);
      numB = random(1, 21);
      opA  = (random(2) == 0) ? '+' : '-';
      if (opA == '-' && numB > numA) {
        int tmp = numA; numA = numB; numB = tmp;
      }
      respuestaCorrecta = (opA == '+') ? numA + numB : numA - numB;
      break;

    case 3:
      // Multiplicaciones y restas, números 1-20
      // Resultado puede ser negativo
      numA = random(1, 21);
      numB = random(1, 21);
      opA  = (random(2) == 0) ? '*' : '-';
      respuestaCorrecta = (opA == '*') ? numA * numB : numA - numB;
      break;

    case 4:
      // Operación encadenada: a op b op c
      // Números 1-15, cualquier combinación
      numA = random(1, 16);
      numB = random(1, 16);
      numC = random(1, 16);
      {
        char ops[3] = {'+', '-', '*'};
        opA = ops[random(3)];
        opB = ops[random(3)];
        int parcial;
        if      (opA == '+') parcial = numA + numB;
        else if (opA == '-') parcial = numA - numB;
        else                 parcial = numA * numB;
        if      (opB == '+') respuestaCorrecta = parcial + numC;
        else if (opB == '-') respuestaCorrecta = parcial - numC;
        else                 respuestaCorrecta = parcial * numC;
      }
      break;
  }
}

// ============================================================
//  Mostrar operación en LCD
// ============================================================
void mostrarOperacionEnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("== MATEMATICAS ==");

  lcd.setCursor(0, 1);
  if (nivel < 4) {
    // Operación simple: a op b = ?
    lcd.print(numA);
    lcd.print(" ");
    lcd.print(opA);
    lcd.print(" ");
    lcd.print(numB);
    lcd.print(" = ?");
  } else {
    // Operación encadenada: a op b op c = ?
    lcd.print(numA);
    lcd.print(opA);
    lcd.print(numB);
    lcd.print(opB);
    lcd.print(numC);
    lcd.print("=?");
  }

  lcd.setCursor(0, 2);
  lcd.print("Respuesta: ");

  lcd.setCursor(0, 3);
  if (nivel >= 3) {
    lcd.print("A=+/-  *=borrar  #=ok");
  } else {
    lcd.print("*=borrar      #=ok");
  }
}

void actualizarRespuestaEnLCD() {
  lcd.setCursor(11, 2);
  lcd.print("         ");  // Borrar espacio
  lcd.setCursor(11, 2);
  if (negativo) lcd.print("-");
  lcd.print(respuestaUsuario);
}

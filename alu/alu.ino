// =====================================================================
//  ALU del AGC - Proyecto 1
//  Autora: Samantha Naomi Vital Flores
//  Matrícula: 241050180
// =====================================================================
// Funciona:
// - Hace la suma (AD) y la resta / negacion (CS) del AGC
// - Recibe A y M y regresa el resultado
// Midió:
// - Resultado de AD y CS con valores de prueba
// - Se imprimen en Monitor Serie a 115200
// Argumentó:
// - La ALU es la calculadora de la CPU, sin ella la CPU no puede sumar
//   el programa de prueba de 7 sumas
// =====================================================================

typedef int16_t palabra_t;

// ---------------- ALU ----------------
palabra_t alu(palabra_t a, palabra_t m, uint8_t op) {
  if (op == 2) return -m;      // CS: A = -M
  if (op == 3) return a + m;   // AD: A = A + M
  return a;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("ALU lista - tabla de operaciones");

  // Tabla de operaciones
  Serial.println("OP | Operacion | Ejemplo | Resultado");
  Serial.printf("AD | A = A + M | 20 + 10 = %d\n", alu(20, 10, 3));
  Serial.printf("CS | A = -M    | - (10) = %d\n", alu(0, 10, 2));
  Serial.printf("AD | 0 + 7 sumas de prueba = %d\n", alu(0, 7, 3));

  // Prueba del programa del AGC: suma 7 veces
  palabra_t A = 0;
  for (int i = 0; i < 7; i++) {
    A = alu(A, 1, 3);
    Serial.printf("Paso %d: A = %d\n", i+1, A);
  }
  Serial.println("ALU OK: despues de 7 sumas, A debe ser 7");
}

void loop() {}

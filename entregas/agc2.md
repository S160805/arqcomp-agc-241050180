AGC2 - Semana 7 - La CPU: ciclo fetch-decode-execute y tu propio ISA

Nombre: Samantha Naomi Vital Flores
Matrícula: 241050180
Grupo: IT5
Materia: Arquitectura de Computadoras - TecNM Pabellón de Arteaga
Fecha: 12 al 16 de octubre del 2026
Herramientas: Wokwi.com / ESP32

Video de la evidencia: [Ver video Semana 7](https://drive.google.com/file/d/1GUYHSkxgvxjbP2IF7C9zdWtdYooOFGwB/view?usp=drive_link)
Documento PDF: LAB7SamanthaVital.pdf [Ver PDF Semana 7] (https://drive.google.com/file/d/1iQguNe6Bq5_XIDl62wPKdrImQKHLhDht/view?usp=drive_link)

Objetivo

Implementar y comprobar el ciclo fetch-decode-execute de una CPU de 16 bits, 
utilizar un conjunto propio de instrucciones (ISA), ejecutar un programa de suma y verificar 
tanto el funcionamiento del DSKY como la detección de errores de memoria.

Desarrollo

1. Formato de instrucción e ISA

La palabra es de 16 bits: los 4 bits altos son el opcode y los 12 bits bajos son la dirección. 
La función `codificar` arma la palabra con `op` desplazado 12 bits unido a `dir`, y `decodificar` separa ambos campos 
usando un entero sin signo para que el desplazamiento no extienda el signo.

| Opcode | Mnemónico | Operación | Modifica |
| --- | --- | --- | --- |
| 0 | TC dir | PC = dir; Q = PC anterior + 1 | PC, Q |
| 1 | CA dir | A = M[dir] | A |
| 2 | CS dir | A = -M[dir] | A |
| 3 | AD dir | A = A + M[dir] | A |
| 4 | TS dir | M[dir] = A | Memoria |
| 5 | XCH dir | Intercambia A y M[dir] | A, Memoria |
| 6 | BZF dir | Si A == 0 entonces PC = dir | PC |
| 7 | HALT | Detiene la CPU | Estado |

- En el AGC, TC (Transfer Control / Salto) guardaba en el registro Q la dirección de retorno (PC_anterior + 1)
antes de cambiar el PC al nuevo destino. Servía para poder regresar de una subrutina, equivalente a un return.
- Con 12 bits de dirección solo se alcanzan las direcciones 0x000 a 0xFFF, así que ninguna instrucción podría apuntar a 0x1000
 si la ROM empezara en 0x1000.

2. Mapa de memoria

Se conserva el mapa del Lab 6, sin traslapes:

- RAM: inicio 0x000, tamaño 64 (0x000 - 0x03F)
- E/S: inicio 0x040, tamaño 16 (0x040 - 0x04F), inmediatamente después de la RAM
- ROM: inicio 0x100, tamaño 64 (0x100 - 0x13F)
- PROG_INICIO: 0x110 (ROM_INICIO + 0x10), dentro de la ROM
- PRUEBAS_INICIO: 0x120 (ROM_INICIO + 0x20)

Registros de E/S (offset en IO): IO_JOY_X 0, IO_JOY_Y 1, IO_JOY_SW 2, IO_TICKS 3 (solo lectura), IO_BOTONES 12.

3. CPU

Se crearon dos archivos nuevos: `agc_cpu.h` para declarar la estructura de la CPU y sus funciones, y `agc_cpu.cpp` para implementar la codificación, decodificación y ejecución. Separar la CPU de la memoria permite mantener el proyecto organizado.

```c
struct CPU {
  uint16_t PC; // contador de programa
  palabra_t A; // acumulador
  uint16_t Q;
  palabra_t IR;
  bool detenida;
  uint32_t ciclos;
};

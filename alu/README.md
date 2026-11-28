ALU del AGC
Autora: Samantha Naomi Vital Flores
Matrícula: 241050180

Diseño
La ALU recibe A, M y el código de operación y regresa el resultado.

Tabla de operaciones
| OP  | Nombre | Operación |   Ejemplo    |
| --- | ---    |    ---    |      ---     |
|  3  |   AD   | A = A + M | 20 + 10 = 30 |
|  2  |   CS   |   A = -M  |  -(10) = -10 |

Simulación
Se corre `alu.ino` en el ESP32, abre Monitor Serie a 115200 y se ve la tabla y los 7 pasos de la suma.

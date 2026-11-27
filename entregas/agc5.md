AGC5 - Laboratorio 11 - AGC4 DMA Video Autoguiado

Nombre: Samantha Naomi Vital Flores
Matrícula: 241050180
Grupo: IT5
Materia: Arquitectura de Computadoras - TecNM Pabellón de Arteaga
Maestro: Omar Francisco Velazquez Juarez
Herramientas: Wokwi.com / ESP32

Video de la evidencia: [Ver video Laboratorio 11](https://drive.google.com/file/d/1JXSZYLGbQNuznsWcgdECoaJBu3mWXOCh/view?usp=drive_link)
Documento PDF: [Ver documento Laboratorio 11](https://drive.google.com/file/d/1G2FYqpFG1EAYhR3IZhaPUadMhJZEKXXD/view?usp=drive_link)

Introducción

Esta práctica tuvo como finalidad ampliar las capacidades de nuestra computadora AGC simulada sobre ESP32 en Wokwi. 
En etapas previas (laboratorios 9 y 10) se estableció la base de ejecución del procesador, el manejo de registros, 
la decodificación de instrucciones y la interacción con una interfaz inspirada en el DSKY original.

El propósito central fue diseñar, implementar y validar dos subsistemas clásicos de arquitectura de computadoras:
1. Una memoria de video (Framebuffer o VRAM) mapeada en memoria,
donde cada palabra o bit tiene correspondencia directa con los píxeles de la OLED SSD1306 de 128x64.
4. Un controlador DMA (Direct Memory Access) mapeado en IOREG,
capaz de transferir bloques desde ROM hacia VRAM sin que la CPU copie cada palabra con instrucciones.

A través de esto se buscó comprender cómo opera la transferencia sin intervención constante del procesador, 
comprobando el Modo Ráfaga, el Modo Robo de Ciclo, la protección de ROM, y el ancho de banda real 
y el límite de FPS impuesto por el I2C de la OLED.

Objetivo

Diseñar, implementar y validar una memoria de video mapeada en memoria y un controlador DMA en la AGC, 
comprobando experimentalmente el Modo Ráfaga, el Modo Robo de Ciclo, la protección de memoria, 
y el ancho de banda real y el límite de FPS de la pantalla OLED.

Objetivos cumplidos:
- Implementar una VRAM de 512 palabras mapeada en 0x800.
- Implementar un controlador DMA con registros en IOREG.
- Comprobar el Modo Ráfaga (Burst) con turnosCPU = 0.
- Comprobar el Modo Robo de Ciclo (Cycle Stealing) con imagen progresiva.
- Comprobar que el DMA no puede escribir en la ROM (estado = 3).
- Medir el refresco real, el ancho de banda y los FPS máximos.
- Comprobar que refrescar más seguido no da más fluidez, sino alarma 1202.

Investigación

1. ¿Qué es un framebuffer / VRAM y cuánto ocupa una pantalla de 128x64 monocromática?
Un framebuffer es una región de memoria dedicada a almacenar la imagen que se mostrará en pantalla.
Total de píxeles: 128 x 64 = 8,192 píxeles. Como cada píxel monocromático es 1 bit: 8,192 bits / 8 = 1,024 bytes (1 KiB).
En palabras de 16 bits: 1,024 / 2 = 512 palabras. Cada fila de 128 píxeles ocupa 128 bits = 8 palabras de 16 bits.
Con 64 filas: 64 x 8 = 512 palabras.

2. ¿Qué registros necesita un controlador DMA?
- IO_DMA_ORIGEN: dirección inicial de donde se leen los datos.
- IO_DMA_DESTINO: dirección inicial donde se escriben los datos.
- IO_DMA_CONTADOR: palabras pendientes por transferir.
- IO_DMA_CTRL: bits de configuración y arranque (bit 1 = arranque, bit 2 = modo ráfaga).
- IO_DMA_ESTADO: inactivo / en curso / completado / error.

3. ¿Qué es bus mastering y por qué cada tarjeta PCIe trae su propio motor de DMA?
El bus mastering es que un periférico asume temporalmente el control del bus para leer
o escribir en memoria sin que el CPU intervenga dato por dato.
Las tarjetas PCIe actuales incorporan su propio motor DMA para transferencias directas y simultáneas sin cuellos de espera,
en vez de depender de un controlador central compartido como el 8237.

4. ¿Qué significan las alarmas 1201 y 1202?
Sobrecarga del sistema ejecutivo del AGC ante exceso de carga.
La 1202 surge al agotarse los core sets para registrar tareas en espera, y la 1201 cuando se terminan las áreas VAC.
En el Apollo 11 el ejecutivo descartó procesos secundarios y conservó la navegación.

5. ¿Qué es tearing y cómo se evita con doble búfer?
El tearing es que la pantalla muestra datos mezclados de dos cuadros distintos,
porque se está escribiendo el cuadro mientras se está dibujando.
Se evita con dos arreglos VRAM_A y VRAM_B y un registro IO_BUFFER_ACTIVO:
se escribe en el búfer inactivo mientras se dibuja desde el activo,
y al terminar se intercambian los roles con un cambio atómico del registro.

Desarrollo

Paso 0: Configuración del entorno en Wokwi y carga del código base

Se configuró un ESP32 Dev Module con:
- OLED SSD1306: SDA en GPIO 21, SCL en GPIO 22, VCC a 3V3 y GND.
- Es la interfaz visual equivalente al DSKY, ahora también para la salida gráfica de VRAM.
- MPU6050: mismo bus I2C (GPIO 21 y 22), VCC a 3V3 y GND. Aporta orientación y aceleración.
- Botón ENTR: GPIO 32 con pull-up interna, en reposo lee alto y al presionar conecta a GND. Simula la tecla ENTR del DSKY.

Se revisó `#define N_SUMA`: la regla es tomar el último dígito de la matrícula y sumarle 3 (con el convenio para el dígito 0). 
Para la matrícula 241050180 (terminada en 0), N_SUMA = 10, cuya suma de 1 a 10 es 55.

Al arrancar, el sistema emitió:
`ANCLA C40A24-Nov 25 2026-04:12:27-66972`
Y telemetría cada segundo, por ejemplo:
`T=11 V=06 N=01 R1=0 R2=10 R3=0`
Esto confirmó que el planificador estaba activo, 
el serie operaba sin errores y la CPU ejecutaba la suma de manera estable antes de las modificaciones.

Paso 1: Ampliación del mapa de memoria

1. Ampliación de E/S (`IO_TAM`): de su valor previo a 32 palabras, reservando 14 a 18 para DMA:
- `IO_DMA_ORIGEN` = 14
- `IO_DMA_DESTINO` = 15
- `IO_DMA_CONTADOR` = 16
- `IO_DMA_CTRL` = 17
- `IO_DMA_ESTADO` = 18

2. VRAM: arreglo `palabra_t VRAM[VRAM_TAM]` con `VRAM_INICIO = 0x800` y `VRAM_TAM = 512` palabras,
y bandera `bool modoVideo = false;`.
4. Ampliación de ROM (`ROM_TAM`): a 640 palabras, retirando el `const` para que `quemarImagen()`
cargue el patrón en la región de ROM de la imagen al arrancar.
6. Decodificación en `leerMem` y `escribirMem`: se agregó rango `[VRAM_INICIO, VRAM_INICIO + VRAM_TAM)
` indexando `VRAM[dir - VRAM_INICIO]`.

Pruebas de acceso a VRAM:
- Se envió `W800,FFFF` para escribir 0xFFFF (todos los bits en 1, -1 en complemento a dos) en 0x800.
- Con `V01N01`, R1 reportó `-1` (177777 en octal). Esto verificó que el decodificador enruta hacia VRAM
y que los valores se conservan sin alteración.

- ¿Por qué ampliaste IO_TAM a 32 y no a 19, que alcanzaba?
Porque los bloques de memoria y espacios de registros suelen alinearse en potencias de 2 (8, 16, 32, 64).
Con IO_TAM = 32, el rango de E/S queda en un bloque limpio que finaliza en 0x05F sin invadir la ROM que inicia en 0x100.
Esto simplifica la decodificación y evita fronteras arbitrarias o solapamientos al agregar futuros registros.

Código agregado:
```cpp
#define IO_TAM 32
#define ROM_TAM 640
#define IO_DMA_ORIGEN 14
#define IO_DMA_DESTINO 15
#define IO_DMA_CONTADOR 16
#define IO_DMA_CTRL 17
#define IO_DMA_ESTADO 18
#define VRAM_INICIO 0x800
#define VRAM_TAM 512
palabra_t VRAM[VRAM_TAM];
bool modoVideo = false;

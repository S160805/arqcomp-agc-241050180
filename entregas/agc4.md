AGC5 - Semana 10 - Interconexión con la PC y adquisición de datos

Nombre: Samantha Naomi Vital Flores
Matrícula: 241050180
Grupo: IT5
Materia: Arquitectura de Computadoras - TecNM Pabellón de Arteaga
Docente: Omar Francisco Velázquez Juárez
Fecha: Noviembre del 2026
Herramientas: Wokwi.com / ESP32

Video de la evidencia: [Ver video Semana 10](https://drive.google.com/file/d/1kwiDtWlvrM8akemHgd_yzGgLfgWjYEpS/view?usp=drive_link)
Documento PDF: [Ver documento Semana 10](https://drive.google.com/file/d/1QYmuFHQnzlh7iKeEbHEzr2Y2KeJ8Y1ju/view?usp=drive_link)

Introducción

En esta práctica se trabajó con la quinta y última fase de emulación del Apollo Guidance Computer (AGC). 
Se establece un enlace serie con nuestra propia "【entity-Houston¦canonical_name=Houston】" virtual (la laptop del operador)
y se conecta un sensor inercial simulado para transicionar de meros datos numéricos abstractos a mediciones físicas reales.

El AGC no operaba aislado: dependía del enlace de bajada de datos (Downlink) y subida de comandos (Uplink) con 
【entity-Houston¦canonical_name=Houston】, que operaba a 51,200 bits por segundo, transmitiendo solo vectores de estado,
registros, banderas y variables de navegación, no video. Y dependía de la plataforma inercial, 
que entregaba voltajes proporcionales a fuerzas y velocidades angulares, 
convertidas a palabras numéricas crudas (LSB) que solo tenían sentido cuando el software aplicaba transformaciones, 
calibraciones y filtrado.

Objetivo

Establecer un canal de comunicación bidireccional fiable entre el AGC emulado en ESP32 y 
una estación terrena (PC mediante Monitor Serie), procesar las lecturas crudas del MPU6050 en ángulos físicos de actitud, 
diseñar y evaluar un filtro de promedio móvil, y visualizar en vivo la telemetría cruda y filtrada.

Objetivos cumplidos:
- Establecer un canal bidireccional entre el AGC y la PC con protocolo de comandos y tramas de telemetría, evaluando rendimiento y ancho de banda.
- Procesar lecturas crudas de 16 bits del MPU6050 con funciones trigonométricas y escalado de precisión fija en ángulos en centésimas de grado.
- Diseñar y evaluar un filtro de promedio móvil de ventana configurable (N = 1, 8, 64), demostrando el equilibrio entre reducción de ruido y retardo temporal.
- Visualizar y graficar en vivo la telemetría cruda y filtrada, validando estabilidad a 10 Hz y bajo saturación.

Investigación

1. ¿Qué significa 115200 8N1 en un enlace serie? ¿Cuántos bits viajan por cada byte?
115200 es la velocidad en baudios (115,200 bits por segundo).
8 = 8 bits de datos útiles por byte. N = sin bit de paridad (None). 1 = 1 bit de parada.
Además toda trama UART necesita 1 bit de inicio. Por cada byte útil viajan: 1 inicio + 8 datos + 1 parada = 10 bits físicos.
Throughput máximo = 115,200 / 10 = 11,520 bytes/segundo.

2. Sensibilidad del acelerómetro del MPU6050 en escala ±2 g: ¿cuántos LSB equivalen a 1 g?
En escala ±2 g, la sensibilidad nominal es 16,384 LSB/g. En reposo con la gravedad sobre un eje,
ese eje entrega ~16,384 (0x4000). En escalas ±4 g, ±8 g y ±16 g la sensibilidad baja a 8,192, 4,096 y 2,048 LSB/g.

3. ¿Cuál es la diferencia entre sondeo (polling) e interrupciones?
El sondeo revisa continuamente si ocurrió un evento.
Las interrupciones permiten que el hardware avise al procesador cuando sucede un evento, sin revisar constantemente.
Una ISR demasiado larga puede retrasar otras tareas.

5. ¿Qué representan las alarmas 1201 y 1202?
Sobrecarga del sistema ejecutivo del AGC, cuando no puede atender las tareas programadas en el tiempo disponible.
La 1202 es la del Apollo 11.

5. ¿Para qué sirve IRAM_ATTR y por qué no usar Serial.print() en una ISR?
IRAM_ATTR ubica una función en la memoria interna de instrucciones del ESP32, importante para interrupciones.
Serial.print() puede tardar demasiado y usar recursos compartidos.
La ISR solo debe registrar el evento y dejar el procesamiento para el loop.

Desarrollo

Paso 0: Arquitectura del código base del AGC (7 bloques)

1. Tipos y ancla de sesión: `palabra_t` = int16_t, estructura `CPU` con PC, A, Q, IR, detenida, ciclos.
`imprimirAncla()` con MAC de `ESP.getEfuseMac()` + sal de `esp_random()`.
3. Mapa de memoria y ROM: RAM 64 palabras 0x000-0x03F, E/S 16 posiciones 0x040-0x04F, ROM 64 palabras 0x100-0x13F.
4. Programa de suma en ROM con N_SUMA = 10, resultado en RAM[10] = 55 (10x11/2 = 55). `leerMem()` y `escribirMem()` validan rangos,
si se escribe en ROM o fuera activan `alarmaMem = true`.
6. CPU: `decodificar()` separa opcode (ins >> 12 & 0x0F) y dirección (ins & 0x0FFF).
`cpuPaso()` hace FETCH, DECODE, EXECUTE con 8 instrucciones: TC, CA, CS, AD, TS, XCH, BZF, HALT.
8. Sensor MPU6050: Con `Wire.h`. `mpuDespertar()` escribe 0 en 0x6B para salir de Sleep.
`mpuLeer()` pide 14 bytes desde 0x3B y actualiza registros de E/S.
10. Interrupciones y cola: Temporizador cada 100 ms (`onTimer`).
No ejecuta tareas pesadas, llama a `pedir()` que encola en cola circular con `portENTER_CRITICAL_SAFE`. Si se satura, alarma 1202.
12. DSKY y serial: `registrosDelSustantivo()` mapea R1, R2, R3 según sustantivo (01 RAM, 20 giróscopos, 21 cola/alarma/ticks,
23 inclinación). `atenderSerie()` interpreta comandos.
14. Tareas y loop: `ejecutarTarea()` mide con `micros()`. `recuperar1202()` vacía cola,
reinicia CPU sin borrar RAM y restaura operación normal.

Entorno: ESP32 a 240 MHz, OLED SSD1306 128x64 por I2C (GPIO 21 SDA, GPIO 22 SCL) a 400 kHz, MPU6050 en 0x68 con 14 bytes, 
botón ENTR en GPIO 32 con INPUT_PULLUP y flanco de bajada (FALLING).

Limitaciones de Wokwi: el serie emulado no reproduce tiempos exactos de un UART físico, 
el MPU6050 simulado no tiene ruido térmico real (por eso en reposo da 0), y los ciclos de emulación dependen de la PC anfitriona.

Paso 1: Houston, enlace con la PC y protocolo serie

Telemetría en tiempo real: En `loop()` se transmite cada segundo una cadena compacta de ~42 bytes, no una pantalla de 1,024 bytes:
`T=1021 V=06 N=21 R1=0 R2=0 R3=1021`
Con ticks, verbo, sustantivo y R1, R2, R3.

Comando V (Sustantivo 21): Se envió comando de cambio de sustantivo al DSKY para verificar el canal bidireccional.

Comando W (Escritura en RAM): `W000,0100` = escribir 0x0100 en RAM 0x000. Respuesta: `OK 000 = 0100`. 
RAM[0] es puntero para R1. Al cambiar de 0x0020 a 0x0100 (base de ROM), 
R1 mostró la primera instrucción del programa de suma (0x100B). Con `V01N01` mostró en octal `010013`, equivalente a 4107 decimal.

Alarma 1202 y reconocimiento remoto: Con `T1` (interrupción cada 1 ms) la cola colapsó, apareció PROG en OLED y 1202 en IO_ALARMA. 
Con `T100` (100 ms) `recuperar1202()` vació la cola y reinició la CPU sin perder RAM. 
Para apagar PROG se envió `W04A,0000` (0x04A = IO_ALARMA). Respuesta: `OK 04A = 0000`, se limpió la alarma y se apagó PROG.

Medición de ancho de banda (Comando B): El comando B transmite 1,024 bytes de 'U' (0x55) midiendo con `micros()`. 
Tres mediciones: 88,366 µs, 88,366 µs, 88,364 µs. Mediana: 88,366 µs. Teórico: 1,024 / 11,520 = 0.08888 s = 88,888.8 µs. 
El medido coincide con el teórico.

¿60 pantallas por segundo? Falso.** Cada cuadro OLED es 128x64 bits = 1,024 bytes. A 60 FPS se necesitan 1,024 x 60 = 61,440 bytes/s. 
El canal solo da 11,520 bytes/s. Factor de saturación: 61,440 / 11,520 = 5.33 veces mayor. 
Solo sostiene ~11.25 pantallas/s (11,520 / 1,024). Por eso se transmiten tramas de texto de ~40 bytes, no video.

Paso 2: Del número crudo al grado - MPU6050 a ángulos con atan2

Cuando el sensor está estático, el vector de aceleración es la gravedad (1 g). Según la inclinación, 
la gravedad se descompone entre Y y Z. Para el ángulo se usa:
`θ = atan2(Ay, Az) x (180.0 / π)`
`atan2` maneja todos los cuadrantes y evita división entre cero.

Como el AGC solo usa enteros de 16 bits, el ángulo se multiplica por 100 y se guarda en `IO_INCL`:
`IO_INCL = (palabra_t)(θ x 100)`
Ejemplo: 45.27° se guarda como 4527.

**Registro IO_INCL y sustantivo 23:** Se añadió `#define IO_INCL 13` y sustantivo 23 (N=23). Con `V06N23`: R1 = inclinación filtrada en centésimas, R2 = crudo de Ay (IO_ACC_Y), R3 = crudo de Az (IO_ACC_Z).

| Prueba                    | Esperado | Obtenido                  |
| ---                       | ---      | ---                       |
| Placa plana, Y=0 g, Z=1 g | 0°       | R1=0, R2=0, R3=16384      |
| Placa +90°, Y=1 g, Z=0 g  | +90.00°  | R1=9000, R2=16384, R3=0   |
| Placa -90°, Y=-1 g, Z=0 g | -90.00°  | R1=-9000, R2=-16384, R3=0 |

El 16384 en Z confirma la sensibilidad de 16,384 LSB/g en ±2 g.

Resolución por LSB cerca de 0°:** θ ≈ Y/Z. Con Z = 16,384 LSB, Δθ = 1 / 16,384 = 0.000061035 rad = 0.003496° ≈ 0.0035° por LSB. 
Por eso mostrar centésimas (0.01°) es válido, mostrar milésimas (0.001°) sería ficticio, por debajo de la resolución del ADC.

Paso 3: Ruido, filtrado y retardo - Promedio móvil F1, F8, F64

Filtro de promedio móvil con búfer circular `#define FILTRO_MAX 64`. Cada lectura nueva entra a la ventana y se calcula la media:
`Inclinación filtrada = (1/N) Σ Ventana[i]`
Comando `F<n>` cambia la ventana en caliente: F1 = sin filtro (N=1), F8 = moderado (N=8), F64 = pesado (N=64).

Modo instrumento (V27):** Con `V27` se suspende la telemetría de cada segundo y se emite flujo continuo `filtrada:0.00, cruda:0.00` 
para graficar en Serial Plotter.

Frecuencia de muestreo 10 Hz: Temporizador cada 100 ms, f = 1 / 0.1 s = 10 Hz. 
Con 100 marcas de tiempo, de t=10393 ms a t=20393 ms pasaron exactamente 10 s para 100 muestras, sin pérdida.

Ruido en reposo: Placa plana, V27, 100 líneas por filtro. F1: promedio 0.0000°, desviación 0.0000°. 
F8: promedio 0.0000°, desviación 0.0000°. F64: promedio 0.0000°, desviación 0.0000°. 
Desviación 0 porque el MPU6050 de Wokwi es ideal, sin ruido térmico. 
Teóricamente el filtro reduce la desviación en 1/√N: F8 en 2.83 veces, F64 en 8 veces.

Retardo ante cambio escalón:
| Filtro | Retardo en muestras | Retardo en tiempo |
| ---    | ---                 | ---               |
| F1     | 1 muestra           | 0.1 s             |
| F8     | 8 muestras          | 0.8 s             |
| F64    | ~64 muestras        | ~6.4 s a 10 Hz    |

F1 sigue instantáneo a la cruda. F8 sube en rampa hasta la 9a muestra. F64 tarda más de 6 segundos. 
Trade-off: más suavidad = más retardo.

Prueba de estrés T10: Con `T10` + `F64`, el temporizador va a 10 ms (100 muestras/s). 
El retardo en segundos baja a 64/100 = 0.64 s. Pero con `V06N21` la cola R1 llegó a 8 (saturada) y la alarma R2 se disparó a 1202. 
La CPU no pudo procesar interrupciones a 100 Hz + filtro de 64. Al volver a `T100`, la cola se vació y el sistema se recuperó.

Evidencia

- Telemetría cada segundo: `T=1021 V=06 N=21 R1=0 R2=0 R3=1021` (~42 bytes).
- Comando W: `W000,0100` -> `OK 000 = 0100`, con `V01N01` R1 = 010013 octal (4107 decimal).
- Alarma 1202 con `T1`, recuperación con `T100`, apagado de PROG con `W04A,0000` -> `OK 04A = 0000`.
- Comando B: 1,024 bytes en 88,366 µs (mediana), teórico 88,888.8 µs.
- Video a 60 FPS imposible: necesita 61,440 bytes/s, canal solo 11,520 bytes/s, factor 5.33.
- IO_INCL 13, sustantivo 23, `V06N23`: plana R1=0, +90° R1=9000, -90° R1=-9000.
- Resolución: 0.0035° por LSB, por eso centésimas de grado.
- Filtros F1, F8, F64 con `V27`, muestreo 10 Hz, 100 muestras en 10 s.
- Ruido en reposo: desviación 0.0000° en los 3 filtros por sensor ideal de Wokwi.
- Retardo: F1 0.1 s, F8 0.8 s, F64 6.4 s.
- Estrés T10 + F64: cola = 8, alarma = 1202, recuperación con T100.

Autovalidación

- [x] La PC escribe en la RAM de mi AGC con el comando W, y la protección impide escrituras en ROM.
- [x] Se midió el enlace serie con el comando B, determinando las limitaciones frente a video.
- [x] Se calculó y comprobó la resolución de 1 LSB cerca de 0° (0.0035°), justificando centésimas de grado.
- [x] Se recolectaron 100 muestras en reposo para caracterizar ruido con varios filtros.
- [x] Se documentó el trade-off: filtro más suave reduce ruido pero penaliza retardo.
- [x] Se validó el Serial Plotter graficando cruda vs filtrada con desfase temporal.

Resultados obtenidos

- Se estableció el enlace bidireccional AGC-PC con telemetría y comandos V, W, B, T, F.
- Se implementó IO_INCL y el sustantivo 23 para mostrar inclinación en centésimas.
- Se comprobó la conversión atan2 en plana, +90° y -90°.
- Se implementó el filtro de promedio móvil F1, F8, F64 y el modo instrumento V27.
- Se verificó muestreo a 10 Hz y retardo de cada filtro.
- Se probó la saturación con T10 y la recuperación de la alarma 1202.

Conclusión

En esta práctica se estableció el enlace con 【entity-Houston¦canonical_name=Houston】 con telemetría compacta, 
se convirtió el número crudo del MPU6050 a ángulo físico con atan2 y escala x100, 
y se demostró el trade-off del filtrado: un filtro más pesado suaviza más pero retarda más. 
El sistema se mantuvo estable a 10 Hz, se saturó con T10 + F64 con alarma 1202, y se recuperó al volver a T100. 
Se comprendió por qué el AGC real transmitía estado y no video, y por qué la ISR debe ser breve y el trabajo pesado debe ir en el loop.

Declaración del uso de la IA

Para la realización, estructuración y desarrollo formal de este reporte académico de la Semana 10, se utilizó la herramienta de asistencia técnica Gemini (Google, 2026) como apoyo y guía de consulta metodológica. La inteligencia artificial colaboró en la comprensión conceptual de los fundamentos teóricos del Apollo Guidance Computer, la sintaxis de los protocolos de comunicación serie asíncrona UART, la formulación matemática de las transformaciones trigonométricas del acelerómetro MPU6050 mediante atan2, el análisis del diseño de filtros de promedio móvil y la redacción formal de la documentación técnica.

Todo el trabajo práctico de configuración del entorno, despliegue y depuración del código en el simulador Wokwi, ejecución de comandos en el Monitor Serie, toma de capturas de pantalla, procesamiento de datos en hojas de cálculo y análisis crítico de los resultados experimentales fue realizado de manera personal y autónoma por la alumna responsable Samantha Naomi Vital Flores.

Referencias

- InvenSense. (2013). MPU-6000 and MPU-6050 Product Specification (Revision 3.4, PS-MPU-6000A-00). Sección 6.2: Accelerometer Specifications.
- Espressif Systems. (s.f.). Arduino core for the ESP32: Serial Communication and Hardware Timer APIs. https://docs.espressif.com/projects/arduino-esp32/
- Arduino Documentation. (s.f.). Using the Serial Monitor and Serial Plotter tools in embedded debugging. https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-serial-plotter/
- Burkey, R. (s.f.). Virtual AGC Project: Apollo Guidance Computer source code, downlink telemetry protocols and inertial platform simulation. https://www.ibiblio.org/apollo/
- NASA. (1968). AGC System Overview and Flight Software Development Standards. MIT Instrumentation Laboratory Report R-546.

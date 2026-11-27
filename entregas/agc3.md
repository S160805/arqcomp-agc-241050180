AGC3 - Semanas 8 y 9 - MPU6050, interrupciones y alarma 1202

Nombre: Samantha Naomi Vital Flores
Matrícula: 241050180
Grupo: IT5
Materia: Arquitectura de Computadoras - TecNM Pabellón de Arteaga
Fecha: 19 al 30 de octubre del 2026
Herramientas: Wokwi.com / ESP32

Documento PDF: [Ver documento Semana 8](https://drive.google.com/file/d/1UTXk_Jtt0uKPXMOKqhzkTEhgbh4TQJdq/view?usp=drive_link)

Video de la evidencia: [Ver video Semana 8 y 9](https://drive.google.com/file/d/1H1W_wX_VMvzHK-bRtwUE4FNsA5XoJzG5/view?usp=drive_link)

Documento PDF: [Ver documento Semana 9](https://drive.google.com/file/d/1lHwRtkgK47ipulbTvaR37Wec36HGNt67/view?usp=drive_link)

Introducción

En esta práctica se trabajó con una computadora de guía simulada, conocida como AGC, implementada en un ESP32. Se integró el sensor MPU6050 para obtener datos de aceleración y velocidad angular mediante el protocolo de comunicación I2C.

Para facilitar el acceso a estos datos, se utilizan registros de entrada y salida que permiten a la CPU consultar la información del sensor sin comunicarse directamente con el bus I2C. Además, se implementaron interrupciones para detectar eventos generados por un temporizador y por el botón ENTR.

Estas actividades permiten comprender la importancia de administrar correctamente los recursos del procesador, atender los eventos en el momento adecuado y evitar que las operaciones prolongadas afecten el funcionamiento general del sistema.

Objetivo

Implementar la lectura del sensor MPU6050 mediante registros de entrada y salida de la AGC, así como utilizar interrupciones de temporizador y de pin en el ESP32. También se busca comprender el funcionamiento de las rutinas de interrupción y reconocer qué operaciones deben ejecutarse fuera de ellas para mantener el sistema funcionando correctamente.

Objetivos cumplidos:
- Implementar la lectura del MPU6050 mediante registros de E/S de la AGC.
- Utilizar interrupciones de temporizador y de pin en el ESP32.
- Comprender el funcionamiento de las rutinas de interrupción (ISR).
- Reconocer qué operaciones deben ejecutarse fuera de la ISR.
- Configurar el botón ENTR con interrupción y antirrebote.
- Probar la alarma 1202 y la recuperación del sistema.

Investigación

1. ¿Cuál es la diferencia entre sondeo (polling) e interrupciones?**
El sondeo consiste en revisar continuamente si ocurrió un evento. El programa consulta repetidamente el estado de un dispositivo, incluso cuando no hay cambios. En cambio, las interrupciones permiten que el hardware avise al procesador cuando sucede un evento. Esto evita que el programa tenga que revisar constantemente el dispositivo y permite atender las situaciones importantes cuando ocurren. Sin embargo, las interrupciones deben utilizarse de manera adecuada, ya que una rutina demasiado larga puede retrasar otras tareas y afectar el funcionamiento del sistema.

2. ¿Qué representan las alarmas 1201 y 1202 en el contexto de la AGC?**
Las alarmas 1201 y 1202 se relacionan con situaciones de sobrecarga del sistema ejecutivo de la computadora de guía Apollo, cuando el procesador no puede atender correctamente las tareas programadas dentro del tiempo disponible. La alarma 1202 es conocida por el incidente ocurrido durante el Apollo 11, cuando la computadora experimentó una sobrecarga de tareas. Estas alarmas muestran la importancia de administrar los recursos del procesador y de establecer mecanismos que permitan mantener las funciones principales del sistema.

3. ¿Para qué sirve IRAM_ATTR y por qué no se recomienda utilizar Serial.print() dentro de una ISR?**
IRAM_ATTR es un atributo utilizado en el entorno ESP32 para ubicar una función en la memoria interna de instrucciones. Esto resulta importante para determinadas funciones que deben ejecutarse durante una interrupción. No se recomienda utilizar Serial.print() dentro de una ISR porque puede consumir demasiado tiempo y utilizar recursos compartidos. Una rutina de interrupción debe realizar únicamente las operaciones necesarias para registrar el evento y dejar el procesamiento adicional para el ciclo principal. De esta manera, se reducen los posibles retrasos y se mejora la estabilidad del programa.

Desarrollo

3.1. Lectura del sensor MPU6050

El MPU6050 es un sensor que permite obtener información sobre la aceleración y la velocidad angular en tres ejes. Para comunicarse con el ESP32 utiliza el protocolo I2C.

Durante la configuración se identificó el sensor mediante la dirección 0x68. Posteriormente, se realizó la lectura de los registros que contienen los datos del dispositivo. El programa solicita 14 bytes consecutivos a partir del registro de aceleración y utiliza esa información para actualizar los registros de entrada y salida de la AGC.

Los datos se organizan en los siguientes registros:

| Registro  |         Información           |
| ---       | --- |
| IO_ACC_X  |       Aceleración en el eje X            |
| IO_ACC_Y  |         Aceleración en el eje Y          |   
| IO_ACC_Z  |        Aceleración en el eje Z           |
| IO_GYR_X  |      Velocidad angular en el eje X       |
| IO_GYR_Y  |      Velocidad angular en el eje Y       |
| IO_GYR_Z  |      Velocidad angular en el eje Z       |
| IO_ALARMA |         Código de alarma activa          |
| IO_COLA   | Cantidad de tareas pendientes en la cola |

Esta organización permite que la CPU consulte los valores del sensor desde los registros de la AGC, sin tener que utilizar directamente las funciones de comunicación I2C.

- ¿Por qué las lecturas del giroscopio en reposo no suelen ser exactamente cero? ¿Por qué Wokwi puede mostrar cero?
En un sensor físico, las lecturas pueden presentar pequeñas variaciones debido al ruido electrónico, las características del dispositivo y el offset. Por eso, aunque el sensor permanezca quieto, sus valores no necesariamente son exactamente cero. En Wokwi, las lecturas dependen del comportamiento del modelo simulado y de la forma en que se representa el movimiento del sensor. Por esta razón, los valores que aparecen en la simulación pueden ser diferentes de los obtenidos con un dispositivo físico.

3.2. Interrupción del temporizador

Se configuró un temporizador en el ESP32 para generar interrupciones periódicas. 
Cada vez que ocurre una interrupción, se incrementa un contador de ticks 
y se activa una bandera que indica que existe una tarea pendiente.

Después, el ciclo principal del programa revisa la bandera 
y realiza las operaciones correspondientes, incluida la lectura del sensor. 
Esta organización permite mantener la rutina de interrupción breve y ejecutar las tareas que requieren más tiempo fuera de ella.

Para configurar el temporizador se utilizaron funciones del entorno Arduino-ESP32 3.x, como `timerBegin()`, `timerAttachInterrupt()` y `timerAlarm()`.

Prueba de lectura I2C dentro de la interrupción:
También se realizó una prueba colocando la lectura I2C dentro de la rutina de interrupción para observar el comportamiento del programa.

- ¿Qué ocurre al colocar la lectura I2C dentro de la ISR y por qué?
La comunicación I2C puede requerir bastante tiempo en comparación con las operaciones simples que
normalmente se realizan dentro de una rutina de interrupción. Si la lectura del sensor se coloca dentro de la ISR,
la atención del evento puede prolongarse y afectar otras operaciones del sistema. Por esta razón,
es recomendable que la ISR se limite a actualizar variables o activar banderas.
La lectura del sensor y el procesamiento de los datos deben realizarse posteriormente en el ciclo principal del programa.

3.3. Interrupción del botón ENTR

En esta parte se trabajó con el botón ENTR conectado al GPIO 32 del ESP32. 
El botón se conecta a GND y se utiliza una resistencia pull-up para mantener estable la señal cuando no está presionado.

La interrupción se configura para detectar el flanco de bajada,
que corresponde al cambio de estado producido al presionar el botón. 
Además, se utiliza un intervalo de antirrebote de 200 milisegundos para evitar que una sola pulsación genere varios eventos.

| Prueba                    | Pulsaciones realizadas | Eventos detectados |
| ---                       | ---                    | ---                |
| Con antirrebote de 200 ms | 10                     |                 10 |

Durante la simulación se registraron 10 pulsaciones y 10 eventos detectados. Esto muestra que, 
con la configuración utilizada, cada pulsación contabilizada produjo un evento. 
El antirrebote ayudó a evitar que las variaciones rápidas de la señal fueran registradas como pulsaciones adicionales.

- ¿Por qué en Wokwi pueden registrarse exactamente 10 eventos y con un botón físico el resultado podría ser diferente?
- ¿Qué elemento físico no representa completamente el simulador?
Un botón físico puede generar rebotes eléctricos cuando sus contactos se abren o se cierran.
Estos rebotes producen cambios rápidos en la señal que pueden interpretarse como varios eventos en lugar de una sola pulsación.
En una simulación, la interacción suele ser más controlada. Por ello, el resultado puede ser más uniforme que con un componente real.
El aspecto que no se representa completamente es el comportamiento eléctrico de los contactos mecánicos,
especialmente el rebote que ocurre al presionar o soltar el botón.

3.4. Pruebas complementarias: Alarma 1202 y recuperación

Como parte de las pruebas del sistema, se modificó el periodo del temporizador para observar cómo cambiaba
el comportamiento de la cola de tareas.

En las evidencias se observa que la alarma 1202 apareció al utilizar un periodo de 2 milisegundos.
Esta configuración incrementa la frecuencia con la que se generan eventos y permite observar cómo una cantidad elevada de trabajo puede provocar la saturación de la cola.

Después de la alarma, el sistema se recupera: se vacía la cola de tareas pendientes, 
se limpia la alarma y la CPU vuelve a ejecutar sus tareas normales.

También se verificó el cambio en RAM 10 y el encolado de una tarea a los 60 segundos, 
como se muestra en las evidencias del monitor serial.

Evidencia

- Comunicación con el MPU6050 mediante I2C, dirección 0x68, lectura de 14 bytes consecutivos desde el registro de aceleración.
- Datos del sensor organizados en registros de E/S: IO_ACC_X, IO_ACC_Y, IO_ACC_Z, IO_GYR_X, IO_GYR_Y, IO_GYR_Z, IO_ALARMA, IO_COLA.
- Temporizador periódico en ESP32 con `timerBegin()`, `timerAttachInterrupt()` y `timerAlarm()`. Cada interrupción incrementa ticks y activa bandera de tarea pendiente.
- Prueba I2C dentro de la ISR: se comprobó que prolonga la ISR y afecta al sistema, por eso la lectura se deja para el loop principal.
- Botón ENTR en GPIO 32, flanco de bajada, pull-up, antirrebote 200 ms: 10 pulsaciones = 10 eventos detectados.
- Alarma 1202 con periodo de temporizador de 2 ms: saturación de la cola por exceso de tareas.
- Recuperación después de la alarma 1202: limpieza de cola y de alarma, regreso a operación normal.
- Evidencias del monitor serial, cambios de RAM, RAM 10 y encolado de tarea a los 60 s.

Autovalidación

| Criterio | Resultado |
| --- | --- |
| Se detecta el MPU6050 en 0x68 y se leen 14 bytes | Cumple |
| Los datos del sensor quedan en registros de E/S | Cumple: ACC y GYR en IO |
| El temporizador genera interrupciones y ticks | Cumple |
| La ISR es breve, la lectura I2C se hace en el loop | Cumple |
| El botón ENTR con antirrebote 200 ms da 10 de 10 | Cumple: 10 pulsaciones, 10 eventos |
| La alarma 1202 aparece con periodo de 2 ms | Cumple |
| El sistema se recupera después de la alarma | Cumple |

Resultados obtenidos

- Se comprobó la comunicación con el sensor MPU6050 mediante I2C.
- Se revisó la lectura de los registros de aceleración y velocidad angular.
- Se trabajó con los registros de entrada y salida de la AGC para organizar los datos del sensor.
- Se implementó un temporizador periódico para generar interrupciones en el ESP32.
- Se analizó la importancia de mantener las rutinas de interrupción breves y eficientes.
- Se configuró el botón ENTR con interrupción y antirrebote de 200 milisegundos.
- Se registraron 10 pulsaciones y 10 eventos del botón durante la simulación.
- Se observó el comportamiento de la cola de tareas al modificar el periodo del temporizador.
- Se probó la alarma 1202 y el procedimiento de recuperación del sistema.

Conclusión

En esta práctica se trabajó con la integración del sensor MPU6050 a la AGC mediante registros de entrada y salida, 
así como con el uso de interrupciones en el ESP32. Se comprendió la importancia de separar las operaciones que se ejecutan
dentro de una interrupción de aquellas que requieren más tiempo, como la comunicación I2C.

La configuración del botón ENTR permitió observar el funcionamiento de las interrupciones y 
la utilidad del antirrebote para evitar eventos repetidos. Asimismo, las pruebas de la alarma 1202 y 
la recuperación ayudaron a comprender cómo la saturación de la cola puede afectar el funcionamiento del sistema y 
por qué es importante administrar correctamente las tareas.

En general, la práctica permitió reforzar los conocimientos sobre comunicación con dispositivos, 
temporizadores, interrupciones y manejo de tareas dentro de una computadora de guía simulada.

Declaración del uso de la IA

Se utilizó la IA para ayuda con la estructura del código y la organización de los resultados y estructura para el reporte.

Referencias

- InvenSense. (s.f.). MPU-6000 and MPU-6050 Register Map and Descriptions (RM-MPU-6000A).
- Espressif Systems. (s.f.). Arduino ESP32 Documentation. https://docs.espressif.com/projects/arduino-esp32/
- Virtual AGC. (s.f.). Virtual AGC Project. https://www.ibiblio.org/apollo/

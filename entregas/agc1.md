AGC1 - Semana 6 - Memoria y E/S

Nombre: Samantha Naomi Vital Flores
Matrícula: 241050180
Video: [Ver video Semana 6](https://drive.google.com/file/d/1NQXHEhFArjurSx4xGgBLLN0X7_s0AzyI/view?usp=drive_link)
Documento PDF: [Ver documento Semana 6](https://drive.google.com/file/d/10MiWbZhiilsTZWs9gZdBqg6AzGjunkzK/view?usp=drive_link)

1. Objetivo
Construir en un ESP32 una versión simplificada del DSKY del Apollo Guidance Computer (AGC): un mapa de memoria con RAM,
E/S mapeada y ROM, alimentado por cuatro botones y consultado en un OLED mediante el protocolo Verbo-Sustantivo.

Objetivos cumplidos:
- Definir un mapa de memoria sin traslapes con regiones RAM, E/S y ROM.
- Depositar el estado de cuatro botones en el registro de E/S IO_BOTONES.
- Mostrar el contenido de direcciones de memoria con los verbos 01 y 06 y los sustantivos 01 y 60.
- Distinguir memoria de registro de E/S con evidencia del propio montaje simulado.

2. Desarrollo
Se montó todo en Wokwi con la opción B. El OLED SSD1306 y el MPU6050 comparten el bus I2C en GPIO 21 / GPIO 22,
y los cuatro botones van del GPIO a GND, sin resistencias externas, con INPUT_PULLUP:
- Botón CAMPO - bit0 de IO_BOTONES - GPIO 25
- Botón MAS - bit1 - GPIO 26
- Botón MENOS - bit2 - GPIO 27
- Botón ENTR - bit3 - GPIO 32

En la E/S mapeada a memoria, los registros de los periféricos ocupan direcciones del mismo espacio que la memoria y 
se accede a ellos con las mismas instrucciones de carga y almacenamiento. 
La palabra del AGC era de 16 bits: 15 de dato y 1 de paridad, y usaba complemento a uno.
La función escribirMem hace cumplir el mapa de memoria, los botones escriben en un registro de E/S y el DSKY lee cualquier dirección y la muestra en octal o en decimal con signo. 
El OLED muestra en la primera línea el verbo y el sustantivo que se están tecleando, tres registros R1-R3 y una línea USO.
Verbo 01 = palabra en octal (6 dígitos), Verbo 06 = palabra en decimal con signo (5 dígitos). Sustantivo 01 = contenido de la dirección en RAM[0], RAM[1], RAM[2]. 
Sustantivo 60 = IO_BOTONES en octal, IO_JOY_SW, IO_TICKS.
Se necesitan 6 dígitos octales para una palabra de 16 bits: cinco cubren 15 bits y el sexto lleva el bit más significativo, por lo que el primer dígito solo puede valer 0 o 1.
Para teclear sin teclado: CAMPO avanza el cursor por las decenas y unidades del verbo y del sustantivo, MAS / MENOS incrementan o decrementan el dígito activo de 0 a 9 con envolvente, 
y ENTR aplica el verbo/sustantivo.

3. Evidencia
- Circuito montado en Wokwi con ESP32, OLED y 4 botones CAMPO, MAS, MENOS, ENTR.
- Ancla: ANCLA C40A24-Oct 15 2026-00:11:19-15894. Evidencias de tres compilaciones del mismo proyecto (23:30:52, 23:58:04 y 00:11:19).
- Con V06 N60: sin botón R1 = 000000, con MAS R1 = 000002, con ENTR R1 = 000010 y R2 = +00000, R3 = ticks en aumento.
- Con V01 N01 aplicado: la matrícula en octal R1 = 022020, R2 = 050030, R3 = 000000, que corresponde a 0x2410, 0x5018, 0x0000.
- Prueba de pulsación sostenida: MAS sostenido ~3 s incrementó una sola vez. El verbo pasó de 06 a 16 sin repetir el incremento.
- Prueba escribirMem(0x100, 5): activa alarmaMem y ROM[0] sigue igual. ROM[0] = 0x2410 antes y después, alarma ACTIVA.
- Prueba con INPUT sin pull-up: un pin flotante lee valores indeterminados, no un 0 confiable. Conteo de rebote: 2 transiciones 0->1 en una sola pulsación de MAS.

4. Conclusión
El mapa de memoria funcionó como frontera entre las regiones: la ROM no se modificó al intentar escribirla,
los accesos fuera del mapa activaron la alarma y el registro IO_TICKS ignoró las escrituras.
Las direcciones físicas confirmaron que el arreglo const quedó en flash mapeada y el arreglo de RAM en SRAM interna,
aunque es escribirMem y no el hardware quien impide escribir en la ROM.
La diferencia entre memoria y registro de E/S quedó clara en el montaje: la RAM guarda lo que se escribe en ella,
mientras que IO_BOTONES e IO_TICKS cambian solos porque reflejan el estado del hardware.
La prueba con INPUT mostró que un pin sin pull-up no tiene un valor confiable, y el antirrebote por software, leyendo solo IOREG,
evitó pulsaciones repetidas y fantasma. El DSKY mostró la matrícula en octal mediante direccionamiento indirecto,
como hacían los astronautas del Apollo con verbos y sustantivos.

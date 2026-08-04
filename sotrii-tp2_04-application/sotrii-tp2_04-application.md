# CESE - Sistemas Operativos de Tiempo Real 

## Trabajo Práctico N°: 2 – Active Object

### TP2 – Actividad 04 – Active Object Application

## Paso 1

Proyecto generado e importado. Compila en STM32CubeIDE.

---

## Paso 2

Archivo de entrega creado

---

## Paso 3

Se implementa la aplicación con tres Active Objects: dos AO Btn para `BTN_A` y `BTN_B`, un AO Sys y tres AO Led para `LED_A`, `LED_B` y `LED_C`. Los AO Btn envían mensajes con `evento y tiempo` al AO Sys, y este último reenvía el `evento` correspondiente a los AO Led.

La lógica del sistema queda definida de la siguiente manera:

- Estado normal: `LED_A` y `LED_B` encendidos, `LED_C` apagado.
- Presión breve de `BTN_A`: hace titilar `LED_A` y enciende `LED_C` durante `T_A = 5 s`.
- Presión prolongada de `BTN_A`: modifica el tiempo de operación con el valor recibido en `tiempo`.
- Presión breve de `BTN_B`: hace titilar `LED_B` y enciende `LED_C` durante `T_B = 10 s`.
- Presión prolongada de `BTN_B`: modifica el tiempo de operación con el valor recibido en `tiempo`.

Se definen las funciones de interfaz para cada AO: `open_btn_ao()`, `release_btn_ao()`, `send_btn_ao()`, `ioctl_btn_ao()`, `open_sys_ao()`, `release_sys_ao()`, `send_sys_ao()`, `ioctl_sys_ao()`, `open_led_ao()`, `release_led_ao()`, `send_led_ao()` e `ioctl_led_ao()`, usando colas de FreeRTOS para el intercambio de eventos.

En `task_sys` se manejan 3 estados, el primero `INIT` se encarga de llevar al sistema al estado normal, con `LED_A` y `LED_B` encendidos. Luego pasa al estado `IDLE` donde se espera hasta el evento `BUTTON_UP`, aqui se compara si el tiempo que ha estado presionado el boton es mayor al umbral de dos segundos se debe modificar el tiempo de operación. Una vez identificado este evento el estado pasa a `ACTIVE` y monitorea constantemente si ya ha terminado el tiempo de operacion para volver al estado `INIT`. 

Si llegan nuevos eventos en el estado `ACTIVE` son ignorados hasta terminar la ejecución del active object actual, lo cual ocurre al volver al estado `INIT`.

### WCET — medicion y registro

Medicion con DWT, STM32CubeIDE Live Expressions:

| Medicion | Variable | WCET [us] |
|----------|----------|-----------|
| Send event to LED | `g_task_send_led_ao_runtime_us` | 2 - 4 |
| Send event to SYS | `g_task_send_sys_ao_runtime_us` | 3 |


### Video del tp funcionando
[Video / archivo en Drive](https://drive.google.com/file/d/1QX4XySN6ypw3yK2sAof4tUiLwWRKkSZm/view?usp=drive_link)

---

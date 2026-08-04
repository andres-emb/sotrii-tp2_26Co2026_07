# CESE - Sistemas Operativos de Tiempo Real 

## Trabajo Práctico N°: 2 – Active Object

### TP2 – Actividad 02 – Active Object Sys


## Paso 1

Proyecto generado e importado. Compila en STM32CubeIDE.

---

## Paso 2

Archivo de entrega creado

---

## Paso 3

Se implementa una nueva estructura para almacenar el estado del objeto activo, esto incluye la cola de los eventos del modulo sys, textos de depuración y el handle de la tarea particular.

Se definen funciones de interfaz para controlar el AO de forma encapsulada:

- `open_sys_ao()`: crea la cola de eventos, la registra en FreeRTOS y lanza la tarea Gatekeeper que ejecutará el Active Object.
- `release_sys_ao()`: libera los recursos del AO, eliminando la cola y finalizando la tarea asociada.
- `send_sys_ao()`: envía un mensaje al AO con el evento y el tiempo, permitiendo que otras tareas soliciten cambios en el comportamiento del sistema. En particular esta función es utilizada por el modulo button.
- `ioctl_sys_ao()`: queda preparada para futuras operaciones de configuración o control del Active Object.

La interfaz permite que el resto del sistema no acceda directamente al estado interno del AO, sino que interactúe con él mediante mensajes.

A nivel de la tarea `task_sys` se utiliza la funcion `send_led_ao()` para cambiar el comportamiento de los leds.

---

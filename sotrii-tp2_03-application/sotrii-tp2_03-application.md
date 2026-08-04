# CESE - Sistemas Operativos de Tiempo Real 

## Trabajo Práctico N°: 2 – Active Object

### TP2 – Actividad 03 – Active Object Button

## Paso 1

Proyecto generado e importado. Compila en STM32CubeIDE.

---

## Paso 2

Archivo de entrega creado

---

## Paso 3

Se implementa una nueva estructura para almacenar el estado del objeto activo, incluyendo textos de depuración y el handle de la tarea particular. Debido a que los eventos relacionados con los botones se procesan en tiempo real no es necesario crear una cola, en su lugar cada evento se reporta al modulo sys.

Se definen funciones de interfaz para controlar el AO de forma encapsulada:

- `open_btn_ao()`: crea la cola de eventos, la registra en FreeRTOS y lanza la tarea Gatekeeper asociada al Active Object Btn. Al igual que ocurre con los LEDs, se pueden crear multiples instancias para controlar un boton en particular.
- `release_btn_ao()`: libera los recursos del AO, eliminando la cola y finalizando la tarea correspondiente.
- `send_btn_ao()`: envía un mensaje compuesto por evento y tiempo, permitiendo que el AO Btn informe a otros módulos del sistema.
- `ioctl_btn_ao()`: queda preparada para futuras operaciones de configuración o control del objeto activo.

A nivel de la tarea `task_bnt` se utiliza la funcion `send_sys_ao()` para reportar el estado de los botones.

---

# CESE - Sistemas Operativos de Tiempo Real 

## Trabajo Práctico N°: 2 – Active Object

### TP2 – Actividad 01 – Active Object Led

## Paso 1

Proyecto generado e importado. Compila en STM32CubeIDE.

---

## Paso 2

Archivo de entrega creado

---

## Paso 3: Analisis del codigo fuente base


Este sistema está diseñado bajo una arquitectura orientada a eventos (**Event-Triggered System**) implementada sobre **FreeRTOS** y la capa de abstracción de hardware **STM32 HAL**.

El flujo principal se basa en la lectura periódica de pulsadores, el procesamiento de sus estados mediante **cartas de estado (Statecharts)** y la propagación de eventos inter-tareas utilizando colas de FreeRTOS (`QueueHandle_t`) para controlar el comportamiento de una serie de LEDs.

---

### 1. Estructura de Datos y Atributos (Archivos `.h` y `.c` de Atributos)

El diseño desacopla la lógica separando las estructuras de datos genéricas en archivos de atributos.

#### `task_btn_attribute.h` y `.c`

* **Modelado del Botón:** Define `btn_id_t` (IDs para `BTN_A` y `BTN_B`), eventos (`EV_BTN_UP`, `EV_BTN_DOWN`) y estados (`ST_BTN_UP`, `ST_BTN_DOWN`).


* **Estructuras:**
* `btn_t`: Guarda el puerto GPIO, pin y el estado de la lectura física.


* `btn_sc_t`: Estructura para el Statechart; realiza el seguimiento del estado actual, los eventos de entrada/salida y el conteo de tiempo (`tick`).


* `h_btn_t`: El manejador (*handler*) que vincula el hardware con su máquina de estados.


* **Instanciación:** En `task_btn.c` se configuran los arreglos `btn` y `btn_sc` para dos pulsadores, asociándolos a sus pines de hardware correspondientes.



#### `task_sys_attribute.h`

* **Lógica Intermedia:** Mapea directamente los eventos del botón a eventos del sistema:


* `EV_SYS_OFF` equivale a `EV_BTN_UP`.


* `EV_SYS_ON` equivale a `EV_BTN_DOWN`.




* Posee un evento propio (`EV_SYS_BLINK`) y tres estados de operación: `ST_SYS_IDLE`, `ST_SYS_ACTIVE_0` y `ST_SYS_ACTIVE_1`.



#### `task_led_attribute.h`

* **Mapeo del Actuador:** Modela tres LEDs (`LED_A`, `LED_B`, `LED_C`).


* Mapea los eventos del sistema directamente hacia los del LED:


* `EV_LED_OFF` = `EV_SYS_OFF`

* `EV_LED_ON` = `EV_SYS_ON`

* `EV_LED_BLINK` = `EV_SYS_BLINK`




---

### 2. Inicialización del Sistema (`app.c`)

El archivo `app.c` se encarga de la puesta a punto del entorno antes de lanzar el planificador de FreeRTOS:

* **Creación de Colas:**
* `h_sys_task_q`: Longitud de 5 elementos. Comunica la tarea de botones con la tarea del sistema (`BTN -> SYS`).


* `h_led_task_q`: Longitud de 1 elemento. Comunica la tarea del sistema con la de LEDs (`SYS -> LED`).




* **Creación de Tareas (`xTaskCreate`):** Inicializa varias tareas. Nota que las tareas de control de hardware (`task_led`, `task_sys`, `task_btn`) corren con prioridad 1 (`tskIDLE_PRIORITY + 1ul`) y reciben como parámetro un puntero a sus respectivos *handlers* de datos (`h_led`, `h_sys`, `h_btn`).

---

### 3. Funcionamiento de las Tareas (`task_*.c`)

Cada una de las tres tareas principales corre de forma periódica dentro de un bucle infinito con un retardo de bloqueo de 50 ms (`vTaskDelay(pdMS_TO_TICKS(50ul))`). Todas ejecutan un esquema de ejecución hasta el fin (**Run to Completion Statechart**).

#### A. Tarea de Botón (`task_btn.c`)

1. **Lectura:** En cada ciclo lee el estado físico del pin mediante `HAL_GPIO_ReadPin`.


2. **Excitación:** Si el botón físico está presionado, asigna `EV_BTN_DOWN`, de lo contrario asigna `EV_BTN_UP` al evento de entrada.


3. **Máquina de Estados (`task_btn_statechart`):**
* **Transiciones:** Si cambia de `ST_BTN_UP` a `ST_BTN_DOWN` (o viceversa), captura el evento de salida, reinicia los contadores de tiempo y **envía el evento sin bloqueo** a la cola del sistema (`h_sys_task_q`).


* Si permanece en el mismo estado, acumula el tiempo transcurrido en incrementos de 50 ms (`DEL_BTN_MIN`).





#### B. Tarea de Sistema (`task_sys.c`)

1. **Consumo de Eventos:** Intenta leer un evento de la cola `h_sys_task_q` sin bloquearse (`TickType_t ZERO`). Si la cola está vacía, define el evento de entrada como `EV_SYS_NONE`.


2. **Máquina de Estados (`task_sys_statechart`):** Actúa como un secuenciador o distribuidor cíclico cada vez que recibe un flanco de presión del botón (`EV_SYS_ON` / `EV_BTN_DOWN`):


* **`ST_SYS_IDLE` + `EV_SYS_ON**` $\rightarrow$ Pasa a `ST_SYS_ACTIVE_0` y envía `EV_SYS_ON` a la cola del LED.


* **`ST_SYS_ACTIVE_0` + `EV_SYS_ON**` $\rightarrow$ Pasa a `ST_SYS_ACTIVE_1` y envía `EV_SYS_BLINK` a la cola del LED.


* **`ST_SYS_ACTIVE_1` + `EV_SYS_ON**` $\rightarrow$ Regresa a `ST_SYS_IDLE` y envía `EV_SYS_OFF` a la cola del LED.





#### C. Tarea de LED (`task_led.c`)

1. **Consumo de Eventos:** Al igual que la tarea de sistema, lee la cola de entrada `h_led_task_q` de manera asincrónica.


2. **Máquina de Estados (`task_led_statechart`):** Controla el estado del hardware luminoso:


* **`EV_LED_OFF` o `EV_LED_ON**`: Configura el pin del LED fijo en apagado o encendido usando `HAL_GPIO_WritePin`.


* **`EV_LED_BLINK`**: Cambia el estado a `ST_LED_BLINK`, inicializa un temporizador de parpadeo a 500 ms (`DEL_LED_BLINK`) y realiza un toggle inmediato en el pin.


* **Permanencia en `ST_LED_BLINK**`: Si no entran nuevos eventos externos (`EV_LED_NONE`), decrementa el campo `tick` de 50 en 50 ms. Cuando el contador llega a `ZERO` (cada 500 ms), invierte el estado físico del LED mediante `HAL_GPIO_TogglePin` y reinicia la cuenta a 500 ms, generando un destello asincrónico perfecto.





---

### Resumen del Flujo de Ejecución

```
Pulsador Presionado 
   └─► task_btn (Periódica, 50ms) -> Detecta flanco
         └─► xQueueSend(h_sys_task_q, EV_BTN_DOWN)
               └─► task_sys (Periódica, 50ms) -> Cambia secuencia activa
                     └─► xQueueSend(h_led_task_q, EV_SYS_BLINK)
                           └─► task_led (Periódica, 50ms) -> Modifica ciclo del LED
                                 └─► HAL_GPIO_TogglePin() (Cada 500ms de forma interna)

```

Este diseño dirigido por eventos evita los retardos bloqueantes tradicionales (`delay`), asegurando que el microcontrolador se mantenga altamente responsivo y permitiendo que FreeRTOS alterne eficientemente entre las tareas.


## Paso 6

Se implementa una nueva estructura para almacenar el estado del objeto activo, esto incluye la cola de los eventos del modulo led, textos de depuración y el handle de la tarea asignada a cada LED

Se implementan funciones de interfaz para operar el Active Object Led de forma controlada:

- `open_led_ao()`: crea la cola de eventos, la registra en FreeRTOS y lanza la tarea encargada de procesar los mensajes. Es posible crear multiples instancias para controlar un LED en particular.
- `release_led_ao()`: libera los recursos asociados al AO, eliminando la cola y finalizando la tarea.
- `send_led_ao()`: envía un evento al AO mediante la cola, permitiendo que otras tareas puedan solicitar cambios en el LED.
- `ioctl_led_ao()`: queda preparada para futuras operaciones de configuración o control del objeto.

Al utilizar el metodo `send_led_ao()` se implementá el patron Gatekeeper donde solo un elemento de la cola se proces a la vez

---
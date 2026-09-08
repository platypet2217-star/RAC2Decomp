#include "types.h"
#include "system.h"     // Necesario para acceder a g_GraphicsSemaphore y g_GraphicsSemaphoreID
#include <SDL2/SDL.h>
#include "graphics.h"   // Necesario para verificar target_fps
#include <string.h>

int ee_memcmp(const void* ptr1, const void* ptr2, size_t num) {
	// En PC, la función estándar 'memcmp' de string.h está optimizada a nivel 
	// de ensamblador moderno (SSE/AVX) y produce exactamente el mismo resultado
	// matemático que el bucle vectorial de la PS2, pero de forma nativa y segura.
	return memcmp(ptr1, ptr2, num);
}

#if !defined(PLATFORM_PS2)
// Estructura de control interna simulada para emular los semáforos del Kernel de Sony en PC
typedef struct {
	s32 count;
	s32 max_count;
} PS2_Simulated_Semaphore;

// Tabla estática de semáforos virtuales para el entorno portátil del port
static PS2_Simulated_Semaphore g_virtual_semaphores[16] = {
	{0, 1}, // ID 0: General / Sistema
	{1, 1}, // ID 1: IO Lock Sema
	{1, 1}, // ID 2: IO Wait Sema
	{1, 1}  // ID 3: IO DMA Sema
};
#endif

/**
 * @brief Reserva y crea un nuevo objeto semáforo en el Kernel de la PlayStation 2...
 */
s32 sceCreateSema(void) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 64 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la asignación devolviendo un ID autoincremental
	static s32 virtual_sema_counter = 4;
	s32 nuevo_id = virtual_sema_counter++;

	// Si el sistema gráfico aún no ha inicializado su semáforo, 
	// aprovechamos esta primera llamada para enlazar el ID moderno.
	if (g_GraphicsSemaphoreID == SYS_SEMAPHORE_INVALID) {
		g_GraphicsSemaphore = SDL_CreateSemaphore(0);
		g_GraphicsSemaphoreID = nuevo_id;
	}

	return nuevo_id;
#endif
}

/**
 * @brief Pausa la ejecución del hilo actual en el Kernel de la PS2...
 */
s32 sceWaitSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto suspende la CPU mediante ensamblador inline:
	// __asm__ volatile("li $v0, 68 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la espera de forma pasiva y segura.

	// Si el usuario configuró los FPS como '0' (ilimitados), no bloqueamos.
	// De lo contrario, respetamos el semáforo para sincronizar el motor.
	if (g_GraphicsCanvasData.target_fps != 0.0f) {
		// En un motor real de PS2, sema_id suele mapearse a un semáforo específico.
		// Dado que este es el semáforo principal de sincronización gráfica:
		if (g_GraphicsSemaphore != NULL) {
			SDL_SemWait(g_GraphicsSemaphore);
		}
	}

	return 0;
#endif
}

/**
 * @brief Envía una señal a un semáforo del Kernel para incrementar su conteo y despertar hilos en...
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x42 MIPS Syscall) (PAL)
 *
 * @param sema_id Identificador único del semáforo al que se le enviará la señal de liberación.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceSignalSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se ejecuta mediante ensamblador inline de MIPS:
	// __asm__ volatile("li $v0, 66 \n syscall");
	return 0;
#else
	// Para el port moderno a PC, emulamos la liberación de forma activa.
	// Si el ID corresponde al semáforo gráfico, notificamos al sistema moderno
	if (sema_id == g_GraphicsSemaphoreID && g_GraphicsSemaphore != NULL) {
		SDL_SemPost(g_GraphicsSemaphore);
	}

	return 0;
#endif
}

/**
 * @brief Verifica de forma no bloqueante si un semáforo está disponible en el Kernel de la PS2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x45 MIPS Syscall) (PAL)
 *
 * @param sema_id Identificador único del semáforo a consultar.
 * @return s32 El conteo actual del semáforo si tuvo éxito, o un valor negativo si está bloqueado.
 */
s32 scePollSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se traduce a la instrucción nativa:
	// __asm__ volatile("li $v0, 69 \n syscall"); // 69 en decimal es 0x45
	return 0;
#else
	// Para el port de PC, validamos primero que el ID sea correcto dentro de nuestro rango virtual
	if (sema_id == SYS_SEMAPHORE_INVALID || sema_id >= 16) {
		return -113; // ID inválido del SDK de Sony
	}

	// Buscamos cuál de nuestros semáforos de SDL2 quiere inspeccionar el motor
	SDL_sem* target_sem = NULL;
	if (sema_id == g_GraphicsSemaphoreID)    target_sem = g_GraphicsSemaphore;
	else if (sema_id == g_RenderSemaphoreID_A) target_sem = g_RenderSemaphore_A;
	else if (sema_id == g_RenderSemaphoreID_B) target_sem = g_RenderSemaphore_B;

	if (target_sem != NULL) {
		// SDL_SemTryWait intenta tomar el semáforo de inmediato:
		// Retorna 0 si estaba libre (éxito). Retorna SDL_MUTEX_TIMEDOUT si estaba ocupado.
		if (SDL_SemTryWait(target_sem) == 0) {
			return 0; // Éxito: El semáforo estaba libre y lo tomamos sin bloquear
		}
		else {
			return -489; // Código oficial de Sony para "Semáforo bloqueado/Cerrado" (Signaled/Wait state)
		}
	}

	// Si es un semáforo secundario que aún no enlazamos, devolvemos éxito por defecto para no colgar el flujo
	return 0;
#endif
}

/**
 * @brief Destruye y libera un objeto semáforo de la memoria protegida del Kernel de la PS2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x41 MIPS Syscall) (PAL)
 *
 * @param sema_id Identificador único del semáforo de hardware que se va a eliminar.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceDeleteSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 65 \n syscall"); // syscall 0x41 nativa
	return 0;
#else
	// Para el port a PC, emulamos la destrucción liberando activamente los recursos de SDL2.

	if (sema_id == g_GraphicsSemaphoreID && g_GraphicsSemaphore != NULL) {
		SDL_DestroySemaphore(g_GraphicsSemaphore);
		g_GraphicsSemaphore = NULL;
		g_GraphicsSemaphoreID = SYS_SEMAPHORE_INVALID;
	}
	else if (sema_id == g_RenderSemaphoreID_A && g_RenderSemaphore_A != NULL) {
		SDL_DestroySemaphore(g_RenderSemaphore_A);
		g_RenderSemaphore_A = NULL;
		g_RenderSemaphoreID_A = SYS_SEMAPHORE_INVALID;
	}
	else if (sema_id == g_RenderSemaphoreID_B && g_RenderSemaphore_B != NULL) {
		SDL_DestroySemaphore(g_RenderSemaphore_B);
		g_RenderSemaphore_B = NULL;
		g_RenderSemaphoreID_B = SYS_SEMAPHORE_INVALID;
	}
	else {
		// Evitamos advertencias del compilador si es un ID de semáforo genérico
		(void)sema_id;
	}

	return 0; // Confirmamos el borrado exitoso
#endif
}

/**
 * @brief Envía una señal de liberación a un semáforo de forma segura desde un contexto de interrupción.
 * Dirección original en Ghidra: Sector de Internal Hooks (Syscall MIPS -67 / 0xFFFFFFFFFFFFFFBD) (PAL)
 *
 * @param sema_id Identificador único del semáforo asignado por el Kernel al canal.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 iSignalSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se ejecuta invocando el hook del Kernel mediante assembly:
	// __asm__ volatile("li $v0, -67 \n syscall");
	return 0;
#else
	// Para el port moderno a PC, al no existir interrupciones físicas de MIPS,
	// emulamos la liberación llamando a la misma lógica activa del semáforo:
	if (sema_id == g_GraphicsSemaphoreID && g_GraphicsSemaphore != NULL) {
		SDL_SemPost(g_GraphicsSemaphore);
	}

	return 0;
#endif
}

int ee_atoi(const char* str) {
	if (str == NULL) return 0;

	// En PC nativo, 'atoi' realiza la conversión en base 10 de forma idéntica
	// al comportamiento esperado por el truncado de 32 bits del Emotion Engine.
	return atoi(str);
}

/**
 * @brief Despierta un hilo de ejecución específico que se encontraba en estado de suspensión en el Kernel de la PS2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x33 MIPS Syscall) (PAL)
 *
 * @param thread_id Identificador único del hilo de ejecución que se desea reactivar.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceWakeupThread(s32 thread_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 51 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la reactivación de hilos de forma pasiva.
	// Como la multitarea moderna de los sistemas operativos gestiona los hilos de fondo,
	// confirmamos la señal de reactivación inmediatamente para mantener el flujo limpio:
	(void)thread_id;
	return 0;
#endif
}

/**
 * @brief Reactiva y despierta de forma segura un hilo de ejecución suspendido desde un contexto de interrupción (ISR).
 * Dirección original en Ghidra: Sector de Internal Hooks (Syscall MIPS -52 / 0xFFFFFFFFFFFFFFCC) (PAL)
 *
 * @param thread_id Identificador único del hilo de ejecución que se va a despertar de urgencia.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 iWakeupThread(s32 thread_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se ejecuta invocando el hook del Kernel mediante assembly:
	// __asm__ volatile("li $v0, -52 \n syscall");
	return 0;
#else
	// Para el port moderno a PC, emulamos la reanudación de forma pasiva
	// confirmando la señal de éxito inmediato del hilo:
	(void)thread_id;
	return 0;
#endif
}

/**
 * @brief Consulta el estado actual de un hilo de ejecución específico en el Kernel de la PlayStation 2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x30 MIPS Syscall) (PAL)
 *
 * @param thread_id Identificador único del hilo a inspeccionar.
 * @param status_ptr Puntero a la estructura donde el Kernel vuelca el estado del hilo (sceThreadStatus).
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceReferThreadStatus(s32 thread_id, void* status_ptr) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 48 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la consulta devolviendo éxito inmediato (0).
	// Esto le indica al motor de Insomniac que los hilos están corriendo de forma óptima

	// Evitamos advertencias de parámetros no utilizados en el compilador moderno de PC
	(void)thread_id;

	if (status_ptr != NULL) {
		// En la PS2 real, la estructura limpia tiene un campo de estado (status).
		// El valor '1' típicamente representa el estado "RUN" (Corriendo).
		// Llenamos los primeros 4 bytes con 1 de forma segura por si el juego valida que el hilo no esté muerto.
		*(s32*)status_ptr = 1;
	}

	return 0;
#endif
}

/**
 * @brief Vacía la caché de instrucciones (I-Cache) de la CPU para asegurar la coherencia antes de ejecutar código.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (100 MIPS Syscall) (PAL)
 *
 * @param cache_type Tipo de operación de vaciado (usualmente 0 para la I-Cache general).
 * @return s32 Código de estado del Kernel (0 para éxito).
 */
s32 sceFlushCache(s32 cache_type) {
#if defined(PLATFORM_PS2)
	// En la PlayStation 2 real, esto se ejecuta mediante la instrucción inline:
	// __asm__ volatile("li $v0, 100 \n syscall");
	return 0;
#else
	// Para el port moderno a PC, el hardware x86_64/ARM maneja de forma automática 
	// la coherencia de la caché de instrucciones sin necesidad de forzar flushes manuales:
	(void)cache_type;
	return 0;
#endif
}

/**
 * @brief Recupera el identificador único (ID) del hilo de ejecución que se encuentra activo en el Kernel de la PS2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x2F MIPS Syscall) (PAL)
 *
 * @return s32 El ID del hilo de ejecución actual (número positivo), o un valor negativo si ocurre un error.
 */
s32 sceGetThreadId(void) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 47 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la llamada devolviendo un ID de hilo lógico fijo (ej. 1 para el hilo principal).
	// Esto es completamente seguro y compatible ya que en PC las tareas IO se ejecutan de largo:
	return 1;
#endif
}

/**
 * @brief Programa una alarma por interrupción basada en tiempo dentro del Kernel de la PlayStation 2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (252 MIPS Syscall / 0xFC) (PAL)
 *
 * @param microseconds Tiempo exacto en microsegundos antes de disparar la alarma por hardware.
 * @param alarm_callback Puntero a la función que actuará como manejador de la interrupción al expirar el tiempo.
 * @param callback_arg Argumento opcional de control que se le pasará a la función manejadora.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceSetAlarm(u32 microseconds, void* alarm_callback, void* callback_arg) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se ejecuta mediante la instrucción inline:
	// __asm__ volatile("li $v0, 252 \n syscall");
	return 0;
#else
	// Para el port a PC, dado que el sistema de archivos del sistema operativo moderno
	// resuelve las transacciones instantáneamente sin esperas físicas de hardware de tarjetas,
	// la alarma virtual confirma el agendamiento de forma inmediata:
	(void)microseconds;
	(void)alarm_callback;
	(void)callback_arg;
	return 0;
#endif
}

/**
 * @brief Suspende voluntariamente la ejecución del hilo de control activo en el Kernel de la PS2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x32 MIPS Syscall) (PAL)
 *
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceSleepThread(void) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 50 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la suspensión de forma pasiva devolviendo éxito inmediato.
	// Dado que el sistema de archivos de PC no requiere retardos de hardware físicos de 8MB,
	// el hilo virtual fluye de largo sin congelar o ralentizar la tasa de cuadros del motor:
	return 0;
#endif
}

void sceFlushCache(int mode) {
#if defined(PLATFORM_PS2)
	// En la consola real invoca la syscall 0x80
	// __asm__ volatile("li $v0, 128 \n syscall"); 
#else
	// Para PC moderno, los procesadores x86_64/ARM manejan la coherencia de 
	// caché por hardware automáticamente. Dejamos esta función vacía de forma 
	// segura para no penalizar el rendimiento ni exigir componentes innecesariamente.
	(void)mode;
#endif
}
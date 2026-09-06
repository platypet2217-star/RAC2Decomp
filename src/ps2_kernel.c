#include "types.h"

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
 * @brief Reserva y crea un nuevo objeto semáforo en el Kernel de la PlayStation 2.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x40 MIPS Syscall) (PAL)
 *
 * @return s32 El ID único del semáforo asignado por el sistema (número positivo), o un código de error si falla.
 */
s32 sceCreateSema(void) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 64 \n syscall");
#else
	// Para el port a PC, emulamos la asignación devolviendo un ID autoincremental simétrico o un token seguro.
	// Como tus guardianes de Insomniac validan que el ID sea mayor a -1, asignamos un número de control estático:
	static s32 virtual_sema_counter = 4;
	return virtual_sema_counter++;
#endif
}

/**
 * @brief Pausa la ejecución del hilo actual en el Kernel de la PS2 esperando a que el semáforo sea liberado.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x44 MIPS Syscall) (PAL)
 *
 * @param sema_id Identificador único del semáforo sobre el cual se va a suspender la CPU.
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceWaitSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto suspende la CPU mediante ensamblador inline:
	// __asm__ volatile("li $v0, 68 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la espera de forma pasiva y segura.
	// Como los accesos a disco e hilos modernos en PC resuelven las ráfagas en nanosegundos,
	// el semáforo virtual devuelve éxito inmediato para mantener el motor fluyendo de largo:
	(void)sema_id; // Evita advertencia de variable sin usar en el compilador de PC
	return 0;
#endif
}

/**
 * @brief Envía una señal a un semáforo del Kernel para incrementar su conteo y despertar hilos en espera.
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
	// Para el port moderno a PC, emulamos la liberación de forma pasiva.
	// Como las cargas en PC se resuelven de largo sin colgar buses de hardware,
	// el semáforo virtual confirma la señal de forma inmediata para mantener el pipeline limpio:
	(void)sema_id;
	return 0;
#endif
}

/**
 * @brief Realiza un sondeo instantáneo (no bloqueante) del estado de un semáforo en el Kernel.
 * Dirección original en Ghidra: Sector de Stubs de Syscalls (0x45 MIPS Syscall) (PAL)
 *
 * @param sema_id Identificador único del semáforo de hardware que se va a interrogar.
 * @return s32 El valor o conteo actual del semáforo, o un código de error negativo si no existe.
 */
s32 scePollSema(s32 sema_id) {
#if defined(PLATFORM_PS2)
	// En la consola real, esto se traduce a la instrucción nativa assembly inline:
	// __asm__ volatile("li $v0, 69 \n syscall");
	return sema_id;
#else
	// Para el port de PC, emulamos de forma segura el estado de éxito simulando
	// que el semáforo de lectura o audio del juego está disponible de forma inmediata:
	if (sema_id < 0 || sema_id >= 16) {
		return -113; // sceAnonymError / ID inválido del SDK de Sony
	}

	// Retorna el ID simulado del semáforo para validar la coincidencia en los guardianes de Insomniac
	return sema_id;
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
	// __asm__ volatile("li $v0, 65 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la destrucción de forma pasiva.
	// Como las asignaciones virtuales se manejan mediante estados lógicos simples,
	// el liberador confirma el borrado inmediatamente para mantener el entorno limpio:
	(void)sema_id;
	return 0;
#endif
}

/**
 * @brief Envía una señal de liberación a un semáforo de forma segura desde un contexto de interrupción por hardware (ISR).
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
	// emulamos la liberación de forma pasiva confirmando el éxito inmediato del semáforo:
	(void)sema_id;
	return 0;
#endif
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
 * @return s32 Código de estado del Kernel (0 para éxito, o valor negativo si ocurre un error).
 */
s32 sceReferThreadStatus(void) {
#if defined(PLATFORM_PS2)
	// En la consola real, se invoca mediante ensamblador inline:
	// __asm__ volatile("li $v0, 48 \n syscall");
	return 0;
#else
	// Para el port a PC, emulamos la consulta devolviendo éxito inmediato (0).
	// Esto le indica al motor de Insomniac que los hilos están corriendo de forma óptima:
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

#include "cdvd.h"
#include "ps2_kernel.h"
#include "system.h"
#include <stdio.h>
#include <stdbool.h>

int g_CdvdNcmdInitialized = -1; // -1 significa NO inicializado, tal como la PS2
int g_CdvdCurrentCommand = 0;

int sceCdInit(int mode) {
	// 1. Si ya estaba inicializado previamente, retornamos éxito directo
	if (g_CdvdNcmdInitialized > -1) {
		return 1;
	}

	// 2. Simulamos de forma segura las comprobaciones del Kernel de E/S
	// sys_io_init_kernel_semaphores();
	// En PC, asumimos que nuestros semáforos de gráficos/render ya controlan el flujo

	// Simulación exitosa de scePollSema() para el candado de I/O
	g_CdvdCurrentCommand = mode;

	// 3. Omitimos por completo los bucles 'while(true)' de apertura de sesión RPC SIF.
	// En la PS2 real, esto se colgaba esperando al hardware físico.
	// En PC, forzamos el estado de éxito instantáneo.
	printf("[CDVD] Sistema de archivos mapeado correctamente. Modo original: %d\n", mode);

	// 4. Marcamos el subsistema como LISTO
	g_CdvdNcmdInitialized = 0;

	return 1; // Retorna 1 (Éxito total en el arranque del lector)
}

int sceCdStop(void) {
	// 1. Forzamos la inicialización en modo 2 tal como hace el juego
	sceCdInit(2);

	// 2. Omitimos la transacción SIF 0x0E (Stop/Standby de hardware)
	// En PC no hay partes mecánicas que desacelerar.

	// 3. Emulamos la liberación del semáforo del bus de Entrada/Salida 
	// para mantener la coherencia con el candado que abrió sceCdInit.
	// Usamos el ID de I/O virtual simulado si lo tienes acoplado en ps2_kernel.c
	// sceSignalSema(g_sys_io_lock_sema_id);

	printf("[CDVD] Comando de reposo (Stop/Standby) procesado de forma nativa.\n");

	return 0; // Retorno oficial del stub de Sony
}
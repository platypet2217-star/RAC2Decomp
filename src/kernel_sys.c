// src/kernel_sys.c
#include "types.h"

// Variables globales estimadas de la estructura de la lista en memoria RAM (0x0013CAC0)
u32 g_list_root_param = 0;
u32 g_list_element_count = 0;
void* g_list_head_ptr = NULL;
void* g_list_tail_ptr = NULL;
u32 g_list_sentinel_node = 0; // Representa a DAT_0013cad0

/**
 * @brief Inicializa una lista enlazada global o cola de administración de memoria del motor.
 * Configura el nodo raíz, inicializa el contador a 0 y enlaza los punteros Head y Tail.
 * Dirección original en Ghidra: 0x0011BAC8 (PAL)
 *
 * @param param_1 Parámetro de configuración inicial o capacidad (registro a0)
 * @return void* Puntero a la cabecera de la estructura global inicializada.
 */
void* sys_queue_initialize(u32 param_1) {
	g_list_root_param = param_1;
	g_list_element_count = 0; // Inicializa el tamaño/contador actual en cero

	// Tanto el Head como el Tail apuntan inicialmente al nodo base vacío (DAT_0013cad0)
	g_list_head_ptr = &g_list_sentinel_node;
	g_list_tail_ptr = &g_list_sentinel_node;

	return &g_list_root_param;
}

// Definiciones ficticias de registros del sistema para mantener la compatibilidad del código
extern u32 Status;
void DI(void);
void SYNC(int type);

// Variables de control de red del sistema
s32 g_deci2_channel_status = -1;
u32 g_deci2_var1 = 0;
u32 g_deci2_var2 = 0;
u32 g_deci2_var3 = 0;
void* g_deci2_buffer_a = NULL;
void* g_deci2_buffer_b = NULL;
void* g_deci2_queue_ptr = NULL;

// Variables ficticias del buffer de cabecera de red
u16 g_net_packet_size = 0;
u8  g_net_packet_id1 = 0;
u8  g_net_packet_id2 = 0;
u8  g_net_packet_flags = 0;
u16 g_net_packet_type = 0;

// Declaraciones de funciones requeridas
void FlushCache(void);
s32 sys_deci2_call_channel_a(void);
void* sys_queue_initialize(u32 capacity);

void EI(void);

// Control de semáforo e impresión
s32 g_deci2_print_mutex = 0;
s32 g_deci2_packet_len = 0;
char g_deci2_print_buffer[256]; // Representa el espacio físico en DAT_2013cc0c

// Prototipos necesarios
bool kernel_system_sync_guard(void);
bool kernel_system_sync_release(void);
void sys_deci2_call_wrapper(void);
void sys_deci2_call_channel_c(void);

/**
 * @brief Transmite un mensaje de registro formateado al sistema de depuración remota.
 * Traduce caracteres de salto de línea e interactúa con los canales de red DECI2.
 * Dirección original en Ghidra: 0x0011BCD0 (PAL)
 *
 * @param p_message Cadena de caracteres a imprimir (param_1)
 * @param max_len Longitud máxima a procesar (param_2)
 * @return int Cantidad de caracteres transmitidos de forma exitosa, o -1 en caso de bloqueo/error.
 */
s32 sys_deci2_print_log(const char* p_message, s32 max_len) {
	s32 total_chars_written = 0;
	s32 status_code = -1;

	if (g_deci2_print_mutex == 0) {
		kernel_system_sync_guard();
		g_deci2_print_mutex = 1;
		status_code = 0;

		// Bucle de formateo y copia de caracteres (Máximo 256 bytes)
		while (total_chars_written < 256 && max_len > 0) {
			max_len--;
			char current_char = *p_message;

			// Conversión de salto de línea de Unix (\n) a formato de consola (\r)
			if (current_char == '\n') {
				g_deci2_print_buffer[total_chars_written] = '\r';
				total_chars_written++;
				if (total_chars_written >= 256) break;
			}

			g_deci2_print_buffer[total_chars_written] = current_char;
			total_chars_written++;
			p_message++;
			status_code++;
		}

		// Configura el tamaño del paquete sumando la cabecera base (12 bytes / 0xc)
		g_deci2_packet_len = total_chars_written + 0xC;

		// Simulación del envío de datos a través de las llamadas del sistema
		sys_deci2_call_wrapper();

		// En PC o plataformas modernas, redirigimos este log directamente a la consola nativa
#ifndef PLATFORM_PS2
		g_deci2_print_buffer[total_chars_written] = '\0'; // Asegurar cierre de string
		// printf("[PS2 LOG]: %s", g_deci2_print_buffer); // Descomentar para ver logs en el port
		g_deci2_print_mutex = 0;
#endif

		// El juego original espera activamente a que el hardware del canal C limpie el mutex
		// while (g_deci2_print_mutex != 0) { sys_deci2_call_channel_c(); }

		kernel_system_sync_release();
	}

	return status_code;
}


/**
 * @brief Rutina de liberación y activación de interrupciones del procesador Emotion Engine.
 * Reactiva los hilos del sistema de la PS2 tras una operación crítica de sincronización.
 * Dirección original en Ghidra: 0x0011F628 (PAL)
 *
 * @return bool Devuelve el estado de la bandera de diagnóstico del procesador.
 */
bool kernel_system_sync_release(void) {
	// Reactiva las interrupciones generales en el hardware de la PlayStation 2
	EI();

	// Evalúa y retorna el estado del bit 16 del registro Status del Coprocesador 0
	return (Status & 0x10000) != 0;
}

/**
 * @brief Inicializa el subsistema de comunicación de depuración DECI2 del motor.
 * Configura la cola de transmisión y escribe las cabeceras de protocolo en memoria.
 * Dirección original en Ghidra: 0x0011BE20 (PAL)
 *
 * @return bool Devuelve true si el canal de red se inicializó correctamente, false en caso contrario.
 */
bool sys_deci2_subsystem_init(void) {
	// 1. Limpieza de las cachés del procesador central
#ifdef PLATFORM_PS2
	FlushCache();
#endif

	// 2. Intenta abrir y verificar el estado del canal de depuración
	// Nota: sys_deci2_call_channel_a() en nuestro port portable devuelve un estado pasivo (ej. 0)
	g_deci2_channel_status = sys_deci2_call_channel_a();

	bool is_channel_valid = (-1 < g_deci2_channel_status);

	if (is_channel_valid) {
		// Inicializa variables de estado internas
		g_deci2_var1 = 0;
		g_deci2_var2 = 0;
		g_deci2_var3 = 0;

		// Asignación de los buffers globales del sistema de diagnóstico
		g_deci2_buffer_a = (void*)0x2013CD40;
		g_deci2_buffer_b = (void*)0x2013CC00;

		// Escribe los valores de la cabecera de protocolo de red del motor de Insomniac Games
		g_net_packet_size = 0x210; // Tamaño del paquete de red
		g_net_packet_id1 = 0x45;  // Identificador de protocolo ('E')
		g_net_packet_id2 = 0x48;  // Identificador de protocolo ('H')
		g_net_packet_flags = 0;
		g_net_packet_type = 0;

		// 3. Inicializa la cola dinámica de mensajes con capacidad para 256 elementos (0x100)
		g_deci2_queue_ptr = sys_queue_initialize(0x100);
	}

	return is_channel_valid;
}


/**
 * @brief Rutina de bloqueo y sincronización del procesador Emotion Engine.
 * Monitorea el estado de las banderas del procesador mediante un spinlock seguro.
 * Dirección original en Ghidra: 0x0011F5E0 (PAL)
 *
 * @return bool Devuelve el estado final de la bandera de diagnóstico del procesador.
 */
bool kernel_system_sync_guard(void) {
	// 0x10000 corresponde a una bandera de estado de interrupción/diagnóstico en el Coprocesador 0 de MIPS
	if ((Status & 0x10000) != 0) {
		do {
			DI();        // Desactivar interrupciones de hardware en la PS2
			SYNC(0x10);  // Forzar la sincronización del pipeline de datos del procesador
		} while ((Status & 0x10000) != 0); // Repetir hasta que el hardware se estabilice

		return (Status & 0x10000) != 0;
	}

	return false;
}

/**
 * @brief Calcula la longitud de una cadena de caracteres (Strlen).
 * Equivalente portátil a la rutina optimizada bitwise del Emotion Engine.
 * Dirección original en Ghidra: 0x001157AC (PAL)
 *
 * @param str Puntero a la cadena de texto de entrada.
 * @return int El número de caracteres en la cadena (sin contar el nulo terminal).
 */
int ee_strlen(const char* str) {
	if (str == NULL) {
		return 0;
	}

	const char* s = str;

	// El motor de PS2 utiliza operaciones bitwise paralelas (0xfefefefefefefeff)
	// para escanear bloques de memoria de 8 y 16 bytes simultáneamente.
	// Conceptualmente, su comportamiento lógico exacto es avanzar hasta encontrar '\0':
	while (*s != '\0') {
		s++;
	}

	// Devuelve la diferencia de direcciones, es decir, la longitud exacta
	return (int)(s - str);
}

/**
 * @brief Compara dos cadenas de caracteres (Strcmp).
 * Equivalente portátil a la rutina optimizada bitwise del procesador Emotion Engine.
 * Dirección original en Ghidra: 0x00115544 (PAL)
 *
 * @param str1 Primera cadena a comparar (param_1)
 * @param str2 Segunda cadena a comparar (param_2)
 * @return int 0 si son idénticas, un valor negativo si str1 < str2, o positivo si str1 > str2.
 */
int ee_strcmp(const char* str1, const char* str2) {
	if (str1 == NULL || str2 == NULL) {
		return (str1 == str2) ? 0 : (str1 == NULL ? -1 : 1);
	}

	const unsigned char* s1 = (const unsigned char*)str1;
	const unsigned char* s2 = (const unsigned char*)str2;

	// En la PS2 real, si la memoria está alineada, se ejecutan restas multimedia en paralelo.
	// Conceptualmente, su comportamiento lógico equivale a iterar byte por byte:
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}

	// Devuelve la diferencia matemática exacta entre los caracteres donde difieren
	return (int)*s1 - (int)*s2;
}

/**
 * @brief Decodifica un carácter multi-byte del motor de texto según la codificación activa.
 * Soporta fallbacks de codificación JIS/SJIS heredados del desarrollo original.
 * Dirección original en Ghidra: 0x00115FC0 (PAL)
 *
 * @param p_localeContext Contexto de localización e idioma activo (param_1)
 * @param p_outChar Puntero donde se almacena el carácter decodificado final (param_2)
 * @param p_srcString Puntero a la cadena de texto a leer (param_3)
 * @param max_bytes Límite de bytes seguros a procesar (param_4)
 * @param p_state Estado de control de secuencia de escape (param_5)
 * @return int Cantidad de bytes leídos para el carácter, o -1 en caso de error.
 */
s32 txt_decode_multibyte_char(u8* p_localeContext, u32* p_outChar, const u8* p_srcString, u32 max_bytes, s32* p_state) {
	if (p_srcString == NULL) {
		return 0;
	}

	// Comprobación de seguridad básica del motor
	if (max_bytes == 0) {
		return -1; // Equivale al 0xffffffff retornado en PS2
	}

	// El motor original de PS2 realiza comparaciones con "C-SJIS", "C-EUCJP" y "C-JIS"
	// utilizando ee_strcmp para decidir si procesa caracteres de doble byte.
	// Para efectos funcionales en el port de la región europea (PAL):

	u8 lead_byte = *p_srcString;

	// Si no es un formato especial asiático, procesa como carácter estándar de 1 byte
	if (p_outChar != NULL) {
		*p_outChar = (u32)lead_byte;
	}

	return (lead_byte != 0) ? 1 : 0;
}

/**
 * @brief Envoltorio de paso para la llamada del sistema de depuración DECI2 de Sony.
 * Utilizada originalmente en los kits de desarrollo (PS2 TOOL) para enviar registros al PC del programador.
 * Dirección original en Ghidra: 0x0011B9C8 (PAL)
 */
void sys_deci2_call_wrapper(void) {
	// En las consolas comerciales y emuladores modernos, esta llamada no tiene efecto funcional.
	// Para el port nativo, se documenta y se deja como un retorno directo vacío (Stub).
	return;
}

/**
 * @brief Segundo envoltorio de paso para el canal de depuración DECI2 de Sony.
 * Dirección original en Ghidra: 0x0011B978 (PAL)
 */
void sys_deci2_call_channel_a(void) {
	// Retorno directo pasivo (Stub) para compatibilidad en el port nativo.
	return;
}

/**
 * @brief Tercer envoltorio de paso para el canal de depuración DECI2 de Sony.
 * Dirección original en Ghidra: 0x0011B9F8 (PAL)
 */
void sys_deci2_call_channel_c(void) {
	// Retorno directo pasivo (Stub) para compatibilidad estructural en el port nativo.
	return;
}

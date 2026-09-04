// src/hud_ammo.c
#include "types.h"

// Definición funcional de la función interna que descubriste vinculando strings
void hud_register_widget_asset(void* widget_struct, const char* asset_name, long param_3, ...);
void* hud_allocate_or_get_node(int* source, ...);
u32 hud_initialize_subsystem(u32 size, long address);

/**
 * @brief Inicializa o actualiza el estado y coordenadas de la interfaz de munición.
 * Dirección original en Ghidra: 0x0034D490 (PAL)
 */
void hud_ammo_update_or_init(u32* p_hudState, s32 p_ammoData, long param_3) {

	// Las líneas 192 y 193 de Ghidra limpian flags de estado específicos en el HUD
	p_hudState[0x567] = 0;
	p_hudState[0x568] = 0;

	// --- REGISTRO DE WIDGETS VISUALES DEL HUD ---

	// Línea 195: Inicializa el fondo del HUD usando la string "BackAmmo"
	hud_register_widget_asset(p_hudState, (const char*)0x001ae6d8, param_3);

	// Línea 202: Inicializa el borde usando "OutlineAmmo"
	// piStack_14c corresponde a un desplazamiento interno en el stack para este componente
	u32* widget_outline = p_hudState + 0x13;
	hud_register_widget_asset(widget_outline, (const char*)0x001ae6e8, param_3);

	// Línea 206: Inicializa el tercer componente (probablemente texto o icono)
	u32* widget_third = p_hudState + 0x26;
	hud_register_widget_asset(widget_third, (const char*)0x001ae6f8, param_3);


	// --- LÓGICA DE CONDICIÓN DE PARÁMETROS (Línea 248) ---
	// Si el contexto externo (param_3) es válido, inicializa sus matrices/vectores a 0
	if (param_3 != 0) {
		// FUN_00338b98 obtiene el nodo o componente de transformación de coordenadas
		int* node_ptr = (int*)hud_allocate_or_get_node((int*)param_3);

		// FUN_00338b00 reserva espacio de memoria para almacenar datos dinámicos (4 words)
		u32 storage_address = hud_initialize_subsystem(0x10, (long)node_ptr);
		u32* vector_data = (u32*)storage_address;

		// Asigna el bloque inicializado al estado global del HUD
		p_hudState[0x201] = (u32)vector_data;

		// Inicialización a cero de un Vector4 o estructura matemática de transformación
		vector_data[0] = 0; // X o Red
		vector_data[1] = 0; // Y o Green
		vector_data[2] = 0; // Z o Blue
		vector_data[3] = 0; // W o Alpha
	}
}

/**
 * @brief Busca un identificador específico dentro de una estructura de tabla indexada.
 * Operación matemática en un arreglo con saltos de 8 bytes (Estructura de pares Clave/Valor).
 * Dirección original en Ghidra: 0x00338AA8 (PAL)
 *
 * @param table_ptr Puntero a la estructura de la tabla base.
 * @param target_id ID o clave que estamos buscando.
 * @return s32 El valor asociado al ID encontrado, o 0 si no existe/se sale de los límites.
 */
s32 game_lookup_id_in_table(u8* table_ptr, s32 target_id) {
	s32 index = 0;

	// El offset +0x18 almacena la cantidad máxima de elementos válidos en la tabla
	s32 total_elements = *(s32*)(table_ptr + 0x18);

	if (0 < total_elements) {
		index = 1;

		// Optimización del motor: Comprobar directamente el primer elemento (+0x1c)
		if (*(s32*)(table_ptr + 0x1c) == target_id) {
			index = *(s32*)(table_ptr + 0x20); // Devuelve el valor asociado en +0x20
		}
		else {
			// Bucle de búsqueda lineal (do-while)
			do {
				if (*(s32*)(table_ptr + 0x18) <= index) {
					return 0; // Fuera de los límites de la tabla, no encontrado
				}

				// Estructura de par Clave-Valor de 8 bytes: 4 bytes para ID, 4 bytes para Datos
				s32* pair_ptr = (s32*)(index * 8 + (table_ptr + 0x1c));
				index++;

				if (*pair_ptr == target_id) {
					return pair_ptr[1]; // Devuelve el valor contiguo en memoria
				}
			} while (1);
		}
	}

	return index;
}

/**
 * @brief Obtiene un valor de configuración mediante búsqueda e inicializa el campo del HUD.
 * Dirección original en Ghidra: 0x00338070 (PAL)
 */
void hud_set_state_from_lookup(u8* p_hudState, u8* table_ptr, s32 target_id) {
	// Realiza la búsqueda en la tabla lógica
	s32 result_value = game_lookup_id_in_table(table_ptr, target_id);

	// Almacena el resultado en el offset de configuración +0x40 del componente del HUD
	*(s32*)(p_hudState + 0x40) = result_value;
}

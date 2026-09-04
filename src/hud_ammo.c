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
	u32* widget_outline = p_hudState + 0x13;
	hud_register_widget_asset(widget_outline, (const char*)0x001ae6e8, param_3);

	// Línea 206: Inicializa el tercer componente (probablemente texto)
	u32* widget_third = p_hudState + 0x26;
	hud_register_widget_asset(widget_third, (const char*)0x001ae6f8, param_3);

	// Línea 212: Primer borrado de memoria detectado (Limpia 24 bytes en el offset 0x39)
	ee_memset((u8*)p_hudState + 0x39, 0, 0x18);

	// [Aquí iría el Widget 3 ("AmmoIcon") que se inicializa en las líneas 219-222]
	// [Aquí iría el Widget 4 que se inicializa en las líneas 225-229]
	// [Aquí iría el Widget 5 que se inicializa en las líneas 230-234]

	// Línea 235: Segundo borrado de memoria detectado (Limpia 24 bytes en el offset 0x8b)
	ee_memset((u8*)p_hudState + 0x8b, 0, 0x18);


	// --- LÓGICA DE CONDICIÓN DE PARÁMETROS (Línea 248) ---
	if (param_3 != 0) {
		int* node_ptr = (int*)hud_allocate_or_get_node((int*)param_3);
		u32 storage_address = hud_initialize_subsystem(0x10, (long)node_ptr);
		u32* vector_data = (u32*)storage_address;

		p_hudState[0x201] = (u32)vector_data;

		vector_data[0] = 0;
		vector_data[1] = 0;
		vector_data[2] = 0;
		vector_data[3] = 0;
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

/**
 * @brief Asigna cuatro valores de 32 bits de forma consecutiva en una estructura de datos.
 * Comportamiento estándar para configurar vectores espaciales (X, Y, Z, W) o colores (R, G, B, A).
 * Dirección original en Ghidra: 0x00337B18 (PAL)
 *
 * @param val1 Primer componente (ej. coordenada X o canal Rojo)
 * @param val2 Segundo componente (ej. coordenada Y o canal Verde)
 * @param val3 Tercer componente (ej. coordenada Z o canal Azul)
 * @param val4 Cuarto componente (ej. coordenada W o canal Alfa/Transparencia)
 * @param p_targetDestination Puntero que contiene la dirección de la estructura destino.
 */
void math_set_vector4(u32 val1, u32 val2, u32 val3, u32 val4, u32* p_targetDestination) {
	// Obtiene la dirección base real del objeto destino
	u32 base_address = *p_targetDestination;

	// Almacena los 4 componentes de manera contigua en la memoria de la PS2 (saltos de 4 bytes)
	*(u32*)(base_address + 0x0) = val1;
	*(u32*)(base_address + 0x4) = val2;
	*(u32*)(base_address + 0x8) = val3;
	*(u32*)(base_address + 0xC) = val4;
}

/**
 * @brief Rellena un bloque de memoria con un valor específico (Memset).
 * Versión funcional simplificada de la rutina optimizada para los registros multimedia de la PS2.
 * Dirección original en Ghidra: 0x00115484 (PAL)
 *
 * @param dest Puntero al bloque de memoria a rellenar.
 * @param value Valor de byte con el que se va a rellenar.
 * @param size Cantidad de bytes a escribir.
 * @return void* Puntero a la memoria de destino.
 */
void* ee_memset(void* dest, u8 value, u32 size) {
	u8* ptr = (u8*)dest;

	// En la PS2 real, aquí se ejecuta un bucle optimizado de 32 y 8 bytes
	// usando instrucciones vectoriales si el puntero está alineado a 16 bytes.
	// Para efectos funcionales, el comportamiento exacto es:
	for (u32 i = 0; i < size; i++) {
		ptr[i] = value;
	}

	return dest;
}

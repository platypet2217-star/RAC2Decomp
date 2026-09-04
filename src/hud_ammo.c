// src/hud_ammo.c
#include "types.h"

// Definición funcional de la función interna que descubriste vinculando strings
void hud_register_widget_asset(void* widget_struct, const char* asset_name, long param_3, ...);
void* hud_allocate_or_get_node(int* source, ...);
void* ee_memset(void* dest, u8 value, u32 size);
u32 hud_initialize_subsystem(u32 size, long address);

// Declaración externa de la función constructora que limpia las matrices
void hud_clear_widget_matrices(u32* param_1, const char* text_ptr, long param_3, ...);

/**
 * @brief Enlaza el recurso tipográfico y configura las propiedades visuales del texto del HUD.
 * Establece fuentes, escalas iniciales (1.0f), espaciado (0.7f) y banderas de renderizado.
 * Dirección original en Ghidra: 0x00338688 (PAL)
 */
void hud_link_widget_text(u32* p_widget, const char* text_resource, long param_3,
	long p4, long p5, long p6, long p7, long p8) {

	// 1. Llama a la subrutina interna para limpiar las matrices de transformación
	hud_clear_widget_matrices(p_widget, text_resource, param_3, p4, p5, p6, p7, p8);

	// 2. Asigna el puntero del recurso de la fuente tipográfica global (offset 0xd)
	p_widget[0xD] = 0x002638D0;

	// 3. Configura las escalas iniciales de renderizado X e Y a 1.0f (0x3f800000)
	f32* p_scale = (f32*)p_widget[1];
	p_scale[0] = 1.0f; // Escala X
	p_scale[1] = 1.0f; // Escala Y

	// 4. Configura propiedades de formato y espaciado (0x3f333333 = 0.7f)
	p_widget[0xE] = 1;          // Flag de inicialización o visibilidad activa
	p_widget[0x10] = 0;          // Offset de desplazamiento de renderizado
	p_widget[0x14] = 0x3f333333; // Espaciado entre caracteres / Kerning (0.7f)
	p_widget[0x11] = 1;          // Modo de alineación (ej. Centrado)
	p_widget[0x13] = 0x200;      // Flags de renderizado adicionales (ej. Activar Sombra)
	p_widget[0x12] = 0;          // Rotación o inclinación del texto
}


/**
 * @brief Inicializa los componentes principales de la interfaz (HUD Master Init).
 * Dirección original en Ghidra: 0x0034D490 (PAL)
 */
void hud_initialize_main_widgets(u32* p_hudState, s32 p_ammoData, long param_3) {

	// Líneas 192-193: Flags de estado a cero
	p_hudState[0x567] = 0;
	p_hudState[0x568] = 0;

	// --- SECCIÓN 1: HUD MUNICIÓN Y EXPERIENCIA DE ARMA ---
	// Línea 195: Fondo del HUD de munición ("BackAmmo" - 0x001ae6d8)
	hud_register_widget_asset(p_hudState, (const char*)0x001ae6d8, param_3);

	// Línea 202: Silueta del indicador de balas ("OutlineAmmo" - 0x001ae6e8)
	u32* widget_outline_ammo = p_hudState + 0x13;
	hud_register_widget_asset(widget_outline_ammo, (const char*)0x001ae6e8, param_3);

	// Línea 206: Barra de experiencia del arma ("WeaXP" - 0x001ae6f8)
	u32* widget_weapon_xp = p_hudState + 0x26;
	hud_register_widget_asset(widget_weapon_xp, (const char*)0x001ae6f8, param_3);

	// Línea 212: Limpieza de bloques de memoria de armas
	ee_memset((u8*)p_hudState + 0x39, 0, 0x18);

	// --- NUEVO BLOQUE IDENTIFICADO (Líneas 210-213) ---
	// Configura propiedades matemáticas y busca el estado del componente de texto
	u32* widget_ammo_text = p_hudState + 0x40; // piVar14 corresponde al offset +0x40
	math_set_vector4(0x41200000, 0x41200000, 0, 0, widget_ammo_text);
	hud_set_state_from_lookup((u8*)widget_ammo_text, (u8*)iVar17, 1);

	// Línea 213: Enlaza el recurso de texto "AmmoText" al componente visual del HUD
	hud_link_widget_text(widget_ammo_text, (const char*)0x001ae700, 1);

	// --- SECCIÓN 2: HUD CONTADOR DE GUITONES (BOLTS) ---
	// Línea 226: Contenedor trasero de guitones ("BackBolt" - 0x001ae720)
	u32* widget_back_bolt = p_hudState + 0x65;
	hud_register_widget_asset(widget_back_bolt, (const char*)0x001ae720, param_3);

	// Línea 230: Borde exterior del marcador de guitones ("OutlineBolt" - 0x001ae730)
	u32* widget_outline_bolt = p_hudState + 0x78;
	hud_register_widget_asset(widget_outline_bolt, (const char*)0x001ae730, param_3);

	// Línea 236: Texto numérico para la cantidad total ("BoltText" - 0x001ae740)
	u32* widget_bolt_text = p_hudState + 0x92;
	hud_register_widget_asset(widget_bolt_text, (const char*)0x001ae740, param_3);

	// Línea 235: Limpieza de bloques de memoria de economía
	ee_memset((u8*)p_hudState + 0x8b, 0, 0x18);
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

/**
 * @brief Controla la visibilidad o factor de escala de un componente del HUD.
 * Escribe 1.0f (0x3f800000) o 0.0f en la propiedad de transformación del elemento.
 * Dirección original en Ghidra: 0x00337B48 (PAL)
 *
 * @param p_widget Destino del componente visual (param_1 / registro a0)
 * @param enable Estado booleano para activar o desactivar (param_2 / registro a1)
 */
void hud_set_widget_visibility(u32* p_widget, long enable) {
	// El offset +0x10 (16 bytes) contiene un puntero a la propiedad flotante (ej. Opacidad/Alpha o Escala)
	f32** p_target_property = (f32**)((u8*)p_widget + 0x10);

	if (enable != 0) {
		// En la PS2 escribe 0x3f800000, lo que equivale a 1.0f (Visibilidad/Escala Máxima)
		**p_target_property = 1.0f;
		return;
	}

	// Si es falso, escribe 0.0f (Completamente oculto/Desactivado)
	**p_target_property = 0.0f;
	return;
}

/**
 * @brief Función stub de paso directo de puntero (Identity Function).
 * Utilizada originalmente en el motor para macros de validación o abstracción de nodos.
 * Dirección original en Ghidra: 0x00338B00 (PAL)
 *
 * @param param_1 Primer parámetro (omitido en el retorno)
 * @param p_node Puntero de nodo secundario que es devuelto de forma directa (param_2)
 * @return void* El mismo puntero recibido en param_2.
 */
void* core_identity_stub(long param_1, void* p_node) {
	// El descompilador de Ghidra demuestra que la PS2 simplemente mueve el registro de entrada
	// al registro de salida de inmediato (move $v0, $a1).
	return p_node;
}

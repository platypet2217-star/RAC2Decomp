// src/hud_ammo.c
#include "types.h"
#include <math.h> // Requerido para invocar a sinf() de forma nativa

// Prototipos requeridos de tus funciones indexadoras de la matriz
void inv_set_weapon_slot_data(u32 p_asset_ptr, u32 current_ammo, u32 max_ammo, u32 weapon_id, u32 experience_val, void* p_matrix_base, s32 slot_index);
void inv_update_weapon_visual_pointers(u32 p_primary_asset, u32 p_secondary_asset, void* p_array_base, s32 slot_index);

/**
 * @brief Inicializa por completo el layout visual de la munición y las matrices del inventario de armas.
 * Orquesta el registro de widgets (Fondo, Borde, Texto, Deslizador) y configura las estadísticas base de las ranuras.
 * Dirección original en Ghidra: 0x0034BF20 (PAL)
 */
void hud_init_ammo_layout(void* p_hud_main_struct, long param_2, long p_hud_pool, long p4, long p5, long p6, long p7, long p8) {
	u8* p_base = (u8*)p_hud_main_struct;

	// 1. Reserva inicial y limpieza de nodos de transformación en el pool
	*(u32*)(p_base + 0x13C) = (u32)p_hud_pool;
	if (p_hud_pool != 0) {
		int* node0 = hud_allocate_node((int*)p_hud_pool, param_2, p_hud_pool, p4, p5, p6, p7, p8);
		u32* vec0 = (u32*)core_identity_stub(0x10, node0);
		*(u32**)(p_base + 8) = vec0;
		vec0[0] = 0; vec0[1] = 0; vec0[2] = 0; vec0[3] = 0;

		int* node1 = hud_allocate_node((int*)*(u32*)(p_base + 0x13C), 0, 0, 0, 0, 0, 0, 0);
		u32* vec1 = (u32*)core_identity_stub(0x10, node1);
		*(u32**)(p_base + 0x1C8) = vec1;
		vec1[0] = 0; vec1[1] = 0; vec1[2] = 0; vec1[3] = 0;
	}

	// 2. Registro de componentes visuales (Widgets) de la munición
	hud_register_widget_asset((u32*)(p_base + 0x10), (const char*)0x001AE668, p_hud_pool, p4, p5, p6, p7, p8); // "AmmoBack"
	hud_register_widget_asset((u32*)(p_base + 0x5C), (const char*)0x001AE678, p_hud_pool, p4, p5, p6, p7, p8); // "AmmoOutline"
	hud_register_widget_asset((u32*)(p_base + 0xA8), (const char*)0x001AE680, p_hud_pool, p4, p5, p6, p7, p8); // "AmmoText"

	// 3. Inicialización y configuración estética del deslizador/medidor de balas
	u32* p_slider = (u32*)(p_base + 0xF4);
	hud_init_slider_widget(p_slider, 0x92, 0, (const char*)0x001AE688, p_hud_pool, p4, p5, p6); // "AmmoBar"

	hud_set_widget_context_2d(p_slider, 0x8049c1ff, 0x80001eff);
	hud_set_widget_context_2d_ext(p_slider, 0x50f0c070, 0x50f0c070);
	hud_set_widget_color_alt(p_slider, 0x60442d00);
	hud_set_widget_render_mode_alt(p_slider, 100);

	// Escalado vertical Y del medidor dinámico
	f32 scale_y = 1.5f; // Valor estimado basado en la interpolación original
	hud_set_widget_scale_y(p_slider, (s32)scale_y);

	// 4. Configuración secuencial de las ranuras del inventario de armas (Capa A)
	void* p_inv_a = (void*)(p_base + 0x140);
	inv_reset_weapon_inventory(p_inv_a);
	inv_set_weapon_inventory_mode(p_inv_a, 2);
	inv_set_active_weapon_slot(p_inv_a, (p_base + 0x1CC));
	inv_set_animation_factor(0.005f, p_inv_a);
	inv_set_quick_select_open_state(p_inv_a, 2);
	inv_set_weapon_slot_data(0, 0x80f0c070, 0x42480000, 0, 0, p_inv_a, 0); // Capacidad 50.0f
	inv_update_weapon_visual_pointers(0, 0, p_inv_a, 0);
	inv_set_weapon_inventory_visibility(p_inv_a, 1);

	// 5. Configuración secuencial de las ranuras del inventario de armas (Capa B)
	void* p_inv_b = (void*)(p_base + 0x1D4);
	inv_reset_weapon_inventory(p_inv_b);
	inv_set_weapon_inventory_mode(p_inv_b, 3);
	inv_set_weapon_slot_data(0, 0x442d00, 0, 0, 0, p_inv_b, 0);
	inv_set_weapon_slot_data(0x3E99999A, 0x60442d00, 0, 0, 0, p_inv_b, 1);
	inv_update_weapon_visual_pointers(0, 0x40000000, p_inv_b, 1);
	inv_set_quick_select_open_state(p_inv_b, 0);
	inv_set_active_weapon_slot(p_inv_b, (p_base + 0x1CC));
	inv_set_weapon_inventory_visibility(p_inv_b, -1);

	// Inicialización de banderas de control secundarias finales
	*(u32*)(p_base + 0x3F4) = 0;
	*(u32*)(p_base + 0x400) = 0xFFFFFFFF;
	*(u32*)(p_base + 0x3F8) = 0;
}

/**
 * @brief Intercambia el valor del candado de animación del inventario (offset 0x28) y retorna su estado previo.
 * Utilizado por el motor para liberar transiciones y verificar estados de sincronización en el HUD.
 * Dirección original en Ghidra: 0x0034B790 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param new_lock_state Nuevo valor de control que se inyectará en la bandera (param_2).
 * @return u32 El estado previo que tenía la bandera antes de ser sobreescrita.
 */
u32 inv_swap_animation_lock(u32* p_inventory_base, u32 new_lock_state) {
	if (p_inventory_base == NULL) {
		return 0;
	}

	// El offset 0x28 equivale al índice 10 en un arreglo de enteros de 32 bits (10 * 4 = 40 bytes)
	u32 old_lock_state = p_inventory_base[0x0A];

	// Sobreescribe la bandera con el nuevo estado solicitado
	p_inventory_base[0x0A] = new_lock_state;

	// Devuelve el valor antiguo al pipeline de renderizado
	return old_lock_state;
}

/**
 * @brief Configura y despacha la transición de visibilidad (Fade In/Out) para la interfaz del inventario de armas.
 * Establece los límites de animación flotante en el offset 0x18 dependiendo del estado solicitado.
 * Dirección original en Ghidra: 0x0034B7F8 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param visibility_state Nuevo estado de visibilidad (1 para visible, 0 para ocultar) (param_2).
 */
void inv_set_weapon_inventory_visibility(u32* p_inventory_base, long visibility_state) {
	if (p_inventory_base == NULL) {
		return;
	}

	// El offset 0x1C equivale al índice 7 en un arreglo de enteros de 32 bits (7 * 4 = 28 bytes)
	p_inventory_base[0x07] = (s32)visibility_state;

	// Offset 0x28 equivale al índice 10 (10 * 4 = 40 bytes), controla el bloqueo de interpolación
	if (p_inventory_base[0x0A] == 0) {
		// Si el estado es 1 (Aparecer), arranca la animación desde 0.0f
		if (visibility_state == 1) {
			p_inventory_base[0x06] = 0; // Offset 0x18 (Índice 6)
		}
		// Si no (Ocultar), arranca la animación de desvanecimiento desde 1.0f (0x3f800000)
		else {
			p_inventory_base[0x06] = 0x3F800000; // Offset 0x18 (Índice 6)
		}

		p_inventory_base[0x0A] = 1; // Activa la bandera de inicio de ciclo de animación
	}
}

/**
 * @brief Configura el estado de visibilidad o apertura del menú radial de selección rápida en el offset 0x20.
 * Dirección original en Ghidra: 0x0034B838 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param open_state Nuevo valor de control (1 para abierto/visible, 0 para cerrado/oculto) (param_2).
 */
void inv_set_quick_select_open_state(u32* p_inventory_base, u32 open_state) {
	if (p_inventory_base != NULL) {
		// El offset 0x20 equivale al índice 8 en un arreglo de enteros de 32 bits (8 * 4 = 32 bytes)
		p_inventory_base[0x08] = open_state;
	}
}

/**
 * @brief Configura la velocidad de interpolación o factor de animación de la interfaz en el offset 0x24 del inventario.
 * Dirección original en Ghidra: 0x0034B840 (PAL)
 *
 * @param animation_val Factor flotante o de control destinado a la velocidad de la interfaz (param_1).
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_2).
 */
void inv_set_animation_factor(f32 animation_val, u32* p_inventory_base) {
	if (p_inventory_base != NULL) {
		// El offset 0x24 equivale al índice 9 en un arreglo de enteros de 32 bits (9 * 4 = 36 bytes)
		// Nota: Ghidra invirtió el orden de los argumentos en el descompilador original (param_1 es el valor, param_2 es el puntero)
		p_inventory_base[9] = *(u32*)&animation_val;
	}
}

/**
 * @brief Configura el índice del slot del arma activa o seleccionada en el offset 0x80 del inventario.
 * Dirección original en Ghidra: 0x0034B788 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param slot_index Índice de la ranura o identificación del arma que se va a equipar (param_2).
 */
void inv_set_active_weapon_slot(u32* p_inventory_base, u32 slot_index) {
	if (p_inventory_base != NULL) {
		// El offset 0x80 equivale al índice 32 en un arreglo de enteros de 32 bits (32 * 4 = 128 bytes)
		p_inventory_base[0x20] = slot_index;
	}
}

/**
 * @brief Configura la bandera de transición o estado de carga del inventario de armas en el offset 0x2C.
 * Dirección original en Ghidra: 0x0034B758 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param transition_flag Nuevo valor de control o bandera de estado a inyectar (param_2).
 */
void inv_set_weapon_inventory_transition_flag(u32* p_inventory_base, u32 transition_flag) {
	if (p_inventory_base != NULL) {
		// El offset 0x2C equivale al índice 11 en un arreglo de enteros de 32 bits (11 * 4 = 44 bytes)
		p_inventory_base[0x0B] = transition_flag;
	}
}

/**
 * @brief Configura el modo de visualización o el estado secundario del inventario de armas en el offset 0x84.
 * Dirección original en Ghidra: 0x0034B7F0 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 * @param inventory_mode Nuevo valor de estado o modo lúdico a inyectar (param_2).
 */
void inv_set_weapon_inventory_mode(u32* p_inventory_base, u32 inventory_mode) {
	if (p_inventory_base != NULL) {
		// El offset 0x84 equivale al índice 33 en un arreglo de enteros de 32 bits (33 * 4 = 132 bytes)
		p_inventory_base[33] = inventory_mode;
	}
}

/**
 * @brief Reinicia el estado global del inventario de armas a sus valores predeterminados de fábrica.
 * Limpia las propiedades dinámicas e inicializa las primeras ranuras de la matriz del arsenal.
 * Dirección original en Ghidra: 0x0034B698 (PAL)
 *
 * @param p_inventory_base Dirección de memoria base de la estructura global del inventario (param_1).
 */
void inv_reset_weapon_inventory(void* p_inventory_base) {
	if (p_inventory_base == NULL) {
		return;
	}

	u8* p_inv = (u8*)p_inventory_base;

	// 1. Limpieza de banderas de estado e inicialización de variables base
	*(u32*)(p_inv + 0x18) = 0;
	*(u32*)(p_inv + 0x28) = 0;
	u32 zero_token = *(u32*)(p_inv + 0x18);

	*(u32*)(p_inv + 0x2C) = 0;
	*(u32*)(p_inv + 0x80) = 0;

	// 2. Inyección del factor de animación fina (0x3ba3d70a = 0.005f)
	*(f32*)(p_inv + 0x24) = 0.005f;

	// 3. Inicialización condicional de las ranuras de la matriz utilizando tus funciones
	inv_set_weapon_slot_data(zero_token, zero_token, zero_token, zero_token, zero_token, p_inventory_base, 0);

	// Inyecta el valor flotante 1.0f (0x3f800000) de forma masiva en el slot 1
	u32 float_one_token = 0x3F800000;
	inv_set_weapon_slot_data(float_one_token, float_one_token, float_one_token, float_one_token, float_one_token, p_inventory_base, 1);

	// 4. Configuración de banderas secundarias e inicialización de punteros visuales
	*(u32*)(p_inv + 0x84) = 2; // Selector o contador secundario por defecto
	inv_update_weapon_visual_pointers(zero_token, zero_token, p_inventory_base, 0);

	*(u32*)(p_inv + 0x20) = 0;
	*(u32*)(p_inv + 0x1C) = 1; // Bandera de activación o visibilidad inicial
}

/**
 * @brief Actualiza los punteros a los recursos gráficos (assets/texturas) vinculados a una ranura específica del arsenal.
 * Escribe las direcciones en ráfagas indexadas de 4 bytes para el pipeline de renderizado del inventario.
 * Dirección original en Ghidra: 0x0034B7D8 (PAL)
 *
 * @param p_primary_asset Puntero al recurso gráfico base o textura principal del arma (param_1).
 * @param p_secondary_asset Puntero al recurso gráfico secundario o máscara de interfaz (param_2).
 * @param p_array_base Dirección de memoria base del arreglo de punteros visuales (param_3).
 * @param slot_index Índice de la ranura o columna a modificar (param_4).
 */
void inv_update_weapon_visual_pointers(u32 p_primary_asset, u32 p_secondary_asset,
	void* p_array_base, s32 slot_index) {
	if (p_array_base == NULL) {
		return;
	}

	// Calcula el puntero exacto a la ranura seleccionada (Paso de 4 bytes)
	u32* p_slot_target = (u32*)((u8*)p_array_base + slot_index * 4);

	p_slot_target[0] = p_primary_asset;   // Inyecta en el offset base +0
	p_slot_target[3] = p_secondary_asset; // Inyecta en el offset +12 bytes (índice 3 en u32)
}

/**
 * @brief Inicializa e inyecta los parámetros estadísticos y visuales de un arma dentro de la matriz indexada de inventario.
 * Calcula los offsets contiguos de 16 bytes (0x10) para almacenar munición, IDs y referencias del HUD.
 * Dirección original en Ghidra: 0x0034B7A0 (PAL)
 *
 * @param p_asset_ptr Puntero al nombre o recurso visual del widget del arma (param_1).
 * @param current_ammo Cantidad de balas actuales (param_2).
 * @param max_ammo Capacidad máxima del cargador (param_3).
 * @param weapon_id Identificador único del tipo de arma/munición (param_4).
 * @param experience_val Progreso de experiencia o nivel del arma (param_5).
 * @param p_matrix_base Dirección de memoria base de la tabla de datos del inventario (param_6).
 * @param slot_index Índice de la ranura o columna a modificar (param_7).
 */
void inv_set_weapon_slot_data(u32 p_asset_ptr, u32 current_ammo, u32 max_ammo, u32 weapon_id,
	u32 experience_val, void* p_matrix_base, s32 slot_index) {

	// Calcula la dirección física de la estructura de datos del arma (Paso de 16 bytes)
	u8* p_data_row = (u8*)p_matrix_base + ((s32)p_matrix_base + slot_index * 0x10);

	// Inyecta en ráfaga contigua las estadísticas dinámicas del cargador en los desplazamientos +0x30
	*(u32*)(p_data_row + 0x30) = current_ammo;    // Munición Actual
	*(u32*)(p_data_row + 0x34) = max_ammo;        // Munición Máxima
	*(u32*)(p_data_row + 0x38) = weapon_id;       // ID del Arma
	*(u32*)(p_data_row + 0x3C) = experience_val;  // Valor de XP / Modificador

	// Enlaza el recurso o asset gráfico del widget en la sección de punteros visuales (+0x70)
	u8* p_visual_row = (u8*)p_matrix_base + slot_index * 4;
	*(u32*)(p_visual_row + 0x70) = p_asset_ptr;
}

/**
 * @brief Configura de forma aislada el componente vertical Y (Escala/Orientación) del vector secundario de un widget.
 * Convierte el valor entero a punto flotante y lo inyecta en el desplazamiento +4 del vector del offset +4.
 * Dirección original en Ghidra: 0x00338A20 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param y_scale Factor de escala o dimensión vertical en enteros que será convertida a flotante (param_2).
 */
void hud_set_widget_scale_y(u32* p_widget, s32 y_scale) {
	if (p_widget != NULL) {
		// El offset +4 de la estructura base almacena el puntero al vector de transformación
		f32** pp_vector_target = (f32**)((u8*)p_widget + 4);
		f32* p_vector = *pp_vector_target;

		if (p_vector != NULL) {
			// El desplazamiento +4 dentro del propio vector corresponde al componente Y (índice 1 en f32)
			p_vector[1] = (f32)y_scale;
		}
	}
}

/**
 * @brief Configura las banderas de renderizado o modo de mezcla (Alias alternativo de hud_set_widget_render_mode).
 * Escribe directamente en la propiedad de control gráfico ubicada en el offset 0x40.
 * Dirección original en Ghidra: 0x003388E8 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param render_flags Máscara de bits con las opciones de dibujado y transparencia (param_2).
 */
void hud_set_widget_render_mode_alt(u32* p_widget, u32 render_flags) {
	if (p_widget != NULL) {
		// El offset 0x40 equivale al índice 0x10 en un arreglo de enteros de 32 bits (16 * 4 = 64 bytes)
		p_widget[0x10] = render_flags;
	}
}

// Prototipo requerido de tu constructor base
void hud_clear_widget_matrices(u32* p_widget_transform, const char* text_ptr, long p_hud_pool, long p4, long p5, long p6, long p7, long p8);

/**
 * @brief Configura el color o la opacidad de un componente visual (Alias alternativo de hud_set_widget_color).
 * Escribe directamente en la propiedad ubicada en el offset 0x44 de la estructura.
 * Dirección original en Ghidra: 0x00338A38 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param color_rgba Valor de 32-bits que codifica el color y opacidad (param_2).
 */
void hud_set_widget_color_alt(u32* p_widget, u32 color_rgba) {
	if (p_widget != NULL) {
		// El offset 0x44 equivale al índice 11 en un arreglo de enteros de 32 bits (17 * 4 = 68 bytes)
		p_widget[0x11] = color_rgba;
	}
}

/**
 * @brief Inicializa una estructura de widget destinada a deslizadores lógicos o selectores de valor en la interfaz.
 * Configura los parámetros iniciales de escala y establece el límite superior por defecto al 100%.
 * Dirección original en Ghidra: 0x003380E8 (PAL)
 *
 * @param p_widget Dirección base de la estructura del widget de la interfaz (param_1).
 * @param value_id Identificador o valor inicial asignado al deslizador (param_2).
 * @param p_data_source Puntero de control o fuente de datos del elemento (param_3).
 */
void hud_init_slider_widget(u32* p_widget, u32 value_id, u32 p_data_source, const char* asset_name_ptr,
	long p_hud_pool, long p6, long p7, long p8) {

	// 1. Invoca al constructor base para inicializar las matrices espaciales y habilitar la visibilidad
	hud_clear_widget_matrices(p_widget, asset_name_ptr, p_hud_pool, (long)asset_name_ptr, p_hud_pool, p6, p7, p8);

	// 2. Inyecta los parámetros de estado y enlaces de control en las propiedades contiguas
	p_widget[0x0F] = value_id;      // Offset 0x3C
	p_widget[0x0D] = p_data_source;  // Offset 0x34

	// 3. Establece los límites y banderas iniciales por defecto del motor (Rango al 100)
	p_widget[0x11] = 0x80000000;    // Offset 0x44 (Máscara de actualización)
	p_widget[0x10] = 100;           // Offset 0x40 (Límite máximo del 100%)
}

/**
 * @brief Configura el segundo par de datos de 32 bits (offsets +8 y +12) dentro de la estructura del contexto del widget (offset 0x0C).
 * Utilizado usualmente para definir los límites de recorte UV o las dimensiones secundarias de una textura.
 * Dirección original en Ghidra: 0x00338190 (PAL)
 *
 * @param p_widget Dirección base de la estructura del widget de la interfaz (param_1).
 * @param val_z Tercer componente o dato de control (param_2).
 * @param val_w Cuarto componente o dato de control contiguo (param_3).
 */
void hud_set_widget_context_2d_ext(u32* p_widget, u32 val_z, u32 val_w) {
	if (p_widget != NULL) {
		// El offset 0x0C equivale al índice 3 en enteros de 32 bits (3 * 4 = 12 bytes)
		u32** pp_context_target = (u32**)((u8*)p_widget + 0x0C);
		u32* p_context = *pp_context_target;

		if (p_context != NULL) {
			p_context[2] = val_z;   // Almacena en el offset +8 del bloque de contexto
			p_context[3] = val_w;   // Almacena en el offset +12 del bloque de contexto
		}
	}
}

// Prototipo de la función que liberamos previamente
void hud_free_node(int* p_hud_pool, int* p_node_to_free);

/**
 * @brief Configura un par de datos contiguos de 32 bits (X, Y) dentro de la estructura del contexto del widget (offset 0x0C).
 * Dirección original en Ghidra: 0x00338178 (PAL)
 *
 * @param p_widget Dirección base de la estructura del widget de la interfaz (param_1).
 * @param val_x Primer componente o dato de control (param_2).
 * @param val_y Segundo componente o dato de control contiguo (param_3).
 */
void hud_set_widget_context_2d(u32* p_widget, u32 val_x, u32 val_y) {
	if (p_widget != NULL) {
		// El offset 0x0C equivale al índice 3 en enteros de 32 bits (3 * 4 = 12 bytes)
		u32** pp_context_target = (u32**)((u8*)p_widget + 0x0C);
		u32* p_context = *pp_context_target;

		if (p_context != NULL) {
			p_context = val_x;   // Almacena en el offset +0 del bloque de contexto
			p_context = val_y;   // Almacena en el offset +4 del bloque de contexto
		}
	}
}

/**
 * @brief Actualiza de forma dinámica el vector de transformación asignado en el offset +4 de un widget.
 * Libera de forma automática el nodo previo mediante hud_free_node si detecta un cambio posicional o de escala.
 * Dirección original en Ghidra: 0x00337B90 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param p_new_vector Puntero al nuevo vector matemático de 4 componentes a renderizar (param_2).
 */
void hud_update_widget_vector(u32* p_widget, u32* p_new_vector) {
	if (p_widget == NULL) {
		return;
	}

	// El offset +4 equivale al índice 1 en un arreglo de enteros de 32 bits (1 * 4 = 4 bytes)
	u32** pp_current_vector = (u32**)((u8*)p_widget + 4);

	// Valida si el nuevo vector es diferente al que ya está cargado en la estructura
	if (p_new_vector != *pp_current_vector) {

		// Offset 0x2C equivale al índice 11 (11 * 4 = 44 bytes), que almacena el puntero del pool
		u32* p_hud_pool = (u32*)p_widget[0x0B];

		if (p_hud_pool == 0) {
			*pp_current_vector = p_new_vector;
		}
		// Offset 0x18 equivale al índice 6 (6 * 4 = 24 bytes), bandera de refresco/ciclo secundaria
		else if (p_widget[0x06] == 0) {
			// Invoca a tu rutina de reciclaje para liberar el nodo de memoria anterior
			hud_free_node((int*)p_hud_pool, (int*)*pp_current_vector);

			p_widget[0x06] = 1; // Activa la bandera de refresco/redibujado del layout vectorial
			*pp_current_vector = p_new_vector;
		}
		else {
			*pp_current_vector = p_new_vector;
		}
	}
}

/**
 * @brief Recupera el puntero al vector secundario de transformación (Escala/Orientación) de un widget del HUD.
 * Lee directamente la dirección física almacenada en el offset +4 de la estructura.
 * Dirección original en Ghidra: 0x00337AF8 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @return f32* Puntero al vector de escala (X, Y, Z, W) del widget, o NULL si no está asignado.
 */
f32* hud_get_widget_vector_ptr(u32* p_widget) {
	if (p_widget == NULL) {
		return NULL;
	}

	// El offset +4 equivale al índice 1 en un arreglo de enteros de 32 bits (1 * 4 = 4 bytes)
	return (f32*)((long)p_widget[1]);
}

/**
 * @brief Calcula el coseno matemático utilizando optimización por hardware (Originalmente vía VU0 Coprocessor 2).
 * Reemplaza de forma portátil las llamadas a microcódigo vectorial e instrucciones _vcallms de la PS2.
 * Dirección original en Ghidra: 0x00283A58 (PAL)
 *
 * @param radians Ángulo en radianes normalizado previamente (param_1).
 * @return f32 El resultado del coseno calculado de forma nativa por hardware.
 */
f32 math_vu0_cos(f32 radians) {
	// En el hardware original de la PS2 se llamaba a la microrutina 0xC90 en la VU0.
	// De manera portable para sistemas modernos, la FPU del PC lo resuelve al instante:
	return cosf(radians);
}

/**
 * @brief Calcula el seno matemático utilizando optimización por hardware (Originalmente vía VU0 Coprocessor 2).
 * Reemplaza de forma portátil las llamadas a microcódigo vectorial e instrucciones _vcallms de la PS2.
 * Dirección original en Ghidra: 0x00283A40 (PAL)
 *
 * @param radians Ángulo en radianes normalizado previamente (param_1).
 * @return f32 El resultado del seno calculado de forma nativa por hardware.
 */
f32 math_vu0_sin_cos(f32 radians) {
	// En el hardware original, se inyecta el ángulo al Coprocesador 2 vía _qmtc2,
	// se llama a la microrutina 0xC80 en la VU0 y se extrae el resultado con _qmfc2.
	// De manera portable para sistemas modernos, la FPU del PC lo resuelve al instante:
	return sinf(radians);
}

// Declaración externa de tu despachador de aserciones del kernel
void sys_assert_dispatch(const char* p_file, s32 line, const char* p_assertion, ...);

// Prototipos del HUD ya documentados en tu ecosistema
int* hud_allocate_node(int* p_hud_pool, long p2, long p3, long p4, long p5, long p6, long p7, long p8);
void* core_identity_stub(long param_1, void* p_node);
void  hud_set_widget_visibility(u32* p_widget, long enable);

// Prototipos requeridos de tu suite del HUD
void  hud_clear_widget_matrices(u32* p_widget_transform, const char* text_ptr, long p_hud_pool, long p4, long p5, long p6, long p7, long p8);

// Prototipo de la función que liberamos en el paso anterior
void hud_free_node(int* p_hud_pool, int* p_node_to_free);

// Prototipos de tu suite del HUD requeridos para la interconexión
void  hud_clear_widget_matrices(u32* p_widget_transform, const char* text_ptr, long p_hud_pool, long p4, long p5, long p6, long p7, long p8);
int* hud_allocate_node(int* p_hud_pool, long p2, long p3, long p4, long p5, long p6, long p7, long p8);
void* core_identity_stub(long param_1, void* p_node);

#define MATH_PI 3.14159265358979323846f

/**
 * @brief Normaliza la suma de dos ángulos en radianes para mantener el resultado en el rango [-PI, PI].
 * Corrige desbordamientos angulares circulares sumando o restando una revolución completa (2*PI).
 * Dirección original en Ghidra: 0x00284458 (PAL)
 *
 * @param angle_alpha Primer componente angular en radianes (param_1).
 * @param angle_beta Segundo componente angular en radianes a adicionar (param_2).
 * @return f32 El ángulo resultante normalizado.
 */
f32 math_normalize_angle_rad(f32 angle_alpha, f32 angle_beta) {
	f32 result_angle = angle_alpha + angle_beta;

	// Si el ángulo es mayor o igual a PI, le resta una revolución completa (2 * PI)
	if (result_angle >= (f32)MATH_PI) {
		result_angle = (result_angle - (f32)MATH_PI) - (f32)MATH_PI;
	}
	// Si el ángulo es menor a -PI, le adiciona una revolución completa (2 * PI)
	if (result_angle < -(f32)MATH_PI) {
		result_angle = result_angle + (f32)MATH_PI + (f32)MATH_PI;
	}

	return result_angle;
}

/**
 * @brief Inicializa y registra una estructura de widget destinada a albergar un recurso visual o textura (Asset).
 * Configura las matrices base estableciendo una escala inicial uniforme de 1.0f (100%) para evitar distorsiones.
 * Dirección original en Ghidra: 0x003381B0 / Línea 274 aproximada (PAL)
 */
void hud_register_widget_asset(u32* p_widget, const char* asset_name_ptr, long p_hud_pool,
	long p4, long p5, long p6, long p7, long p8) {

	// 1. Invoca al constructor base para limpiar e inicializar las matrices espaciales
	hud_clear_widget_matrices(p_widget, asset_name_ptr, p_hud_pool, p4, p5, p6, p7, p8);

	// El offset 0x0B equivale al índice 11, almacena el puntero del pool de memoria
	if ((int*)p_widget[0x0B] == NULL) {
		p_widget[0x12] = 0; // Inicializa en cero la bandera de estado secundario (offset 0x48)
	}
	else {
		u32* p_vector_node;

		// 2. Reserva y limpia el primer nodo auxiliar en el offset 0x0D
		int* p_node1 = hud_allocate_node((int*)p_widget[0x0B], 0, 0, p4, p5, p6, p7, p8);
		p_vector_node = (u32*)core_identity_stub(0x10, p_node1);
		p_widget[0x0D] = (u32)p_vector_node;

		p_vector_node[0] = 0;
		p_vector_node[1] = 0;
		p_vector_node[2] = 0;
		p_vector_node[3] = 0;

		// 3. Reserva y limpia el segundo nodo auxiliar en el offset 0x0E
		int* p_node2 = hud_allocate_node((int*)p_widget[0x0B], 0, 0, 0, 0, 0, 0, 0);
		p_vector_node = (u32*)core_identity_stub(0x10, p_node2);
		p_widget[0x0E] = (u32)p_vector_node;

		p_vector_node[0] = 0;
		p_vector_node[1] = 0;
		p_vector_node[2] = 0;
		p_vector_node[3] = 0;

		// 4. Configura la escala base predeterminada en 1.0f (0x3f800000) en el segundo vector indexado (param_1[1])
		f32* p_scale_vector = *(f32**)(&p_widget[1]);
		if (p_scale_vector != NULL) {
			p_scale_vector[0] = 1.0f; // Escala X
			p_scale_vector[1] = 1.0f; // Escala Y
			p_scale_vector[2] = 1.0f; // Escala Z
		}

		p_widget[0x12] = 0;
	}

	// Inicializa en cero la propiedad del offset 0x11 (44 bytes)
	p_widget[0x11] = 0;
}

/**
 * @brief Actualiza de forma dinámica el recurso de datos o contexto asignado a un widget del HUD.
 * Libera de forma automática el nodo previo mediante hud_free_node si detecta un cambio de recurso visual.
 * Dirección original en Ghidra: 0x00337C00 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param p_new_resource Puntero al nuevo dato, string o ícono gráfico a renderizar (param_2).
 */
void hud_update_widget_context(u32* p_widget, u32* p_new_resource) {
	if (p_widget == NULL) {
		return;
	}

	// El offset 0x10 equivale al índice 4 en un arreglo de enteros de 32 bits (4 * 4 = 16 bytes)
	u32** pp_current_resource = (u32**)((u8*)p_widget + 0x10);

	// Valida si el nuevo recurso es diferente al que ya está cargado en pantalla
	if (p_new_resource != *pp_current_resource) {

		// Offset 0x2C equivale al índice 11 (11 * 4 = 44 bytes), que almacena el puntero del pool
		u32* p_hud_pool = (u32*)p_widget[0x0B]; // Usando el índice 11 indexado

		if (p_hud_pool == 0) {
			*pp_current_resource = p_new_resource;
		}
		// Offset 0x24 equivale al índice 9 (9 * 4 = 36 bytes), bandera de refresco/ciclo
		else if (p_widget[0x09] == 0) {
			// Invoca a tu rutina del HUD para reciclar y liberar el nodo de memoria anterior
			hud_free_node((int*)p_hud_pool, (int*)*pp_current_resource);

			p_widget[0x09] = 1; // Activa la bandera de refresco/redibujado del layout
			*pp_current_resource = p_new_resource;
		}
		else {
			*pp_current_resource = p_new_resource;
		}
	}
}

/**
 * @brief Libera un nodo de memoria del HUD y lo devuelve a la lista de reusables (Free List).
 * Decrementa el contador de widgets activos y reestructura los punteros de la cabecera del pool.
 * Dirección original en Ghidra: 0x00338C28 (PAL)
 *
 * @param p_hud_pool Estructura de cabecera del pool de memoria de la interfaz (param_1).
 * @param p_node_to_free Puntero al bloque de memoria del nodo que se va a liberar (param_2).
 */
void hud_free_node(int* p_hud_pool, int* p_node_to_free) {
	if (p_hud_pool == NULL || p_node_to_free == NULL) {
		return;
	}

	// El offset 0x14 corresponde al índice 5 en enteros de 32 bits (5 * 4 = 20 bytes)
	int* p_current_free_head = (int*)p_hud_pool[5];

	// Enlaza el nodo que se libera al frente de la lista de reusables anterior
	*p_node_to_free = (int)p_current_free_head;

	// Coloca el nodo liberado como la nueva cabecera de elementos disponibles en el pool
	p_hud_pool[5] = (int)p_node_to_free;

	// El offset 0x10 (índice 4, 16 bytes) reduce el contador de widgets del HUD en uso
	p_hud_pool[4] = p_hud_pool[4] - 1;
}

/**
 * @brief Configura la posición bidimensional (X, Y) en punto flotante para un componente del HUD.
 * Convierte las coordenadas enteras y las almacena secuencialmente en el puntero del offset 0x34.
 * Dirección original en Ghidra: 0x00338600 (PAL)
 *
 * @param p_widget Dirección base de la estructura del widget de la interfaz (param_1).
 * @param x_coord Coordenada horizontal en píxeles (param_2).
 * @param y_coord Coordenada vertical en píxeles (param_3).
 */
void hud_set_widget_position_2d(u32* p_widget, s32 x_coord, s32 y_coord) {
	if (p_widget != NULL) {
		// El offset 0x34 equivale al índice 13 en un arreglo de enteros de 32 bits (13 * 4 = 52 bytes)
		f32** pp_vector_target = (f32**)((u8*)p_widget + 0x34);
		f32* p_vector = *pp_vector_target;

		if (p_vector != NULL) {
			p_vector[0] = (f32)x_coord; // Inyecta la coordenada X como flotante
			p_vector[1] = (f32)y_coord; // Inyecta la coordenada Y como flotante contigua (+4 bytes)
		}
	}
}

/**
 * @brief Recupera el puntero de datos dinámicos o contexto enlazado a un widget del HUD.
 * Lee directamente la dirección física almacenada en el offset 0x0C de la estructura.
 * Dirección original en Ghidra: 0x00337B00 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @return void* Puntero al contexto de datos del widget, o NULL si no está asignado.
 */
void* hud_get_widget_data_ptr(u32* p_widget) {
	if (p_widget == NULL) {
		return NULL;
	}

	// El offset 0x0C equivale al índice 3 en un arreglo de enteros de 32 bits (3 * 4 = 12 bytes)
	return (void*)((long)p_widget[3]);
}

/**
 * @brief Inicializa una estructura de widget destinada a medidores o barras del HUD (ej. Barra de Experiencia).
 * Configura los vectores base con escalas flotantes predeterminadas (100.0f y 64.0f) y reserva su nodo de estado.
 * Dirección original en Ghidra: 0x003383F0 (PAL)
 */
void hud_init_meter_widget(u32* p_widget, const char* text_ptr, long p_hud_pool,
	long p4, long p5, long p6, long p7, long p8) {

	// 1. Invoca al constructor base del widget para limpiar e inicializar las matrices espaciales
	hud_clear_widget_matrices(p_widget, text_ptr, p_hud_pool, p4, p5, p6, p7, p8);

	u32* p_state_vector;

	// 2. Si el pool está activo, reserva y limpia el bloque de estado del medidor en el offset 0xD
	if (p_hud_pool == 0) {
		p_state_vector = (u32*)p_widget[0x0D];
	}
	else {
		int* p_node = hud_allocate_node((int*)p_widget[0x0B], 0, 0, p4, p5, p6, p7, p8);
		p_state_vector = (u32*)core_identity_stub(0x10, p_node);
		p_widget[0x0D] = (u32)p_state_vector;

		p_state_vector[0] = 0;
		p_state_vector[1] = 0;
		p_state_vector[2] = 0;
		p_state_vector[3] = 0;

		p_state_vector = (u32*)p_widget[0x0D];
	}

	p_state_vector[0] = 0;
	p_state_vector[1] = 0;

	// 3. Inyecta los valores mágicos flotantes de inicialización (100.0f y 64.0f)
	f32* p_vector_pos = *(f32**)p_widget;       // Primer vector indexado
	f32* p_vector_scale = *(f32**)(&p_widget[1]); // Segundo vector indexado

	if (p_vector_pos != NULL) {
		p_vector_pos[0] = 100.0f; // 0x42c80000 en la PS2
		p_vector_pos[1] = 100.0f;
	}

	if (p_vector_scale != NULL) {
		p_vector_scale[0] = 64.0f; // 0x42800000 en la PS2
		p_vector_scale[1] = 64.0f;
	}

	// Inicializa en cero la bandera de animación o temporizador
	p_widget[0x0E] = 0;
}


/**
 * @brief Configura el color base o el tinte de transparencia (Alpha) de un widget del HUD.
 * Escribe directamente en la propiedad de control cromático ubicada en el offset 0x44.
 * Dirección original en Ghidra: 0x00338728 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param color_rgba Valor de 32-bits que codifica el color y opacidad en formato RGBA/Color-ID (param_2).
 */
void hud_set_widget_color(u32* p_widget, u32 color_rgba) {
	if (p_widget != NULL) {
		// El offset 0x44 equivale al índice 0x11 en un arreglo de enteros de 32 bits (17 * 4 = 68 bytes)
		p_widget[0x11] = color_rgba;
	}
}

/**
 * @brief Configura las banderas de renderizado o modo de mezcla Alpha en la estructura de un widget del HUD.
 * Escribe directamente en la propiedad de control gráfico ubicada en el offset 0x40.
 * Dirección original en Ghidra: 0x00338730 (PAL)
 *
 * @param p_widget Dirección base de la estructura del componente de la interfaz (param_1).
 * @param render_flags Máscara de bits con las opciones de dibujado y transparencia (param_2).
 */
void hud_set_widget_render_mode(u32* p_widget, u32 render_flags) {
	if (p_widget != NULL) {
		// El offset 0x40 equivale al índice 0x10 en un arreglo de enteros de 32 bits (16 * 4 = 64 bytes)
		p_widget[0x10] = render_flags;
	}
}

/**
 * @brief Almacena de forma directa cuatro componentes individuales (X, Y, Z, W) en un puntero vectorial indexado.
 * Utilizado por el motor para actualizar las coordenadas físicas de transformación de los widgets del HUD.
 * Dirección original en Ghidra: 0x00337C68 (PAL)
 *
 * @param x Componente físico X o canal de color Red (param_1).
 * @param y Componente físico Y o canal de color Green (param_2).
 * @param z Componente físico Z o canal de color Blue (param_3).
 * @param w Componente físico W o canal de control Alpha (param_4).
 * @param p_dest_struct Estructura contenedora del puntero destino real (param_5).
 */
void math_set_vector4_ptr(u32 x, u32 y, u32 z, u32 w, void* p_dest_struct) {
	// Recupera el puntero físico real almacenado en el desplazamiento +4
	u32** pp_vector_target = (u32**)((u8*)p_dest_struct + 4);
	u32* p_vector = *pp_vector_target;

	if (p_vector != NULL) {
		p_vector[0] = x;   // Componente X / R
		p_vector[1] = y;   // Componente Y / G
		p_vector[2] = z;   // Componente Z / B
		p_vector[3] = w;   // Componente W / A
	}
}

/**
 * @brief Inicializa las matrices de transformación espacial y vectores tridimensionales de un Widget del HUD.
 * Reserva los bloques de memoria requeridos en el pool y activa la visibilidad del componente.
 * Dirección original en Ghidra: 0x00337CA8 (PAL)
 */
void hud_clear_widget_matrices(u32* p_widget_transform, const char* text_ptr, long p_hud_pool,
	long p4, long p5, long p6, long p7, long p8) {

	// Guarda la dirección del pool de memoria de la interfaz en el offset 0xB
	p_widget_transform[0x0B] = (u32)p_hud_pool;

	if (p_hud_pool != 0) {
		u32* p_vector;

		// 1. Asignar y limpiar Vector 0 (Posición Inicial)
		int* node0 = hud_allocate_node((int*)p_hud_pool, (long)text_ptr, p_hud_pool, p4, p5, p6, p7, p8);
		p_vector = (u32*)core_identity_stub(0x10, node0);
		p_widget_transform[0] = (u32)p_vector;
		p_vector[0] = 0; p_vector[1] = 0; p_vector[2] = 0; p_vector[3] = 0;

		// 2. Asignar y limpiar Vector 2 (Rotación)
		int* node1 = hud_allocate_node((int*)p_widget_transform[0x0B], 0, 0, 0, 0, 0, 0, 0);
		p_vector = (u32*)core_identity_stub(0x10, node1);
		p_widget_transform[2] = (u32)p_vector;
		p_vector[0] = 0; p_vector[1] = 0; p_vector[2] = 0; p_vector[3] = 0;

		// 3. Asignar y limpiar Vector 1 (Escala)
		int* node2 = hud_allocate_node((int*)p_widget_transform[0x0B], 0, 0, 0, 0, 0, 0, 0);
		p_vector = (u32*)core_identity_stub(0x10, node2);
		p_widget_transform[1] = (u32)p_vector;
		p_vector[0] = 0; p_vector[1] = 0; p_vector[2] = 0; p_vector[3] = 0;

		// 4. Asignar y limpiar Vector 3 (Velocidad / Interpolación)
		int* node3 = hud_allocate_node((int*)p_widget_transform[0x0B], 0, 0, 0, 0, 0, 0, 0);
		p_vector = (u32*)core_identity_stub(0x10, node3);
		p_widget_transform[3] = (u32)p_vector;
		p_vector[0] = 0; p_vector[1] = 0; p_vector[2] = 0; p_vector[3] = 0;

		// 5. Asignar y limpiar Vector 4 (Desplazamiento Secundario)
		int* node4 = hud_allocate_node((int*)p_widget_transform[0x0B], 0, 0, 0, 0, 0, 0, 0);
		p_vector = (u32*)core_identity_stub(0x10, node4);
		p_widget_transform[4] = (u32)p_vector;
		p_vector[0] = 0; p_vector[1] = 0; p_vector[2] = 0; p_vector[3] = 0;
	}

	// Inicialización de flags físicos y offsets lógicos del motor gráfico
	p_widget_transform[10] = (u32)text_ptr;
	p_widget_transform[8] = 0;
	p_widget_transform[5] = 0;
	p_widget_transform[7] = 0;
	p_widget_transform[6] = 0;
	p_widget_transform[9] = 0;

	// Fuerza la activación de visibilidad para renderizar el componente en pantalla
	hud_set_widget_visibility(p_widget_transform, 1);
}


/**
 * @brief Asigna u obtiene un nodo de memoria libre para un componente visual del HUD (Pool Allocator).
 * Realiza comprobaciones estrictas de límites físicos y dispara aserciones ante desbordamientos de memoria del HUD.
 * Dirección original en Ghidra: 0x00338B98 (PAL)
 *
 * @param p_hud_pool Estructura de cabecera del pool de memoria de la interfaz (param_1).
 * @return int* Puntero al bloque de memoria del nodo inicializado listo para el widget.
 */
int* hud_allocate_node(int* p_hud_pool, long p2, long p3, long param_4,
	long param_5, long param_6, long param_7, long param_8) {
	int* p_allocated_node = (int*)p_hud_pool[5];

	// Caso A: No hay nodos libres reusables en la lista, se debe recortar memoria nueva
	if (p_allocated_node == NULL) {
		int current_offset = p_hud_pool[3];
		u32 next_target_size = current_offset + p_hud_pool[2];

		// Comprobación de desbordamiento de memoria del Pool de la interfaz
		if ((u32)p_hud_pool[1] < next_target_size) {
			// Invoca a tu manejador del kernel para congelar el software e informar la línea del bug
			sys_assert_dispatch((const char*)0x001adb18, 0x53, (const char*)0x001adb60,
				param_4, param_5, param_6, param_7, param_8);
			p_allocated_node = NULL;
		}
		else {
			p_hud_pool[3] = next_target_size;     // Avanza el puntero de asignación de memoria
			p_hud_pool[4] = p_hud_pool[4] + 1;     // Incrementa el contador de widgets activos
			p_allocated_node = (int*)(*p_hud_pool + current_offset); // Dirección física calculada
		}
	}
	// Caso B: Camino rápido (Recicla un nodo previamente liberado en la Free List)
	else {
		int next_free_node = *p_allocated_node;
		p_hud_pool[4] = p_hud_pool[4] + 1;         // Incrementa widgets activos
		p_hud_pool[5] = next_free_node;           // Mueve la cabecera al siguiente nodo libre
	}

	return p_allocated_node;
}


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

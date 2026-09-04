// src/hud_ammo.c
#include "types.h"

// Definición estimada de la estructura de un elemento del HUD
typedef struct {
	const char* label;
	u32 flags;
	s32 posX;
	s32 posY;
} HudElement;

/**
 * Función encargada de inicializar los componentes lógicos de la munición en el HUD.
 * Dirección aproximada en Ghidra: 0x0034D494 (Inicio del bloque visualizado)
 */
void hud_init_ammo_components(void* param_1, void* param_2) {
	// El descompilador de Ghidra muestra que se realizan múltiples operaciones
	// de cálculo de direcciones (addiu) y almacenamiento en el Stack (sw/sd).

	// Aquí comenzaremos a recrear la lógica funcional de asignación de los elementos:
	HudElement back_ammo;
	HudElement outline_ammo;
	HudElement ammo_text;
	HudElement ammo_icon;

	// En 0x0034d4d4 vemos que se referencia directamente la string "BackAmmo" (0x001ae6d8)
	back_ammo.label = (const char*)0x001ae6d8; // "BackAmmo"

	// Siguiendo la lógica de las strings consecutivas detectadas por Ghidra:
	outline_ammo.label = (const char*)0x001ae6e8; // "OutlineAmmo"
	ammo_text.label = (const char*)0x001ae700; // "AmmoText"
	ammo_icon.label = (const char*)0x001ae710; // "AmmoIcon"

	// TODO: Mapear los offsets aritméticos (0x158, 0x194, etc.) para identificar 
	// si corresponden a las coordenadas en pantalla de cada elemento gráfico.
}
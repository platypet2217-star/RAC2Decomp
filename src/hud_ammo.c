// src/hud_ammo.c
#include "types.h"

// Estructura interna estimada para los componentes visuales del HUD de munición
typedef struct {
    f32 posX;
    f32 posY;
    f32 scaleX;
    f32 scaleY;
    u32 colorRGBA;
    const char* assetName;
} HudAmmoWidget;

/**
 * @brief Inicializa o actualiza el estado y coordenadas de la interfaz de munición.
 * Dirección original en Ghidra: 0x0034D490 (PAL)
 *
 * @param p_hudState Puntero al estado global del HUD (param_1 / registro a0)
 * @param p_ammoData Puntero a los datos de munición del arma actual (param_2 / registro a1)
 */
void hud_ammo_update_or_init(void* p_hudState, s32 p_ammoData, long param_3) {
    // Las variables 'extraout' de Ghidra representan el coprocesador vectorial de la PS2 (VU0/VU1)
    // o registros flotantes cargando matrices de transformación para los elementos visuales.

    // El ensamblador muestra instrucciones 'swc1' (Store Word Coprocessor 1), 
    // lo que significa que el juego está guardando valores de punto flotante (f32) en el Stack.

    // Ejemplos de offsets de datos matemáticos detectados en el ensamblador (swc1 $f25, 0x128($sp)):
    // Estos corresponden a las posiciones de renderizado o transformaciones de las strings:
    // "BackAmmo", "OutlineAmmo", "AmmoText", "AmmoIcon"

    // TODO: Conectar con el depurador de PCSX2 para interceptar qué valores flotantes 
    // específicos se almacenan en los desplazamientos 0x128, 0x120 y 0x118 del Stack.
}

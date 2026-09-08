# English

# Español

# boot_txt_render_extended_string

## Función original

`boot_txt_render_extended_string`

## Propósito

Renderiza una cadena de texto utilizando el sistema de
tipografía utilizado durante el arranque/debug.

La función recibe una cadena y siete parámetros adicionales
que son reenviados al sistema de renderizado de texto.

## Comportamiento observado

1. Guarda el callback tipográfico actual.
2. Sustituye temporalmente el callback.
3. Construye un bloque local con los parámetros recibidos.
4. Llama a `txt_render_scientific_string()`.
5. Restaura el callback anterior.
6. Devuelve el resultado del renderizado.

## Dependencias

- `txt_render_scientific_string`
- `g_hud_typography_callback_ptr`
- `sys_debug_console_write_char`
- `custom_hud_glyph_decoder` [adaptación propia]

## Estado

Implementada.

## Pendientes

- Confirmar representación exacta de los siete parámetros.
- Revisar comportamiento de `txt_render_scientific_string`.
- Determinar si `custom_hud_glyph_decoder` forma parte de la
  adaptación PC o corresponde al comportamiento original.
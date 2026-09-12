#ifndef LIFECYCLE_H
#define LIFECYCLE_H

/*
 * Lista de callbacks de cierre del motor (LIFO).
 *   g_cleanup_table[0]  = header (-1 = "lista terminada en 0")
 *   g_cleanup_table[1..N] = punteros a funciones de cleanup
 *   g_cleanup_table[N+1]  = 0 (sentinela)
 *
 * En PS2 la llenaba el loader del IOP (otro binario, vía SIF/RPC).
 * En PC la llenamos nosotros: cada recurso que se crea registra aquí
 * su destructor, y run_cleanup_callbacks() los ejecuta en reversa al salir.
 */
extern void (*g_cleanup_table[])(void);

/* Registra un destructor. Devuelve el slot asignado, o -1 si el array está lleno. */
int  cleanup_register(void (*fn)(void));

/* Ejecuta todos los callbacks registrados, del último al primero (LIFO).
 *   Con la tabla vacía (BSS=0) es un no-op seguro. */
void run_cleanup_callbacks(void);

#endif /* LIFECYCLE_H */

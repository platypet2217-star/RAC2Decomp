#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

/* Inicializa el canal de display (GS) del motor.
 *   Idempotente: solo corre si RCNT3_FLAG_INIT no está set.
 *   [MOD-PENDING] El nº de buffers (6) es hardcode; para mods
 *   de render (triple/quad buffer) externalizar a data/. */
void display_init_channel(void);

#endif

#ifndef SCE_COMPAT_H
#define SCE_COMPAT_H

/*
 * Capa de compatibilidad: re-implementa (o no-op) las llamadas al
 * OSR / kernel de PS2 que el ELF original hace, para que el código
 * descripto de Ghidra compile y corra tal cual en PC.
 *
 * Regla de uso: el resto del proyecto llama a estas funciones SIN
 * #ifdef. La decisión de "hacer algo o no" vive aquí, en un solo lugar.
 */

 /* sceFlushCache(mode, addr, size)  ->  syscall 100 en PS2.
  *   mode: 0 invalidate, 1 flush, 2 invalidate+flush */
int  sceFlushCache(int mode, void* addr, int size);

/* (aquí van las próximas: sceSif*, sceGs*, scePad*, ...) */

#endif /* SCE_COMPAT_H */

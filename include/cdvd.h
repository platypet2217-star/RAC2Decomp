#ifndef CDVD_H
#define CDVD_H

extern int g_CdvdNcmdInitialized;
extern int g_CdvdCurrentCommand;

/**
 * @brief Inicializa el sistema de lectura de archivos emulado para PC.
 * @param mode Modo de inicialización original de la PS2.
 * @return 1 para éxito, 0 para fallo.
 */
int sceCdInit(int mode);

// ... Mantener lo anterior (sceCdInit, etc.) ...

/**
 * @brief Detiene el motor de rotación virtual del lector de discos en PC.
 * @return Siempre retorna 0 por compatibilidad con el Kernel original.
 */
int sceCdStop(void);

#endif // CDVD_H

#ifndef PS2_KERNEL_H
#define PS2_KERNEL_H

#include <stddef.h>

/**
 * @brief Compara dos bloques de memoria de forma fiel a la optimización de PS2.
 * @return 0 si son iguales, o la diferencia entre los primeros bytes que difieren.
 */
int ee_memcmp(const void* ptr1, const void* ptr2, size_t num);

// ... Mantener lo anterior ...

/**
 * @brief Convierte una cadena de caracteres en un número entero decimal.
 * @param str Puntero a la cadena de texto a convertir.
 * @return El valor entero resultante de la conversión.
 */
int ee_atoi(const char* str);

// ... Mantener lo anterior ...

/**
 * @brief Vaciado manual del caché de la CPU (Emotion Engine).
 * @note En PC nativo es una operación nula (NOP) debido a la coherencia de hardware moderna.
 * @param mode El modo de vaciado (Originalmente 0 = Instruction & Data Cache).
 */
void sceFlushCache(int mode);

#endif

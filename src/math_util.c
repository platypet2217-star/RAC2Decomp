// src/math_util.c
#include "types.h"

/**
 * @brief División de 64 bits con signo (Equivalente portátil a __divdi3).
 * Sostiene las operaciones lógicas de división con números negativos en el hardware de la PS2.
 * Dirección original en Ghidra: 0x00121B20 (PAL)
 *
 * @param dividend Número de 64-bits con signo (param_1)
 * @param divisor Número de 64-bits con signo (param_2)
 * @return long El cociente con signo de la división.
 */
s64 math_div64(s64 dividend, s64 divisor) {
	if (divisor == 0) {
		return 0; // Salvaguarda contra división por cero
	}

	// En arquitecturas modernas, el compilador traduce esto a una sola instrucción de CPU
	return dividend / divisor;
}

/**
 * @brief División y módulo de 64 bits sin signo (Equivalente portátil a __udivdi3).
 * Rutina de software utilizada originalmente en la PS2 para suplir la falta de división nativa de 64-bits.
 * Dirección original en Ghidra: 0x001220F0 (PAL)
 *
 * @param dividend Número de 64-bits a dividir (param_1)
 * @param divisor Número de 64-bits que divide (param_2)
 * @return ulong El cociente de la división.
 */
u64 math_udiv64(u64 dividend, u64 divisor) {
	// Si en sistemas modernos ocurre una división por cero, el sistema operativo
	// arrojará la señal correspondiente de forma nativa (equivalente al trap(7) de la PS2).
	if (divisor == 0) {
		// Comportamiento de salvaguarda
		return 0;
	}

	// En PC/Consolas modernas, esta línea de C se compila en una sola instrucción de CPU,
	// reemplazando por completo las más de 100 líneas de ensamblador de la PS2.
	return dividend / divisor;
}

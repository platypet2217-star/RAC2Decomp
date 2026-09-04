// include/types.h
#ifndef TYPES_H
#define TYPES_H

// Tipos de datos estándar de PlayStation 2 (Emotion Engine)
typedef unsigned char      u8;   // 8 bits sin signo (0 a 255)
typedef unsigned short     u16;  // 16 bits sin signo
typedef unsigned int       u32;  // 32 bits sin signo (Direcciones de memoria)
typedef unsigned long long u64;  // 64 bits sin signo

typedef signed char        s8;   // 8 bits con signo
typedef signed short       s16;  // 16 bits con signo
typedef signed int         s32;  // 32 bits con signo
typedef signed long long   s64;  // 64 bits con signo

typedef float              f32;  // Punto flotante estándar de 32 bits

// Estructuras matemáticas vectoriales comunes en el motor de Insomniac
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vector3;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vector4;

#endif // TYPES_H

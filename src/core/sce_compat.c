#include "core/sce_compat.h"

/*
 * sceFlushCache
 * ------------------------------------------------------------------
 * PS2:  li $v0, 100 ; syscall   (coherencia de i/d-cache sobre [addr,addr+size])
 * PC:   la coherencia la garantiza el hardware (MESI/MOESI). No-op seguro.
 *
 * Se deja el cuerpo vacío a propósito: en x86_64/ARM no hay que forzar
 * un wbinvd/clflush por región como en PS2, y hacerlo penalizaría sin
 * aportar nada. Si en el futuro se detecta un bug de coherencia al
 * compartir memoria con la GPU, se rellena aquí y en ningún otro sitio.
 */
int sceFlushCache(int mode, void* addr, int size)
{
#if defined(PLATFORM_PS2)
    /* li $v0, 100 ; syscall  -- $a0=mode $a1=addr $a2=size */
    register int   r_mode __asm__("a0") = mode;
    register void* r_addr __asm__("a1") = addr;
    register int   r_size __asm__("a2") = size;
    __asm__ volatile ("li v0, 100\n\t syscall\n\t"
        : /* out */
    : "r"(r_mode), "r"(r_addr), "r"(r_size)
        : "memory", "v0");
    return 0;
#else
    (void)mode;
    (void)addr;
    (void)size;
    return 0;
#endif
}

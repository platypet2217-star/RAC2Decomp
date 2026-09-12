#include "engine/display_init.h"
#include "engine/motor_regs.h"
#include "core/sce_compat.h"

/* Nº de buffers de display que el handshake inicializa (2..7 = 6 buffers).
 * [MOD-PENDING] Hardcode del ELF. Para mods → cargar de data/levels o
 * config. Mientras, se define como macro para que el cambio sea local. */
#define DISPLAY_INIT_BUFFERS  8u

void display_init_channel(void)
{
    uint32_t cnt;

    if ((g_rcnt3_mode & RCNT3_FLAG_INIT) == 0u)
    {
        cnt = 2u;

        /* Fase 1: kicks + waits + flushes (handshake inicial EE<->IOP) */
        sce_stub_syscall116();      /* kick  */
        sce_stub_syscall90();       /* wait  */
        sce_stub_syscall90();       /* wait  */
        sceFlushCache(0, 0, 0);     /* flush */
        sceFlushCache(0, 0, 0);     /* flush */
        sce_stub_syscall116();      /* kick  */

        /* Fase 2: handshake por buffer (6 iteraciones) */
        do {
            cnt = cnt + 1u;
            sce_stub_syscall91();      /* signal: libera el buffer anterior */
            sce_stub_syscall116();     /* kick:   arranca el buffer siguiente */
        } while (cnt < DISPLAY_INIT_BUFFERS);

        /* [CONFIRM] Marcar canal como inicializado (guard de idempotencia).
         * El ELF probablemente lo hace vía write a un registro que Ghidra
         * no resolvió; lo seteo aquí para que la 2ª llamada no re-inicie. */
        g_rcnt3_mode |= RCNT3_FLAG_INIT;
    }
    /* Si ya estaba set, no se hace nada: idempotente. */
}

#include "core/sce_compat.h"

/* ------------------------------------------------------------------ */
/*  Stub: syscall 116 – "kick" de transferencia SIF/DMA               */
/*  PS2:  li v1, 0x74 ; syscall                                       */
/*  PC:   no-op                                                       */
/* ------------------------------------------------------------------ */
int sce_stub_syscall116(void)
{
#if defined(PLATFORM_PS2)
	__asm__ volatile ("li v0, 0x74\n\t syscall\n\t" : : : "memory", "v0");
	return 0;
#else
	return 0;
#endif
}

/* ------------------------------------------------------------------ */
/*  Stub: syscall 131 – "poll" de estado del anillo                   */
/*  PS2:  li v1, 0x83 ; syscall                                       */
/*  PC:   no-op, devuelve 0                                           */
/* ------------------------------------------------------------------ */
int sce_stub_syscall131(void)
{
#if defined(PLATFORM_PS2)
	int r;
	__asm__ volatile ("li v0, 0x83\n\t syscall\n\t"
		: "=v"(r)
		: : "memory", "v0");
	return r;
#else
	return 0;
#endif
}

/* ------------------------------------------------------------------ */
/*  Global de estado (DAT_00134e20 en el ELF)                         */
/*  [MOD-PENDING] Identificar quién lee esta global y ponerle el      */
/*  nombre real del motor Insomniac.                                   */
/* ------------------------------------------------------------------ */
int g_sync_state = 0;

/* ------------------------------------------------------------------ */
/*  sce_sync_barrier  (FUN_0011f718)                                  */
/*                                                                     */
/*  PS2 original:                                                     */
/*    kick(); kick();                                                 */
/*    head_a = poll() - 0x20c;                                        */
/*    head_b = poll() - 0x168;                                        */
/*    while (head_a != head_b) { spin... }                            */
/*    DAT_00134e20 = head_a;                                          */
/*                                                                     */
/*  PC: no hay IOP ni anillo SIF. El "spin" es inútil (ambos          */
/*  stubs devuelven 0 → convergen al instante).  Solo escribimos      */
/*  la global para no romper la cadena de dependencias.               */
/* ------------------------------------------------------------------ */
void sce_sync_barrier(void)
{
	int head_a, head_b;

#if defined(PLATFORM_PS2)
	sce_stub_syscall116();   /* kick 1 */
	sce_stub_syscall116();   /* kick 2 */

	head_a = sce_stub_syscall131() - 0x20c;
	head_b = sce_stub_syscall131() - 0x168;

	/* Spin-wait: re-poll el cabezal que va "detrás" hasta converger */
	while (head_a != head_b) {
		if (head_a < head_b)
			head_a = sce_stub_syscall131() - 0x20c;
		else
			head_b = sce_stub_syscall131() - 0x168;
	}
#else
	/* PC: ambos stubs devuelven 0, el while no se entra nunca.
	   Se deja el código para que la lógica sea 1:1 con el ELF
	   y sirva de referencia si alguien emula. */
	(void)head_a;
	(void)head_b;
	head_a = sce_stub_syscall131() - 0x20c;  /* 0 - 0x20c */
	head_b = sce_stub_syscall131() - 0x168;  /* 0 - 0x168 */
	/* head_a != head_b → se entra al while → convergen en 1 iter */
	while (head_a != head_b) {
		if (head_a < head_b)
			head_a = sce_stub_syscall131() - 0x20c;
		else
			head_b = sce_stub_syscall131() - 0x168;
	}
#endif

	g_sync_state = head_a;
}

/* ------------------------------------------------------------------ */
/*  Stub: syscall 64 – sceSemaCreate (kernel semaphore)              */
/*  PS2:  li v0, 0x40 ; syscall   (param en $a0)                     */
/*  PC:   devuelve un ID virtual estable. No crea SDL_Semaphore:     */
/*  eso lo hace la capa engine (Sys_InitGraphicsSemaphore).          */
/* ------------------------------------------------------------------ */
int sceSemaCreate(void* param)
{
#if defined(PLATFORM_PS2)
	register void* r_param __asm__("a0") = param;
	int r;
	__asm__ volatile ("li v0, 0x40\n\t syscall\n\t"
		: "=v"(r)
		: "r"(r_param)
		: "memory", "v0");
	return r;
#else
	(void)param;
	static int virtual_sema_counter = 4;   /* 4 = 1-based; 0 reservado como invalid */
	return virtual_sema_counter++;
#endif
}

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


/* ------------------------------------------------------------------ */
/*  Stub: syscall 90 – event-set / cola de eventos del kernel         */
/*  PS2:  li v0, 0x5a ; syscall                                       */
/*  PC:   no-op (no hay event-sets del IOP que esperar)               */
/* ------------------------------------------------------------------ */
int sce_stub_syscall90(void)
{
#if defined(PLATFORM_PS2)
	__asm__ volatile ("li v0, 0x5a\n\t syscall\n\t" : : : "memory", "v0");
	return 0;
#else
	return 0;
#endif
}

/* ------------------------------------------------------------------ */
/*  Stub: syscall 91 – signal de event-set (libera el evento)         */
/*  PS2:  li v0, 0x5b ; syscall                                       */
/*  PC:   no-op (no hay event-sets del IOP que liberar)               */
/* ------------------------------------------------------------------ */
int sce_stub_syscall91(void)
{
#if defined(PLATFORM_PS2)
	__asm__ volatile ("li v0, 0x5b\n\t syscall\n\t" : : : "memory", "v0");
	return 0;
#else
	return 0;
#endif
}

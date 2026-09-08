#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "graphics.h"
#include "system.h"
#include "ps2_kernel.h"

// Punteros globales para la ventana moderna de PC
SDL_Window* g_MainWindow = NULL;
SDL_Renderer* g_MainRenderer = NULL;

// Declaración de la función principal descompilada del juego (La que arranca todo en la intro)
// TODO: Reemplazar con el nombre real de tu función de arranque si ya la renombraste en Ghidra
extern void Game_InitializeAllSystems(void);

int main(int argc, char* argv[]) {
	(void)argc; (void)argv; // Evitar advertencias de parámetros no usados

	printf("[RAC2PC] Iniciando port nativo de Ratchet & Clank 2 (PAL)...\n");

	// 1. Inicializar la capa del sistema de temporizadores y semáforos
	Sys_InitGraphicsSemaphore();
	Sys_InitRenderBuffers();

	// 2. Configurar el entorno de pantalla moderno (Por defecto a 1080p, 60 FPS)
	// El juego llamará internamente a sceGsDefDispEnv para registrar sus modos.
	Graphics_SetCustomResolution(1920, 1080, 60.0f);

	// 3. Inicializar el subsistema de video nativo de PC con SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "[ERROR] No se pudo inicializar SDL2: %s\n", SDL_GetError());
		return -1;
	}

	// Creamos la ventana de PC leyendo los valores modernos que inyectamos en tu estructura
	g_MainWindow = SDL_CreateWindow(
		"Ratchet & Clank 2: Going Commando - Native PC Port",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		g_GraphicsCanvasData.width_modern, g_GraphicsCanvasData.height_modern,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (!g_MainWindow) {
		fprintf(stderr, "[ERROR] No se pudo crear la ventana: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	g_MainRenderer = SDL_CreateRenderer(g_MainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!g_MainRenderer) {
		fprintf(stderr, "[ERROR] No se pudo crear el renderizador: %s\n", SDL_GetError());
		SDL_DestroyWindow(g_MainWindow);
		SDL_Quit();
		return -1;
	}

	// 4. Arrancar el código descompilado del juego
	// Esto llamará a Graphics_InitSifInterface, cargar rom0:ROMVER (que ya puenteamos), etc.
	printf("[RAC2PC] Saltando al bucle de inicialización del motor original...\n");

	// Aquí el juego cargará la intro y correrá el lazo principal que sincronizamos con los semáforos
	// Game_InitializeAllSystems();

	// 5. Bucle de ejecución de emergencia para mantener la ventana viva si el motor retorna
	bool running = true;
	SDL_Event event;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}

		// Limpiar pantalla en PC (Fondo negro de carga original)
		SDL_SetRenderDrawColor(g_MainRenderer, 0, 0, 0, 255);
		SDL_RenderClear(g_MainRenderer);

		// Aquí tu backend gráfico moderno dibujará el framebuffer escalado cuando conectemos el rasterizador

		SDL_RenderPresent(g_MainRenderer);
	}

	// Limpieza al cerrar el ejecutable
	printf("[RAC2PC] Cerrando juego de forma segura y liberando semáforos...\n");
	sceDeleteSema(g_GraphicsSemaphoreID);
	sceDeleteSema(g_RenderSemaphoreID_A);
	sceDeleteSema(g_RenderSemaphoreID_B);

	SDL_DestroyRenderer(g_MainRenderer);
	SDL_DestroyWindow(g_MainWindow);
	SDL_Quit();

	return 0;
}

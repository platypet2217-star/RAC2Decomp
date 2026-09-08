# English

# Español

# Subsistema Gráfico y Control de Cuadros (GS / V-Sync Sync)

Este documento registra los puntos de control del Sintetizador Gráfico (GS) y la estrategia para desligar el motor lúdico del límite físico original de la consola.

## Logs de Integridad del Bus DMA Gráfico (VIF/GIF Channels)

| Dirección de Memoria | Tipo de Dato | Valor de Cadena Estática | Propósito / Uso en el Motor |
| :--- | :--- | :--- | :--- |
| **`0x0013B563`** | `char[]` | `"sceGsExecLoadImage: DMA Ch.2 does not terminate...\r\n"` | Error crítico cuando el canal DMA 2 (GIF) se congela al transferir texturas a la VRAM. |
| **`0x0013B5D1`** | `char[]` | `"sceGsExecStoreImage: GS does not terminate...\r\n"` | Error crítico cuando el pipeline gráfico colapsa al intentar volcar el buffer de dibujado. |

## Estrategia de FPS Dinámicos para el Port de PC
* **Lazo Lúdico (Engine Tick):** Fijado lógicamente a pasos fijos equivalentes a la tasa original (16.66ms para emular 60Hz nativos).
* **Lazo Visual (Render Frame Rate):** Desacoplado mediante interpolación lineal de variables lúdicas (`DeltaTime` en PC), permitiendo tasas de refresco escalables (60Hz, 144Hz, Uncapped) configurables por el usuario.

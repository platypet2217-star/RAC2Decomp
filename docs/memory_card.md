# English

# Español

# Documentación del Subsistema de Memory Card - Ratchet & Clank 2 (PAL)

Este documento centraliza las cadenas de texto y las llamadas al SDK de Sony (`libmc`) utilizadas para gestionar el progreso de la partida.

## Plantillas de Formato de Archivo (Memory Card Path Template)

| Dirección de Memoria | Tipo de Dato | Valor de Cadena Estática | Propósito / Uso en el Motor |
| :--- | :--- | :--- | :--- |
| **`0x001A9AA2`** | `char[]` (String) | `"BESCES-50916RATCHET/save%d.bin"` | Máscara de formato utilizada por vsnprintf para definir la ruta del archivo binario de la partida en el slot (`mc0:` / `mc1:`). |


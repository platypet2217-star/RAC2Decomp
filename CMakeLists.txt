cmake_minimum_required(VERSION 3.15)
project(RAC2Decomp C)

# 1. Configurar el estándar de C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 2. Configurar RUTAS EXPLÍCITAS de SDL2 (Sin usar find_package)
include_directories("C:/SDL2/include")
link_directories("C:/SDL2/lib/x64")

# 3. Indicar dónde están tus archivos de cabecera (.h) locales de RAC2
include_directories(include)

# 4. Recolectar automáticamente todos tus archivos fuente .c (incluyendo subcarpetas)
file(GLOB_RECURSE SOURCES 
    "src/*.c"
    "src/platform/ps2/*.c"
)

# 5. Definir el nombre del archivo ejecutable final (.exe)
add_executable(RAC2PC ${SOURCES})

# 6. Enlazar directamente las librerías físicas de Windows
target_link_libraries(RAC2PC PRIVATE "C:/SDL2/lib/x64/SDL2.lib" "C:/SDL2/lib/x64/SDL2main.lib")

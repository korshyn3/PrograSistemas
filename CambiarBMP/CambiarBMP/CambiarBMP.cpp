// CambiarBMP.cpp
// Programa de consola para visualizar y cambiar el color de los pixeles
// de una imagen BMP de 24 bits por pixel (sin compresion).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} CabeceraArchivoBMP;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} CabeceraInfoBMP;
#pragma pack(pop)

// Habilita el interprete de codigos ANSI en la consola de Windows,
// necesario para poder pintar colores de fondo con codigos "\x1b[48;2;r;g;bm".
static void habilitarColorANSI(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (hOut == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleMode(hOut, &modo)) return;
    SetConsoleMode(hOut, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

static void limpiarBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Lee un archivo completo a memoria. Devuelve el buffer y, por parametro,
// el tamanio en bytes. Devuelve NULL si no se pudo abrir/leer.
static uint8_t* leerArchivo(const char* nombre, int64_t* tamanioSalida) {
    FILE* f = fopen(nombre, "rb");
    if (f == NULL) {
        printf("No se pudo abrir el archivo \"%s\".\n", nombre);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int64_t tamanio = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*)malloc((size_t)tamanio);
    if (buffer == NULL) {
        printf("No hay memoria suficiente para cargar el archivo.\n");
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, (size_t)tamanio, f);
    fclose(f);

    *tamanioSalida = tamanio;
    return buffer;
}

static int guardarArchivo(const char* nombre, uint8_t* buffer, int64_t tamanio) {
    FILE* f = fopen(nombre, "wb");
    if (f == NULL) return 0;
    fwrite(buffer, 1, (size_t)tamanio, f);
    fclose(f);
    return 1;
}

// Numero de bytes por fila en el archivo (cada fila BMP se rellena a multiplo de 4).
static int32_t bytesPorFila(const CabeceraInfoBMP* info) {
    return ((info->biWidth * info->biBitCount + 31) / 32) * 4;
}

// Devuelve un puntero a los 3 bytes (B, G, R) del pixel (x, y).
// (0,0) es la esquina superior izquierda de la imagen, tal como se ve en pantalla.
static uint8_t* obtenerPixel(uint8_t* buffer, const CabeceraInfoBMP* info,
                              uint32_t offBits, int32_t x, int32_t y) {
    int32_t alturaAbs = info->biHeight < 0 ? -info->biHeight : info->biHeight;
    int32_t topDown = info->biHeight < 0;
    int32_t filaArchivo = topDown ? y : (alturaAbs - 1 - y);
    int32_t fila = bytesPorFila(info);
    return buffer + offBits + (int64_t)filaArchivo * fila + (int64_t)x * 3;
}

static void mostrarInfo(const CabeceraArchivoBMP* fh, const CabeceraInfoBMP* ih) {
    int32_t alturaAbs = ih->biHeight < 0 ? -ih->biHeight : ih->biHeight;
    printf("\n--- Informacion del BMP ---\n");
    printf("Ancho:         %d px\n", ih->biWidth);
    printf("Alto:          %d px\n", alturaAbs);
    printf("Bits/pixel:    %d\n", ih->biBitCount);
    printf("Tamanio total: %u bytes\n", fh->bfSize);
    printf("----------------------------\n");
}

// Dibuja la imagen en la terminal usando bloques de color (fondo ANSI truecolor).
// Si la imagen es pequenia, agrega una regla de coordenadas para facilitar elegir
// el pixel a cambiar.
static void mostrarPixeles(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits) {
    int32_t ancho = info->biWidth;
    int32_t alto = info->biHeight < 0 ? -info->biHeight : info->biHeight;
    int mostrarRegla = (ancho <= 60 && alto <= 60);

    printf("\n");
    if (mostrarRegla) {
        printf("      ");
        for (int32_t x = 0; x < ancho; x++) printf("%2d", x % 100);
        printf("\n");
    }

    for (int32_t y = 0; y < alto; y++) {
        if (mostrarRegla) printf("y=%3d ", y);
        for (int32_t x = 0; x < ancho; x++) {
            uint8_t* p = obtenerPixel(buffer, info, offBits, x, y);
            uint8_t b = p[0], g = p[1], r = p[2];
            printf("\x1b[48;2;%d;%d;%dm  \x1b[0m", r, g, b);
        }
        printf("\n");
    }
    printf("\n");
}

static void cambiarPixel(uint8_t* buffer, const CabeceraInfoBMP* info, uint32_t offBits,
                          int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t* p = obtenerPixel(buffer, info, offBits, x, y);
    p[0] = b;
    p[1] = g;
    p[2] = r;
}

static void mostrarMenu(void) {
    printf("\n=== CambiarBMP ===\n");
    printf("1. Ver informacion del archivo\n");
    printf("2. Ver pixeles\n");
    printf("3. Cambiar color de un pixel\n");
    printf("4. Guardar cambios\n");
    printf("0. Salir\n");
    printf("Opcion: ");
}

int main() {
    habilitarColorANSI();

    char nombreArchivo[260];
    printf("Archivo BMP a abrir (ENTER para usar \"azul_1px.bmp\"): ");
    if (fgets(nombreArchivo, sizeof(nombreArchivo), stdin) == NULL) return 1;
    nombreArchivo[strcspn(nombreArchivo, "\n")] = '\0';

    // Si la entrada trae un BOM UTF-8 (comun al redirigir desde archivos de texto en Windows), se descarta.
    if ((unsigned char)nombreArchivo[0] == 0xEF && (unsigned char)nombreArchivo[1] == 0xBB && (unsigned char)nombreArchivo[2] == 0xBF) {
        memmove(nombreArchivo, nombreArchivo + 3, strlen(nombreArchivo + 3) + 1);
    }

    if (nombreArchivo[0] == '\0') {
        strcpy(nombreArchivo, "azul_1px.bmp");
    }

    int64_t tamanio = 0;
    uint8_t* buffer = leerArchivo(nombreArchivo, &tamanio);
    if (buffer == NULL) return 1;

    if ((size_t)tamanio < sizeof(CabeceraArchivoBMP) + sizeof(CabeceraInfoBMP)) {
        printf("El archivo es demasiado pequenio para ser un BMP valido.\n");
        free(buffer);
        return 1;
    }

    CabeceraArchivoBMP* fileHeader = (CabeceraArchivoBMP*)buffer;
    CabeceraInfoBMP* infoHeader = (CabeceraInfoBMP*)(buffer + sizeof(CabeceraArchivoBMP));

    if (fileHeader->bfType != 0x4D42) { // "BM"
        printf("El archivo no tiene la firma BMP valida.\n");
        free(buffer);
        return 1;
    }
    if (infoHeader->biBitCount != 24) {
        printf("Este programa solo soporta BMP de 24 bits por pixel (el archivo tiene %d).\n", infoHeader->biBitCount);
        free(buffer);
        return 1;
    }
    if (infoHeader->biCompression != 0) {
        printf("Este programa solo soporta BMP sin compresion.\n");
        free(buffer);
        return 1;
    }

    int opcion;
    do {
        mostrarMenu();
        if (scanf("%d", &opcion) != 1) {
            limpiarBufferEntrada();
            printf("Entrada invalida.\n");
            continue;
        }
        limpiarBufferEntrada();

        int32_t ancho = infoHeader->biWidth;
        int32_t alto = infoHeader->biHeight < 0 ? -infoHeader->biHeight : infoHeader->biHeight;

        switch (opcion) {
        case 1:
            mostrarInfo(fileHeader, infoHeader);
            break;
        case 2:
            mostrarPixeles(buffer, infoHeader, fileHeader->bfOffBits);
            break;
        case 3: {
            int32_t x, y, r, g, b;
            printf("Coordenada x (0 a %d): ", ancho - 1);
            scanf("%d", &x);
            printf("Coordenada y (0 a %d): ", alto - 1);
            scanf("%d", &y);
            if (x < 0 || x >= ancho || y < 0 || y >= alto) {
                printf("Coordenadas fuera de rango.\n");
                limpiarBufferEntrada();
                break;
            }
            printf("Nuevo valor R (0-255): ");
            scanf("%d", &r);
            printf("Nuevo valor G (0-255): ");
            scanf("%d", &g);
            printf("Nuevo valor B (0-255): ");
            scanf("%d", &b);
            limpiarBufferEntrada();
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                printf("Los valores de color deben estar entre 0 y 255.\n");
                break;
            }
            cambiarPixel(buffer, infoHeader, fileHeader->bfOffBits, x, y,
                         (uint8_t)r, (uint8_t)g, (uint8_t)b);
            printf("Pixel (%d, %d) actualizado en memoria. Use la opcion 4 para guardar.\n", x, y);
            break;
        }
        case 4:
            if (guardarArchivo(nombreArchivo, buffer, tamanio)) {
                printf("Cambios guardados en \"%s\".\n", nombreArchivo);
            } else {
                printf("No se pudo guardar el archivo.\n");
            }
            break;
        case 0:
            printf("Saliendo...\n");
            break;
        default:
            printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    free(buffer);
    return 0;
}

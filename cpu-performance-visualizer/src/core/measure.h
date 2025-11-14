#ifndef MEASURE_H
#define MEASURE_H

#include <stddef.h>   // für size_t
#include <stdint.h>   // für uint64_t
#ifdef __cplusplus
extern "C" {



// Verhindert C++ Name-Mangling bei C-Funktionen
#endif
// Misst, wie viele CPU-Taktzyklen ein Speicherzugriff auf 'data' mit gegebener Größe benötigt.
// data: Zeiger auf das Datenarray, das im Speicher bearbeitet wird.
// size: Anzahl der Elemente im Array.
// Rückgabewert: Anzahl der CPU-Zyklen (64-Bit, da der Zähler sehr groß werden kann).
uint64_t measure_access_time(int *data, size_t size, int mode);

#ifdef __cplusplus
}
#endif
#endif

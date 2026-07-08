#ifndef __TYPES_H__
#define __TYPES_H__

// C/C++
// typedef int Type;

// C++11, C++14, C++17, C++20, C++23 ...
using Type = int;

// T1 must be int for 32-bit architecture and long long for 64-bit architecture
// It must work for windows, linux, iOS, macOS, android, etc.
// Se usa para todo indice, posicion o contador (reemplaza al "int" nativo).
using T1 = int;

// Tipo generico para referencias/IDs de objeto y contadores de uso (reemplaza al "long" nativo).
using Ref = long;

using TypeBTree = char;  // tipo de clave del BTree
using Character = char;  // caracter individual (parseo de formato, delimitadores)
#endif // __TYPES_H__
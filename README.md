# Image Equalization

Programa de visión artificial desarrollado en C++ utilizando OpenCV.

## Descripción

Este programa implementa técnicas de ecualización de histogramas para mejorar el contraste de imágenes digitales.

El objetivo es conseguir una distribución más equilibrada de los niveles de intensidad, mejorando la visibilidad de los detalles en zonas oscuras y claras de la imagen.

## Funcionamiento

El programa realiza los siguientes pasos:

1. Carga la imagen de entrada utilizando OpenCV.
2. Calcula el histograma de intensidades de la imagen.
3. Analiza la distribución de los niveles de intensidad.
4. Aplica técnicas de ecualización para redistribuir los valores de intensidad y mejorar el contraste.
5. Genera la imagen resultante con el contraste mejorado.

Para ejecutar el programa:

```bash
./img_equalization <../data/ejemplo.jpg> <salida.jpg>
```


### Modo interactivo

El programa dispone de un modo interactivo que se activa mediante el argumento `-i`.

Este modo permite visualizar los resultados y ajustar los parámetros del balance de color de forma interactiva, facilitando la comparación de los diferentes resultados.

Para activarlo, añadir `-i` a los argumentos de ejecución:

```bash

./color_balance -i <../data/ejemplo.jpg> <salida.jpg>

```

En caso de necesitar una imagen de entrada, se deben proporcionar también los argumentos correspondientes.

## Requisitos

- C++
- OpenCV
- CMake
- Compilador compatible con C++

## Compilación

Desde el directorio principal del proyecto:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Directorio `data`

El directorio `data/` contiene las imágenes utilizadas para probar el funcionamiento del programa.

Estas imágenes permiten comprobar los resultados de la ecualización de histogramas sobre diferentes ejemplos y comparar las imágenes originales con las procesadas.
# Fusion Studio — primera etapa

Aplicación independiente de Windows para revisar la GUI sin cargar código en Minecraft.

## Qué hace

- Abre su propia ventana Win32/OpenGL con la interfaz compartida `fusionGui.h`.
- Sus switches y sliders solo actualizan valores locales: no hay juego conectado.
- Guarda todos los valores editables y amigos en perfiles JSON separados, bajo `%LOCALAPPDATA%/FusionStudio/profiles`.
- Valida nombres y contenido antes de cargar un perfil; un perfil inválido no aplica cambios parciales.
- Cierra con X, con el cierre normal de Windows o con Close preview.
- Tema carbón compacto, tipografía Segoe UI/Segoe UI Semibold de Windows (con fuente de respaldo), iconos vectoriales y switches animados.
- Appearance ofrece cinco acentos y un color personalizado con entrada hexadecimal; se guarda en `appearance.json` al salir o pulsar Save appearance.
- La ilustración es opcional y aparece solo en ventanas suficientemente anchas.
- El guardado reemplaza el archivo mediante un temporal completo, sin truncar primero el perfil anterior. Las listas de color deben tener exactamente cuatro componentes.

## Qué se compila

Solo `standalone/main.cpp`, Dear ImGui y sus backends Win32/OpenGL2. No se enlazan JNI, JVM, MinHook, el arranque de la DLL ni los módulos del juego. No hay código de inyección, descargas, autoarranque ni actualización automática en esta aplicación.

Los componentes Win32 de ImGui conservan sus funciones estándar de teclado, ratón, portapapeles y DPI. El soporte de mandos está desactivado en esta aplicación.

## Compilar y comprobar

Con Visual Studio 2022 y CMake:

```powershell
cmake -S standalone -B standalone/build -A x64
cmake --build standalone/build --config Release
ctest --test-dir standalone/build -C Release --output-on-failure
```

El ejecutable queda en `standalone/build/Release/FusionStudio.exe`. El workflow `Build standalone GUI` compila y ejecuta las comprobaciones, y publica únicamente el EXE y su hash en el log.

Las comprobaciones cubren guardado/carga de valores en memoria, rechazo atómico de datos inválidos, nombres de perfiles, cinco páginas en tres tamaños de ventana y un switch mediante eventos internos de ImGui. No simulan ni inyectan entrada en Windows o en un juego.

Esta es una base para revisar por partes, no una certificación de seguridad. La compatibilidad con Minecraft y la detección de Defender de la DLL anterior son asuntos separados.

Se conserva la licencia GPLv3 y los créditos de Fusion/Fusion+ del repositorio. Dear ImGui conserva sus avisos de licencia en los fuentes.

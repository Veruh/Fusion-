# Fusion Studio — primera etapa

Aplicación independiente de Windows para revisar la GUI sin cargar código en Minecraft.

## Qué hace

- Abre su propia ventana Win32/OpenGL con la interfaz compartida `fusionGui.h`.
- Sus switches y sliders solo actualizan valores locales: no hay juego conectado.
- Guarda todos los valores editables y amigos en perfiles JSON separados, bajo `%LOCALAPPDATA%/FusionStudio/profiles`.
- Valida nombres y contenido antes de cargar un perfil; un perfil inválido no aplica cambios parciales.
- Cierra con X, con el cierre normal de Windows o con Close preview.
- Negros cálidos y acento esmeralda centralizados en premiumTheme.h. Inter Regular y Semibold integradas como recursos del EXE, marca FUSION con espaciado propio, iconos vectoriales y switches personalizados.
- Appearance ofrece variantes Emerald/Jade/Forest e intensidad dentro de la familia esmeralda; se guarda en `appearance.json` al salir o pulsar Save appearance. Los acentos antiguos fuera de esta familia se normalizan visualmente, sin impedir abrir perfiles anteriores.
- La ilustración es opcional y aparece solo en ventanas suficientemente anchas.
- Modules es la página inicial: tarjetas de Aim Assist, ESP y Velocity con switch, estado y acceso a ajustes. Las tarjetas activas usan el acento elegido.
- Filtros All/Favorites/Combat/Render/Movement y búsqueda por nombre. Los favoritos se conservan en appearance.json y también se eliminan con la limpieza local.
- Appearance incluye Delete local data and exit, con confirmación: borra los perfiles de nombre válido y appearance.json de esta app, conserva el EXE y cierra sin volver a guardar. No recorre subdirectorios ni borra archivos ajenos; rechaza carpetas enlazadas y elimina las carpetas propias solo si quedan vacías. No limpia registros de Windows, historial antivirus ni archivos fuera de su directorio de datos. No es una herramienta de eliminación de malware.
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

## Emerald edition: movimiento e inicio

La app consulta SPI_GETCLIENTAREAANIMATION de Windows y refresca la preferencia al recibir WM_SETTINGCHANGE. Reduce motion puede reducir adicionalmente el movimiento, pero no vuelve a activar lo que Windows ha desactivado. Transiciones con interpolación exponencial y ease-out: hover 140–150 ms, switches 210 ms, indicadores 230–240 ms, secciones 260 ms y entrada 420 ms. No hay animación continua de partículas.

El arranque muestra pasos completados de contexto gráfico, preparación de fuentes, carpeta de perfiles y preferencias. No hay pausas artificiales para mostrar el progreso: en equipos rápidos la pantalla puede verse solo brevemente, seguida de la transición de entrada.

Inter 4.1 se obtiene solo al compilar desde la publicación oficial de rsms/inter, verificando el SHA-256 del archivo. Las fuentes y su licencia SIL OFL se integran en los recursos de Windows; el EXE no descarga fuentes ni necesita instalarlas. La licencia se distribuye también como LICENSE-Inter.txt.

CI genera capturas rasterizadas del mismo ImGui para módulos, apariencia e inicio. Son escenas de prueba, no capturas de una sesión real de Windows ni mediciones de fluidez en el equipo del usuario.

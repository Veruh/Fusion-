# Fusion GUI — violet edition

Nueva interfaz ImGui inspirada en la referencia proporcionada: fondo oscuro, acento violeta, barra lateral, paneles con bordes suaves y controles visibles sin clic derecho.

## Qué incluye

- Aim Assist: todos los ajustes existentes, organizados por general, filtros, adaptación y feedback.
- Visuals: ajustes de ESP, colores, texto y amigos.
- Velocity: los controles existentes de Jump Reset.
- Profiles: seleccionar, cargar y guardar configuraciones.
- Settings: acento, movimiento con menú abierto, watermark, overlay y amigos.
- Panel ilustrativo de estilo y contadores de módulos/amigos. No presenta estadísticas inventadas ni objetivos en vivo.
- Vista compacta sin panel ilustrativo para ventanas de menos de 980 píxeles de ancho; contenido desplazable verticalmente.
- INSERT conserva su función de abrir/cerrar el menú. X solo cierra el menú. Detach requiere confirmación.

## Cambios de código

- `fusion-plus/src/base/menu/fusionGui.h`: nueva presentación compartida con el renderizador de prueba.
- `fusion-plus/src/base/menu/renderMenu.cpp`: conecta la GUI a los ajustes, perfiles y amigos existentes.
- `fusion-plus/src/base/configManager/settings.h`: acento violeta predeterminado y corrección del tamaño del array de modos de Velocity, de cero a uno.
- `fusion-plus/src/base/menu/hooks/wglSwapBuffers.cpp`: fuente de respaldo si no se pueden cargar las fuentes de Windows.

Los perfiles existentes mantienen sus valores, incluido su color de acento. Para usar violeta con un perfil anterior, cambia Accent color en Settings y vuelve a guardarlo.

## Compilación

Este ZIP contiene el código fuente completo basado en `Veruh/Fusion-`, commit `ef9d70980d7a34b6b0a156d87eec5d78ab4c6508`, y conserva la licencia GPLv3 y el workflow existente. Puede compilarse en Visual Studio 2022 o con ese workflow de GitHub Actions después de incorporar los cambios al repositorio.

Esta modificación de interfaz no corrige ni verifica la compatibilidad de la inyección con Lunar/JVM. La DLL anterior de Downloads no se modifica. El SHA-256 de cualquier DLL recompilada será diferente.

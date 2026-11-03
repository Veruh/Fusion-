# Estado de la revisión de módulos

Este trabajo está en curso. No es una versión lista para cargar en Minecraft.

## Alcance comprobado

La aplicación `standalone` es una vista previa independiente; sus controles no se comunican con el juego. La DLL tiene una instancia de la misma GUI y los controles usan las variables que consultan los módulos heredados. Esto no prueba que el cliente sea compatible con una versión concreta de Lunar.

## Correcciones en revisión

- La inicialización JNI interrumpe el arranque si no puede preparar el contexto; no intenta destruir la JVM del juego. Se conserva el classloader con una referencia global y se libera durante el cierre.
- Aim Assist usa la altura del jugador evaluado, revisa el estado actual del botón y evita divisiones por suavizado inválido y coordenadas no finitas.
- ESP evita la división por cero al calcular transparencia, inicializa correctamente los límites de pantalla y elimina datos obsoletos cuando no hay jugadores.
- Velocity usa un temporizador en lugar de contar iteraciones, limita la probabilidad y comprueba el foco antes de generar un salto. Intenta liberar una tecla generada al desactivar, perder foco o cerrar.
- Los módulos arrancan desactivados, incluso si la última configuración los dejó activados.
- Los perfiles antiguos ya no se cargan automáticamente al iniciar; la GUI conserva la opción de cargarlos manualmente. Una lista de amigos antigua inválida no debe interrumpir el arranque.
- No se compilan los archivos heredados de JavaHook/HotSpot: ninguna referencia en el código restante los usa. Los archivos siguen en el repositorio para revisión histórica.

## Trabajo imprescindible antes de publicar una DLL

1. Reemplazar o sincronizar el estado compartido entre el hilo de actualización, el render y el procedimiento de ventana. En particular, `CommonData`, los vectores de ESP, los ajustes y el estado de cierre se comparten sin un mecanismo completo de sincronización.
2. Revisar referencias y excepciones JNI, caché de clases/campos y limpieza de hooks bajo errores y cambios de ventana. La detección de versión heredada requiere validación para la versión exacta de Lunar.
3. Completar la reescritura y las pruebas de los módulos. Las correcciones anteriores se basan en código heredado; no constituyen una implementación nueva completa.
4. Probar en un entorno aislado, con cambios de foco, cierre del juego, entrada a menús, carga de perfiles y apagado con cada módulo activo. `SendInput` afecta a la entrada del sistema: comprobar que no quedan teclas pulsadas ni se envían eventos inesperados a otras ventanas.
5. Revisar código y dependencias y analizar el binario exacto. Un workflow exitoso solo demuestra que compiló y pasó sus pruebas configuradas; no certifica que esté libre de malware.

## Cambios experimentales de sincronización

La actualización de módulos, la construcción de la interfaz y el tratamiento de mensajes comparten un mutex recursivo. Las llamadas originales de Windows/OpenGL se hacen fuera de la sección protegida. Esto requiere pruebas reales de bloqueos y fluidez: no demuestra por sí solo que todos los accesos o ciclos de mensajes estén cubiertos.

Al cerrar el menú de la DLL se deshabilitan los hooks, pero la DLL y los trampolines de MinHook permanecen en memoria hasta que termine Minecraft. Esto busca evitar que callbacks pendientes regresen a código descargado. No permite prometer descarga completa ni recarga en la misma sesión, y necesita pruebas en el juego.

La versión experimental limita el arranque a los nombres de Lunar 1.8.9 y detiene la inicialización si alguno de los nombres del caché no se resuelve. Como el caché incluye elementos heredados que podrían no ser necesarios para los módulos, esta verificación conservadora puede rechazar incluso una instalación compatible. La prueba de CI verifica que existan los controles, pero no simula la JVM ni valida los nombres reales de Lunar.

Cada actualización crea un marco de referencias locales JNI y lo libera al terminar, después de vaciar la lista temporal de jugadores. Si detecta una excepción al terminar la lectura o entre módulos, desactiva el ciclo. Hay código heredado que realiza varias llamadas JNI antes de volver al ciclo, por lo que todavía se debe auditar la propagación de errores dentro de cada llamada.

La compilación de revisión no publica el DLL como artefacto. El EXE de vista previa no debe presentarse como si ya controlase el juego.


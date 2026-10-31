# Fusion+ recortado (versión mínima auditable)

Base: Fusion+ v0.0.4 (commit 0394a85). Esta copia deja solo lo necesario para estos módulos:

- Combate: Left Auto Clicker, Right Auto Clicker, Aim Assist
- Movimiento: Velocity, Sprint Reset
- Visual: ESP

## Qué se quitó

**Módulos:** Reach, Radar, Block ESP, Bridge Assist, Chest Stealer, Inventory Sorter, Array List,
Client Brand Changer, Block Reach, Tag Back, IT ESP (con sus ajustes y su guardado en configs).

**Parche de Java (Patcher):** el .jar embebido, miniz, el classloader desechable y el truco de
reemplazar `ThreadContext.EMPTY_MAP` de log4j. Solo lo usaba Reach.

**Injector:** TODO el código de red y de actualización (WinHTTP, el Pastebin, la descarga de la DLL,
la auto-actualización del injector, el broadcast de WM_CLOSE) y los botones de Discord/GitHub.
El injector ya no puede conectarse a internet. La DLL se copia a mano a
`Documentos\Fusion+\fusion-plus_v0.0.4.dll`.

**Hooks:** los de ClientBrandRetriever y getBlockReachDistance. Tampoco hay ya sistema de inventario.

## Qué NO cambió
El núcleo (arranque, enganche con la JVM, menú ImGui, configs y lista de amigos) y los 6 módulos
se dejaron tal cual. Los bugs conocidos de esos módulos siguen ahí (por ejemplo `target.height`
sin inicializar en Aim Assist).

## Compilar
Pestaña Actions -> build -> Run workflow. El log imprime el SHA256 de cada archivo.
Los archivos salen en `fusion-injector/build/` y `fusion-plus/build/`.

Licencia: GPLv3 (ver LICENSE). Proyecto original de x64x70 y colaboradores; Fusion+ de h1meji.

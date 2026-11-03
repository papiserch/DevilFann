# Devil Fan (plugin para FL Studio)

Multi-modulador con 5 modos: Chorus, Vibrato, Tremolo, Rotary y Fan (efecto de cantar frente a un ventilador).
Controles: Rate, Depth, Width (mid/side) y Mix.
El diablillo en la interfaz se mueve con el efecto: la cola y el cuerpo siguen el LFO, las llamas parpadean,
y en modo Fan canta frente a un ventilador cuyas aspas giran mas rapido con Rate.

## Como compilarlo (Windows)
Necesitas: Visual Studio 2022 (carga de trabajo "Desarrollo para el escritorio con C++"), CMake 3.22+ y Git.

    cmake -B build -G "Visual Studio 17 2022"
    cmake --build build --config Release

CMake descarga JUCE solo (necesita internet la primera vez).
El resultado queda en `build/DevilFan_artefacts/Release/VST3/Devil Fan.vst3` y se copia a
`C:\Program Files\Common Files\VST3` (abre la terminal como administrador para que pueda copiarlo).

## Usarlo en FL Studio
Options > Manage plugins > Find installed plugins (Start scan). Aparece como "Devil Fan" en el Plugin database.

## Compilar con GitHub (sin instalar nada)
1. Crea un repositorio nuevo en GitHub y sube todo el contenido de esta carpeta (incluida `.github`).
2. Ve a la pestana **Actions**. Si no arranca solo, elige "Build Devil Fan" y pulsa **Run workflow**.
3. Cuando termine (unos 10 minutos), abre la ejecucion y descarga **DevilFan-VST3-Windows** en la seccion Artifacts.
4. Descomprime y copia la carpeta `Devil Fan.vst3` a `C:\Program Files\Common Files\VST3`, luego haz el scan en FL Studio.

# Návod pro kompilaci

## Závislosti pro kompilaci
* C++
* OpenCv > 2
  * --with-ffmpeg support
  * --with-qt support
* Qt5 framework
* CMake > 3

## Kompilace
Nejdříve je potřeba nainstalovat všechny závislosti.

### Unixové distribuce
Preferována instalace pomocí [brew](http://brew.sh/) ([linuxbrew](https://github.com/Linuxbrew/linuxbrew)).

`brew install opencv -v --with-ffmpeg --with-qt --c++11`

`brew install qt5`

`brew linkapps qt5`

`brew install moc` - auto mock needed for qt

---

Aplikace se kompiluje pomocí CMake. Navigujte do složky aplikace v terminálu. Složka musí mít povolený zápis, můžeme povolit pomocí příkazu:

`chmod 777 ./`

Potom proveďte následující příkazy:

`cmake .`

`make`

`./bpm`

Pro vyšší výkon je možno spustit aplikaci s vyšší prioritou, hlavně při použití webkamery.

`$ nice -n -10 ./bpm`

To je vše!

### Windows
Aplikace je vyvíjena na systému OS X, ale vzhledem k použitým nástrojům by se měla dát stejnou cestou zkompilovat i na systému Windows.


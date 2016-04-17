# Synopsis

## Teoretický uvod & vymezení pojmů
* Eulerian magnification
  * Image pyramid
    * OpenCV: PyrDown pyramid
    * source: <http://docs.opencv.org/2.4/doc/tutorials/imgproc/pyramids/pyramids.html>
  * rgb <-> ntsc
    * převod do jiného barevného schématu, kde se více projevují změny?
  * Temporal Spatial
    * zobrazení na testovacích datech - originální + amplified verzi v určité frekvenci
  * source: <http://people.csail.mit.edu/mrub/vidmag/>
* Detekce tepu
  * Pomocí barevných změn - fluktuace krve
    * Green channel (oxo hemoglobin) s nejvýraznější fluktuací
    * source: <https://www.osapublishing.org/oe/abstract.cfm?uri=oe-16-26-21434>
    * Zmínit ještě přístup, kdy se snímá jenom prst -> 100% plochy je kůže -> nevhodná pro náš případ, kdy chceme následně tep i zobrazovat pomocí magnification
  * Zmínit jiné možnosti určení tepu
    * Head motions (source: <http://people.csail.mit.edu/balakg/pulsefromheadmotion.html>)
* Fourierova transformace
  * stěžejní pro hledání nejsilnější frekvence, která má odpovídat tepu

## Požadavky na aplikaci
  * požadavky na aplikace
  * nice to have požadavky
  * zde by mělo být napsáno jak by měla appka vypadat, ale ne jak toho dosáhneme
  * popsat 3 různé módy, které budeme implementovat
    - static video source mode (zpracování celého video)
    - real video source mode (zpracování videa jako z webkamery)
    - webcamera source mode

## Zvolené technologie dle požadavků
* požadavky na skoro real time zpracování proto přechod z matlabu do c++ & opencv
  * napsat rozdíl ve zpracování videa v matlabu a c++, případně změřit
* Viola Jones: detekce obličeje nejstabilnější řešení
  * porovnání s ostatními postupy
  * <http://www.vision.caltech.edu/html-files/EE148-2005-Spring/pprs/viola04ijcv.pdf>
* Forehead detection
  * Cascade eyes detection same as face?
* Opencv & c++
* Qt, ffmpeg?
  * ffmpeg library pro rychlejší vytažení framů z videa než defaultní opencv stream
* Boost threading library
  * rozepsat se o performance
  * esenciální pro efektnivní, časově přijatelné řešení
* verzovací systém git
  * standarní workflow: master, develop, pull requesty pro přidávání nových features
* generování grafů + obraz. příloh v aplikaci matlab
* zmínit psaní v JetBrains CLion ?

## Návrh aplikace
* minimalni pocet buffer frames - podlozit daty
* minimalni pocet frames pro vizualizaci - podlozit prilohou
* Hlavni rozhrani
* Detekce - lock pozice
* Middleware thread

### Výpočet tepu - hledání nejsilnější frekvence
* Zde by to mělo být podrobně vysvětlené, je to stěžejní část práce - se špatně určeným tepem zvýrazněná fluktuace nefunguje příliš dobře
* Hledání peaků grafů, které odpovídají tepům srdce
* Fourier vs ostatní pristupy (zkousel prokládání polyval, apod..)
* Zobrazeni temporal spatial ve které vlastně hledáme
* Zobrazeni fourierovy domény + popisy
* Popis nalezení nejsilnější frekvence a odpovídajícího tepu - viz. funkce `freqToBpmMapper()`
* Popsat cut-off frekvence, které považujeme za nekorektní (50 - 180 bpm - range of interest)

### Demonstrace na konkrétním postupu
* hodně doprovázet obrazovou přílohou a podrobně vysvětlit chování částí aplikace
* společné chování pro camera mode a video real mode x rozdílné pro static mode

### Grafy rozdílných (čím dál více pokročilých) přístupů
* V této části postupovat od naivního iniciálního měření (bez detekovaného obličej) až po konečné řešení s detekovaným čelem a potlačení globálních světelných změn - podložit reálnými daty
  * intenzity celého obrazu
  * intenzity s detekovanym oblicejem
  * intenzity v zeleném kanalu
  * intenzity v zeleném kanalu s detekovaným čelem
  * průměrné intenzity vs medián
  * potlačení globálních světelných změn - jako finální měření

### Měření dat a porovnávání se Ground truth
* generovat tabulky a porovnávat s Ground truth
* výpočet relativních a absolutních chyb, korelace?
* možnost generování statistickým dat a obrazů asi zachovat i v aplikaci?
* zde bychom měli zhodnotit, zda je algoritmus použitelný

### Zhodnocení
* zmínit požadavky na vstupní videa, bez velkých globálních světelných změn

-----
## Obrazová příloha
* Zde jsou nápady co by bylo zajímavé v bakalářce zobrazit
* Nápady se budou prolínat napříč celou prací

### Detektory
* Obličej
* Oči
* Čelo - ukázat i default při neúspěchu
* Postranní strips - ground truth background

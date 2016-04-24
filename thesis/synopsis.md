# Synopsis

## Shrnutí
* shrnutí práce

## Úvod

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
  * Zmínit jiné experimentální možnosti určení tepu
    * Head motions (source: <http://people.csail.mit.edu/balakg/pulsefromheadmotion.html>)
* Fourierova transformace
  * stěžejní pro hledání nejsilnější frekvence, která má odpovídat tepu

## Požadavky a cíle aplikace
  * požadavky na aplikaci
  * zde by mělo být napsáno jak by měla aplikace vypadat, ale ne jak toho dosáhneme
  * popsat 3 různé módy, které budeme implementovat
    - static video source mode (zpracování celého video)
    - real video source mode (zpracování videa jako z webkamery)
    - webcamera source mode

## Zvolené technologie dle požadavků
* Zde bych rád popsal technologie, které v aplikaci využívám.
* Požadavky na skoro real time zpracování proto přechod z matlabu do c++ & opencv
  * Napsat rozdíl ve zpracování videa v matlabu a c++, případně změřit.
* Opencv & c++
* Viola Jones: detekce obličeje nejstabilnější řešení
  * porovnání s ostatními postupy
  * <http://www.vision.caltech.edu/html-files/EE148-2005-Spring/pprs/viola04ijcv.pdf>
* Forehead detection
  * Cascade eyes detection same as face?
* Qt, ffmpeg?
  * ffmpeg library pro rychlejší vytažení framů z videa než defaultní opencv stream
* Boost threading library
  * rozepsat se o performance
  * esenciální pro efektnivní, časově přijatelné řešení
  * doložit daty - může být jen stručně - jednovláknové x vícevláknové
* Verzovací systém git
  * standarní workflow: master, develop, pull requesty pro přidávání nových features
* Generování grafů + obraz. příloh v aplikaci Matlab.
* Zmínit psaní v ide JetBrains CLion?

## Návrh aplikace
* Videa pro demonstraci, že postup funguje manuálně čelo, ruka apod.

* Hrubý popis jak by měla aplikace fungovat (pohled uživatele)
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
* Popsat chybu měření v závislosti na počtu zpracovávaných snímku (např. při 100 snímcích rozdíl mezi dvěma index je až 10bpm - při 400 už podstatně zredukováno)
* minimalni pocet buffer frames - podlozit daty

### Demonstrace na konkrétním postupu
* hodně doprovázet obrazovou přílohou a podrobně vysvětlit chování částí aplikace
* společné chování pro camera mode a video real mode x rozdílné pro static mode
* diskutovat minimální počet snímků pro vizualizaci - podložit přílohou

### Grafy rozdílných (čím dál více pokročilých) přístupů
* V této části postupovat od naivního iniciálního měření (bez detekovaného obličej) až po konečné řešení s detekovaným čelem a potlačení globálních světelných změn - podložit reálnými daty
  * intenzity celého obrazu
  * intenzity s detekovanym oblicejem
  * intenzity v zeleném kanalu
  * intenzity v zeleném kanalu s detekovaným čelem
  * průměrné intenzity vs medián
  * potlačení globálních světelných změn - jako finální měření
  * jestli budu video registrovat - toto by šlo implementovat alespoň do statistického módu

### Měření dat a porovnávání s Ground truth (tlakoměr)
* generovat tabulky a porovnávat s gt
* výpočet relativních a absolutních chyb, korelace?
* možnost generování statistickým dat a obrazů asi zachovat i v aplikaci (teď jen jako developer fáze)
* minimalni pocet buffer frames - podlozit daty
  * zmínit konvergenci k lepším výsledkům při delších video sekvencích (ty ale nejsou využitelné s webkamerou - nesmí se hýbat a trvá hodně dlouho)
  * zmínit jaká je nejdelší doba, kterou je někdo schopen čekat před webkamerou ?
* minimalni pocet frames pro vizualizaci - podlozit prilohou
  * toto se příliš nedá měčřit -> musíme nechat na subjektivním pocitu

### Zhodnocení
* zmínit požadavky na vstupní videa
  * bez velkých globálních světelných změn
  * SNR signal to noise ratio
  * fps - zmínit nyquist fraquency
  * resolution - dokázat s daty
  * musí být detekován obličej
  * dostupné čelo bez vlasů
  * hraniční body, kdy je algoritmus ještě korektní
    * hlavně velikost videa ~ obličeje
    * delka videa
* zde bychom měli zhodnotit
  * zda je algoritmus použitelný
  * za jakých podmínek funguje dobře
* Už teď mám spoustu nápadů, které by šly implementovat a tím chod aplikace zlepšít, ale nejsou úplně triviální
  * do budoucna bude co dělat :-)
---
## Obrazová příloha
* Zde jsou nápady co by bylo zajímavé v bakalářce zobrazit
* Nápady se budou prolínat napříč celou prací

### Detektory
* Obličej
* Oči
* Čelo - ukázat i default při neúspěchu
* Postranní strips - ground truth background

---
## Notes - not part of synopsis
### General reminders
* While describing how many frames are necessary for correct compute of bpm we cannot say just the more frames we have, the more precise results we got - probably because of some long terms freq changes present in video
* discuss ffmpeg in opencv which significantly reduce performing grabbing frames operations from source video
* provide explanation why is setting of fps in camera mode so weird (unable to hard set fps to )

### Data generating
#### Options for
* Real bpm vs computed bpm difference
* Output amplified video
* Intensities data
  * global intensities
  * detected forehead intensities
  * channels differences (as we use only green channel we should provide data that it's really best )
  * show mean vs average differences - maybe decide according to data analyze for better Options
  * ** Graphs ** - should be illustrated via graphs
  * before & after bandpass filtering

### Others
* Measure one super perfect video with good camera - e. g. only forehead and try to extract really good graph of intensities from it
* Bandpassing eliminates phantoms in video such as repiration movements
* Takto ocitovat
  * H. Wu et al. Eulerian video magnification for revealing subtle
changes in the world. ACM Trans. Graph. (Proceedings
SIGGRAPH 2012), 31(4), 2012.
---
## TODO:
### Must do
* Dynamically compute fps of camera
```
  grabbedFrameTimes = [];
  foreach inputStream as inputFrame
    grabbedFrameTimes.push(currentTime);
  end
  avgDiff = computeAvgDiffrence();
  fps = mapToFps(avgDiff);
```
### Nice to have
* Better Qt gui
* Global light changes supression
  * Study resources

# mnKarp felhasználói útmutató

[English](USER_GUIDE.md) · **Magyar**

Az mnKarp egy rack két MIDI-effektnek: betölti a telepített **mnKordot** (egy billentyű egy akkordot szólaltat meg), alatta pedig az
**mnArpee-t** (a hangokat mintában játssza le). A MIDI előbb az mnKordon, aztán az mnArpee-n megy át, és mindkettő kikapcsolható.

```
MIDI be -> [ mnKord ] -> [ mnArpee ] -> MIDI ki
              be/ki         be/ki
```

A két ablak maga az mnKord és az mnArpee ablaka, ezért az útmutatóik (mintarács, presetböngésző, akkordszimbólumok, skálák,
Undo / Redo) itt is érvényesek. Ez az útmutató azt írja le, amit a rack hozzátesz.

## Telepítés

Futtasd az `mnKarp-<verzió>-win-x64-setup.exe` fájlt. A VST3 a `Common Files\VST3\MNAudio\mnKarp.vst3` mappába kerül, amelyet minden
DAW átnéz, ez az útmutató pedig a `Program Files\MNAudio\mnKarp` mappába. Zárd be előtte a DAW-ot, ha be van töltve az mnKarp.
**Az mnKordnak és az mnArpee-nak is telepítve kell lennie** (az mnKordból legalább az 1.7.0, az egy billentyűzet miatt): az mnKarp a
VST3-mappából tölti be őket (`Common Files\VST3\MNAudio`, vagy bárhonnan a `Common Files\VST3` alól). Ha valamelyik hiányzik, a
szekciója ezt írja ki, és a MIDI átmegy rajta; a telepítő is figyelmeztet. Az mnKord vagy az mnArpee frissítése a rackot is frissíti.

## Használat DAW-ban

Az mnKarpot **hangszersávra** tedd, mint az mnKordot vagy az mnArpee-t. A sáv MIDI-jét kapja, és a MIDI-jét adja tovább a következő
eszköznek, egy szintinek vagy samplernek. Hangot nem ad.

## Az ablak

![Az mnKarp ablaka](images/main.png)

Fentről lefelé:

- **A felső sáv.** A cím és a verzió, jobbra a **Drag MIDI** és a méret gomb.
- **Az mnKord szekció.** Egy sáv a kapcsolóval, a névvel és az összecsukó nyíllal, alatta az mnKord ablaka (a **TRIGGER** sávot és a
  billentyűzetet az mnKord útmutatója írja le).
- **A hangsor.** Azok a hangok, amelyeket az mnKord átad az mnArpee-nak, élőben: billentyű lenyomásakor itt látod az akkordot. Gyorsan
  megmutatja, mit csinál az első fokozat.
- **Az mnArpee szekció.** A sávja és az mnArpee ablaka.

### Fokozat kikapcsolása

A sáv bal szélén lévő kerek **kapcsolóval** ki- és bekapcsolhatod a fokozatot. A kikapcsolt fokozat nem csinál semmit: a MIDI
érintetlenül átmegy rajta. A sávban az "OFF - the MIDI passes through" felirat látszik, az ablak szerkeszthető marad.

| mnKord | mnArpee | Amit kapsz |
|--------|---------|------------|
| be | be | a billentyű akkordot szólaltat meg, az akkordot pedig mintában játssza |
| be | ki | a billentyű akkordot szólaltat meg |
| ki | be | a lenyomott billentyűket mintában játssza |
| ki | ki | a MIDI változatlanul átmegy |

A kapcsolók host-paraméterek (`mnKord On`, `mnArpee On`), ezért automatizálhatók, vagy kontrollerre köthetők. Amikor egy fokozatot
kikapcsolsz, felengedi, amit éppen szólaltat (az akkordot, az arpeggiót), így nem ragad be hang.

### Ablak összecsukása

A sáv jobb szélén lévő nyíl elrejti a fokozat ablakát (és újra megmutatja). A hangzást nem érinti. Hasznos, hogy a rack alacsonyabb
legyen, amíg a másik fokozaton dolgozol. A projekt megjegyzi, mi van összecsukva.

### Méret és szélesség

A felső sáv **méret gombja** (kezdetben 75%) a két ablak méretét együtt állítja, 60% és 115% között. 75%-on a rack kb. 950 képpont
magas; ha nem fér el a képernyőn, válassz kisebbet.

A két ablak **mindig egyforma széles**. Az mnArpee ablaka átméretezhető a jobb alsó sarkával, és az mnKord ablaka követi, meg
fordítva (az mnKord ablaka 100%-on 848 és 1600 képpont közötti szélességet vehet fel). A méretet és a szélességet a projekt megjegyzi.

## Presetek

A racknek nincsenek saját presetjei. A presetek az **mnKord és az mnArpee sajátjai**: az ablakaikban választod, mented és böngészed
őket, ugyanúgy, mint az önálló pluginokban (ugyanazok a könyvtárak). Az mnKarppal mentett projekt mindkét fokozat teljes állapotát és
a két kapcsolót megtartja.

## Hol vannak a fájlok

| Mi | Hol |
|----|-----|
| A plugin | `C:\Program Files\Common Files\VST3\MNAudio\mnKarp.vst3` |
| mnKord presetek | `Documents\MNAudio\mnKord` (ugyanaz, mint az mnKord sajátja) |
| mnArpee presetek | `Documents\MNAudio\mnArpee` (ugyanaz, mint az mnArpee sajátja) |

## Drag MIDI: a végeredmény klipként

Az mnKarp megjegyzi, amit az utolsó 10 percben kiküldött. A felső sáv **Drag MIDI** gombja az **utolsó részt** adja át a DAW-nak: húzd a
gombot egy sávra (vagy az arrangerbe), és a hangok MIDI-klipként odakerülnek. Kattintásra `.mid` fájlba menti.

- Az utolsó rész a legutóbbi játék: az első hanggal kezdődik, amely előtt legalább 2 másodpercig semmi sem szólt (vagy ahol a DAW
  loopja visszaugrott), és az utolsó hangig tart.
- Ha a DAW transzportja fut, a klip az első hang ütemének elején kezdődik, így arra az ütemre ejtve a helyén lesz. Transzport nélkül
  az első hanggal kezdődik.
- A fájlban benne van a tempó is; negyedenként 960 tick.
- Amíg nincs mit áthúzni, a gomb szürke.

## A DAW-ban: automatizálás

**Paraméterek.** A host a rack következő paramétereit látja, ezeket automatizálhatod, felveheted, és kontrollerre is kötheted:

| Paraméter | Mi ez |
|-----------|-------|
| `mnKord On`, `mnArpee On` | a két kapcsoló |
| `mnArpee Rate`, `Octave`, `Gate %`, `Swing %`, `Steps` | az mnArpee fejlécének értékei |
| `mnArpee Latch`, `Group octaves`, `Cut on release` | az mnArpee három kapcsolója |

Ezek az mnArpee ablakának saját vezérlői: ha az ablakban megmozdítasz egyet, a host felveszi, ha a hostban automatizálsz egyet, a
vezérlő követi. Az mnKordnak nincs saját paramétere.

A fokozatok sorrendje fix: előbb az mnKord, aztán az mnArpee.

## Kérdések

**Nem szól semmi, ha megnyomok egy billentyűt.** Nézd meg, hogy a sáv hangszersáv-e, és hogy az mnKarp utáni szinti kap-e tőle MIDI-t.
Ellenőrizd a két kapcsolót, és hogy az mnKordban van-e kiosztás a billentyűre (lásd az útmutatóját).

**A rack magasabb a képernyőmnél.** Válassz kisebb méretet a méret gombbal, vagy csukd össze a nem használt ablakot.

**Lenyomott billentyű mellett kikapcsoltam egy fokozatot, és most a billentyű néma.** A lenyomott billentyűt felengedte a fokozattal
együtt. Nyomd meg újra.

**Kiváltja az mnKordot és az mnArpee-t?** Nem: az mnKarp őket tölti be, ezért telepítve kell lenniük. Ha csak akkordkiosztót vagy
csak arpeggiátort szeretnél, használd őket külön.

## Licenc

Az mnKarp szabad szoftver az AGPL-3.0 alatt, mint az mnKord és az mnArpee. Vibecoder: Németh Miklós.

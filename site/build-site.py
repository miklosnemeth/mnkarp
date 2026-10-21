#!/usr/bin/env python3
"""Builds the web pages in docs/ (GitHub Pages serves that folder):

    docs/index.html      English home page
    docs/hu/index.html   Hungarian home page
    docs/sitemap.xml     both pages, with hreflang alternates

The look is mnKord's: site/site.css is a copy of mnKord's stylesheet, so the MNAudio pages look the same. The version comes from CMakeLists.txt. The FAQ is written once (FAQ below) and feeds both the visible
questions and the FAQPage structured data.

    python site/build-site.py

Python 3 only, no packages.
"""
import datetime
import html
import json
import os
import re
import struct

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DOCS = os.path.join(ROOT, "docs")
KORD_CSS = os.path.join(ROOT, "site", "site.css")

SITE = "https://miklosnemeth.github.io/mnkarp/"
REPO = "https://github.com/miklosnemeth/mnkarp"
KORD_SITE = "https://miklosnemeth.github.io/mnkord/"
ARPEE_REPO = "https://github.com/miklosnemeth/mnArpee"
AUTHOR = "Miklós Németh"
AUTHOR_HU = "Németh Miklós"


def project_version():
    with open(os.path.join(ROOT, "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r"project\(\s*mnKarp\s+VERSION\s+([0-9.]+)", f.read())
    return m.group(1) if m else ""


def png_size(path):
    with open(path, "rb") as f:
        head = f.read(24)
    return struct.unpack(">II", head[16:24])


T = {
    "en": {
        "locale": "en_US", "dir": "", "base": "",
        "title": "mnKarp – free chord + arpeggiator rack plugin (VST3) for Windows",
        "description": "mnKarp is a free MIDI rack plugin (VST3) for Windows: mnKord turns one key into a chord, mnArpee "
                       "plays it as a pattern, each with an on/off switch. Rack presets, automation, drag the result into your DAW.",
        "og_alt": "The mnKarp window: mnKord above mnArpee",
        "skip": "Skip to the content",
        "nav": [("#features", "Features"), ("#faq", "FAQ"), ("#get", "Get it")],
        "other_lang": ("HU", "Magyar", "hu/"), "this_lang": "EN",
        "eyebrow": "Free MIDI rack plugin · VST3",
        "h1_hidden": " – chords and arpeggios in one rack (VST3)",
        "tagline": "Chords and arpeggios in one rack.",
        "lede": "mnKarp puts mnKord (one key, a whole chord) above mnArpee (a step-pattern arpeggiator) in one plugin. "
                "Play a single note and hear the chord as a pattern; switch either one off and the MIDI passes it untouched.",
        "cta_download": "Download for Windows", "cta_guide": "User guide (PDF)", "cta_source": "Source on GitHub",
        "pdf": "mnKarp-User-Guide.pdf",
        "facts": ["v{version}", "VST3 · Windows x64", "Free and open source (AGPL-3.0)"],
        "main_alt": "The mnKarp window: the rack presets on top, mnKord's keyboards, the notes between, mnArpee's pattern grid",
        "how_h": "How it works",
        "how_sub": "mnKarp is a MIDI effect: it makes no sound itself, it sits in front of your synth.",
        "steps": [("Play one key", "Your MIDI keyboard sends a single note."),
                  ("mnKord makes it a chord", "The key is looked up in mnKord's preset and becomes a chord. You see the notes in the line under it."),
                  ("mnArpee plays it as a pattern", "The chord goes on to mnArpee, which plays it in time with your DAW, step by step.")],
        "feat_h": "Two plugins, one rack",
        "feat_sub": "mnKord and mnArpee, with what a rack adds.",
        "cards": [
            ("On/off for each", "A power switch on each stage. Off, the MIDI passes it untouched, and nothing it played is left hanging."),
            ("Rack presets", "Both plugins' presets and the two switches under one name. They are also the DAW's program list."),
            ("Automation", "The switches and mnArpee's rate, gate, swing, octave, steps, latch, group and cut are host parameters."),
            ("Drag MIDI", "Drag the last thing you played into the DAW as a MIDI clip, on the bar it started on."),
            ("One keyboard for chords", "Pick the trigger key in the lane above the keys, click the chord notes on the same keyboard. Colours tell them apart."),
            ("Fits the screen", "One size for both windows (60 to 115%), equally wide, and fold a stage away when you don't need it."),
        ],
        "list_h": "Everything in the box",
        "list_sub": "The full feature list of mnKarp {version}.",
        "feature_groups": [
            ("The rack", [
                "mnKord (chords) above mnArpee (arpeggiator): the MIDI goes through mnKord, then mnArpee.",
                "A power switch for each stage; a stage that is off is dimmed and still editable.",
                "Switching a stage off releases the notes it is sounding.",
                "Fold a stage's window away; the project remembers it.",
                "One size for both windows, from 60% to 115%; both are always equally wide.",
                "The notes between the two stages, live.",
            ]),
            ("Presets and the DAW", [
                "Rack presets: both plugins' presets and the switches under one name, shared by every project.",
                "Previous / next, Save, Save as, Revert, Delete; a dot shows unsaved changes.",
                "The DAW's program list is the list of the rack presets (128 slots, oldest first).",
                "Host parameters: the two switches and mnArpee's eight values, for automation and MIDI controllers.",
                "Drag MIDI: the last take as a MIDI clip (with its tempo, starting on a bar), or save it as a file.",
            ]),
            ("Inside", [
                "mnKord and mnArpee: their presets, libraries, browsers and Undo / Redo work as in the single plugins.",
                "mnKord with one keyboard: the trigger keys in a lane above it, the chord notes on the keys, told apart by colour.",
                "The notes mnKord hands to mnArpee, shown live between the two.",
                "mnKord's and mnArpee's preset libraries are shared with the single plugins.",
                "The project keeps the full state of both stages.",
                "Windows installer; user guide in English and Hungarian.",
            ]),
        ],
        "req_h": "Requirements",
        "reqs": ["Windows 10 or 11, 64-bit", "A DAW that hosts VST3 plugins (Bitwig, FL Studio, Reaper, Cubase, Ableton Live ...)",
                 "A synth or sampler after mnKarp on the same track"],
        "faq_h": "Questions",
        "get_h": "Get mnKarp",
        "get_sub": "Free, no account, no copy protection.",
        "get_win": "Windows",
        "get_steps": ["Download <strong>mnKarp-{version}-win-x64-setup.exe</strong> from the latest release.",
                      "Run it. The plugin goes to the standard VST3 folder.",
                      "In your DAW, put mnKarp on an instrument track, before a synth."],
        "get_btn": "Download the installer",
        "docs_h": "Documentation and source",
        "links": [("{base}mnKarp-User-Guide.pdf", "User guide (PDF)"), ("{repo}", "Source code on GitHub"),
                  ("{repo}/blob/master/CHANGELOG.md", "What changed"), (KORD_SITE, "mnKord, the chord plugin alone"),
                  (ARPEE_REPO, "mnArpee, the arpeggiator alone")],
        "mac": "Only for Windows for now.",
        "by": "Vibecoded by", "name": AUTHOR, "name_html": "Miklós <span>Németh</span>",
        "about": "mnKarp, mnKord and mnArpee are made in Hungary, as free software.",
        "built": "Built with JUCE",
    },
    "hu": {
        "locale": "hu_HU", "dir": "hu/", "base": "../",
        "title": "mnKarp – ingyenes akkord + arpeggiátor rack plugin (VST3) Windowsra",
        "description": "Az mnKarp ingyenes MIDI rack plugin (VST3) Windowsra: az mnKord egy billentyűből akkordot csinál, az mnArpee "
                       "mintában játssza, mindkettő kikapcsolható. Rack presetek, automatizálás, a végeredmény áthúzható a DAW-ba.",
        "og_alt": "Az mnKarp ablaka: felül az mnKord, alatta az mnArpee",
        "skip": "Ugrás a tartalomra",
        "nav": [("#features", "Funkciók"), ("#faq", "Kérdések"), ("#get", "Letöltés")],
        "other_lang": ("EN", "English", "../"), "this_lang": "HU",
        "eyebrow": "Ingyenes MIDI rack plugin · VST3",
        "h1_hidden": " – akkordok és arpeggiók egy rackben (VST3)",
        "tagline": "Akkordok és arpeggiók egy rackben.",
        "lede": "Az mnKarp egy pluginban egymás fölé teszi az mnKordot (egy billentyű, egy egész akkord) és az mnArpee-t (lépésmintás "
                "arpeggiátor). Egy hangot játszol, és mintában hallod az akkordot; bármelyiket kikapcsolhatod, és a MIDI érintetlenül átmegy rajta.",
        "cta_download": "Letöltés Windowsra", "cta_guide": "Útmutató (PDF)", "cta_source": "Forráskód a GitHubon",
        "pdf": "mnKarp-Felhasznaloi-utmutato.pdf",
        "facts": ["v{version}", "VST3 · Windows x64", "Ingyenes, nyílt forrású (AGPL-3.0)"],
        "main_alt": "Az mnKarp ablaka: felül a rack presetek, az mnKord billentyűi, a köztes hangok, az mnArpee mintarácsa",
        "how_h": "Hogyan működik",
        "how_sub": "Az mnKarp MIDI-effekt: maga nem szól, a szintid elé kerül.",
        "steps": [("Lenyomsz egy billentyűt", "A MIDI-billentyűzet egyetlen hangot küld."),
                  ("Az mnKord akkordot csinál belőle", "A billentyűhöz az mnKord presetje akkordot rendel. A hangokat az alatta lévő sorban látod."),
                  ("Az mnArpee mintában játssza", "Az akkord az mnArpee-hoz kerül, amely a DAW tempójában, lépésről lépésre játssza.")],
        "feat_h": "Két plugin, egy rack",
        "feat_sub": "Az mnKord és az mnArpee, és amit a rack hozzátesz.",
        "cards": [
            ("Be/ki mindkettőre", "Mindkét fokozaton kapcsoló. Kikapcsolva a MIDI érintetlenül átmegy, és nem ragad be hang."),
            ("Rack presetek", "A két plugin presetje és a két kapcsoló egy név alatt. Ez a DAW programlistája is."),
            ("Automatizálás", "A kapcsolók és az mnArpee rate, gate, swing, octave, steps, latch, group és cut értéke host-paraméter."),
            ("MIDI áthúzás", "Az utoljára játszott részt MIDI-klipként behúzhatod a DAW-ba, az ütem elejére igazítva."),
            ("Egy billentyűzet az akkordokhoz", "A triggert a billentyűk fölötti sávban választod, az akkordhangokat ugyanazon a billentyűzeten kattintod. A színek különválasztják őket."),
            ("Elfér a képernyőn", "Egy méret mindkét ablakra (60–115%), egyforma szélességgel, és a nem használt fokozat összecsukható."),
        ],
        "list_h": "Minden funkció",
        "list_sub": "Az mnKarp {version} teljes funkciólistája.",
        "feature_groups": [
            ("A rack", [
                "Felül az mnKord (akkordok), alatta az mnArpee (arpeggiátor): a MIDI előbb az mnKordon, aztán az mnArpee-n megy át.",
                "Kapcsoló mindkét fokozatra; a kikapcsolt fokozat halvány, de szerkeszthető.",
                "Kikapcsoláskor a fokozat felengedi a hangjait.",
                "A fokozat ablaka összecsukható; a projekt megjegyzi.",
                "Egy méret mindkét ablakra, 60 és 115% között; a kettő mindig egyforma széles.",
                "A két fokozat közötti hangok, élőben.",
            ]),
            ("Presetek és a DAW", [
                "Rack presetek: a két plugin presetje és a kapcsolók egy név alatt, minden projekt közösen használja.",
                "Előző / következő, Save, Save as, Revert, Delete; egy pötty jelzi a nem mentett változást.",
                "A DAW programlistája a rack presetek listája (128 hely, a legrégebbitől kezdve).",
                "Host-paraméterek: a két kapcsoló és az mnArpee nyolc értéke, automatizáláshoz és kontrollerhez.",
                "MIDI áthúzás: az utolsó rész MIDI-klipként (tempóval, ütem elejéről), vagy fájlba mentve.",
            ]),
            ("Belül", [
                "Az mnKord és az mnArpee: a presetjeik, könyvtáraik, böngészőik és az Undo / Redo ugyanúgy működik, mint külön.",
                "Az mnKord egy billentyűzettel: fölötte a trigger-sáv, a billentyűkön az akkordhangok, színekkel megkülönböztetve.",
                "Az mnKord által az mnArpee-nak átadott hangok élőben, a kettő között.",
                "Az mnKord és az mnArpee presetkönyvtára közös az önálló pluginokéval.",
                "A projekt mindkét fokozat teljes állapotát megtartja.",
                "Windows telepítő; útmutató angolul és magyarul.",
            ]),
        ],
        "req_h": "Rendszerkövetelmények",
        "reqs": ["Windows 10 vagy 11, 64 bites", "VST3 pluginokat kezelő DAW (Bitwig, FL Studio, Reaper, Cubase, Ableton Live ...)",
                 "Egy szinti vagy sampler az mnKarp után, ugyanazon a sávon"],
        "faq_h": "Kérdések",
        "get_h": "Az mnKarp letöltése",
        "get_sub": "Ingyenes, regisztráció és másolásvédelem nélkül.",
        "get_win": "Windows",
        "get_steps": ["Töltsd le az <strong>mnKarp-{version}-win-x64-setup.exe</strong> fájlt a legutóbbi kiadásból.",
                      "Futtasd. A plugin a szabványos VST3 mappába kerül.",
                      "A DAW-ban tedd az mnKarpot egy hangszersávra, a szinti elé."],
        "get_btn": "A telepítő letöltése",
        "docs_h": "Dokumentáció és forráskód",
        "links": [("{base}mnKarp-Felhasznaloi-utmutato.pdf", "Felhasználói útmutató (PDF)"), ("{repo}", "Forráskód a GitHubon"),
                  ("{repo}/blob/master/CHANGELOG.md", "Változások"), (KORD_SITE + "hu/", "mnKord, az akkord plugin külön"),
                  (ARPEE_REPO, "mnArpee, az arpeggiátor külön")],
        "mac": "Egyelőre csak Windowsra.",
        "by": "Vibecoder", "name": AUTHOR_HU, "name_html": "Németh <span>Miklós</span>",
        "about": "Az mnKarp, az mnKord és az mnArpee Magyarországon készül, szabad szoftverként.",
        "built": "JUCE-szal készült",
    },
}

FAQ = {
    "en": [
        ("Is mnKarp free?", "Yes. It is free and open source under the AGPL-3.0, with no account and no copy protection."),
        ("Do I need mnKord and mnArpee too?", "No. mnKarp has both inside. If you have them, the three work side by side and share the "
                                              "preset libraries of mnKord and mnArpee."),
        ("Why is there no sound?", "mnKarp is a MIDI effect: put a synth or a sampler after it on the same instrument track."),
        ("Can I automate the arpeggiator?", "Yes. mnArpee's rate, octave, gate, swing, steps, latch, group and cut, and the two on/off "
                                           "switches, are host parameters."),
        ("How do I get the result into my DAW as MIDI?", "Play, then drag the <strong>Drag MIDI</strong> button onto a track. The clip "
                                                        "starts on the bar of its first note and carries the tempo. A click saves it as a file instead."),
        ("Does it run on macOS or Linux?", "Not yet: mnKarp is built for Windows (64-bit VST3)."),
    ],
    "hu": [
        ("Ingyenes az mnKarp?", "Igen. Ingyenes és nyílt forrású (AGPL-3.0), regisztráció és másolásvédelem nélkül."),
        ("Kell hozzá az mnKord és az mnArpee is?", "Nem. Az mnKarp mindkettőt tartalmazza. Ha megvannak, a három egymás mellett "
                                                   "működik, és közösen használják az mnKord és az mnArpee presetkönyvtárát."),
        ("Miért nem szól?", "Az mnKarp MIDI-effekt: tegyél utána egy szintit vagy samplert ugyanarra a hangszersávra."),
        ("Automatizálható az arpeggiátor?", "Igen. Az mnArpee rate, octave, gate, swing, steps, latch, group és cut értéke és a két "
                                            "kapcsoló host-paraméter."),
        ("Hogyan kerül a végeredmény MIDI-ként a DAW-ba?", "Játssz, aztán húzd a <strong>Drag MIDI</strong> gombot egy sávra. A klip az "
                                                           "első hang ütemének elején kezdődik, és a tempót is viszi. Kattintásra fájlba menti."),
        ("Fut macOS-en vagy Linuxon?", "Még nem: az mnKarp Windowsra készül (64 bites VST3)."),
    ],
}

ICONS = [
    '<circle cx="12" cy="13" r="7"/><path d="M12 3v7"/>',
    '<path d="M4 7h16M4 12h10M4 17h6"/>',
    '<path d="M3 18c3 0 3-12 6-12s3 12 6 12 3-8 6-8"/>',
    '<path d="M12 3v12m0 0-5-5m5 5 5-5M4 20h16"/>',
    '<rect x="3" y="5" width="18" height="14" rx="2"/><path d="M8 5v8M12 5v8M16 5v8"/>',
    '<rect x="4" y="4" width="16" height="7" rx="1.5"/><rect x="4" y="13" width="16" height="7" rx="1.5"/>',
]


def esc(s):
    return html.escape(s, quote=True)


def plain(s):
    return html.unescape(re.sub(r"<[^>]+>", "", s))


def logo_svg():
    return ('<svg viewBox="0 0 32 32" aria-hidden="true"><rect width="32" height="32" rx="7" fill="#13211a"/>'
            '<rect x="5" y="6" width="22" height="8" rx="2" fill="#3ddc84"/><rect x="5" y="18" width="22" height="8" rx="2" fill="#fbfcfb"/></svg>')


FAVICON = ("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'%3E%3Crect width='32' height='32' rx='7' "
           "fill='%230b1510'/%3E%3Crect x='5' y='6' width='22' height='8' rx='2' fill='%233ddc84'/%3E%3Crect x='5' y='18' width='22' "
           "height='8' rx='2' fill='%23fbfcfb'/%3E%3C/svg%3E")

GITHUB_ICON = ('<svg viewBox="0 0 24 24" fill="currentColor" aria-hidden="true"><path d="M12 .5a11.5 11.5 0 0 0-3.64 22.41c.58.1.79-.25.79-.56v-2'
               'c-3.2.7-3.88-1.37-3.88-1.37-.52-1.33-1.28-1.69-1.28-1.69-1.04-.71.08-.7.08-.7 1.15.08 1.76 1.19 1.76 1.19 1.03 1.76 2.69 1.25 '
               '3.35.96.1-.75.4-1.25.73-1.54-2.55-.29-5.24-1.28-5.24-5.69 0-1.26.45-2.28 1.19-3.09-.12-.29-.52-1.46.11-3.05 0 0 .97-.31 3.17 '
               '1.18a11 11 0 0 1 5.78 0c2.2-1.49 3.17-1.18 3.17-1.18.63 1.59.23 2.76.11 3.05.74.81 1.19 1.83 1.19 3.09 0 4.42-2.69 5.39-5.25 '
               '5.68.41.36.78 1.06.78 2.14v3.17c0 .31.21.67.8.56A11.5 11.5 0 0 0 12 .5Z"/></svg>')


def build_page(lang, ctx):
    t = T[lang]
    base = t["base"]
    fmt = dict(version=ctx["version"], repo=REPO, base=base)

    def f(s):
        return s.format(**fmt)

    url = SITE + t["dir"]
    image = SITE + "images/main.png"
    iw, ih = ctx["image_size"]
    download = REPO + "/releases/latest"
    description = t["description"]
    faq = FAQ[lang]

    graph = [
        {"@type": "WebSite", "@id": SITE + "#website", "url": SITE, "name": "mnKarp", "inLanguage": ["en", "hu"],
         "publisher": {"@id": SITE + "#author"}},
        {"@type": "Person", "@id": SITE + "#author", "name": AUTHOR, "alternateName": AUTHOR_HU,
         "url": "https://github.com/miklosnemeth", "jobTitle": "Vibecoder"},
        {"@type": "SoftwareApplication", "@id": SITE + "#app", "name": "mnKarp", "url": url, "description": description,
         "inLanguage": lang, "applicationCategory": "MultimediaApplication",
         "applicationSubCategory": "MIDI effect, VST3 audio plugin", "operatingSystem": "Windows 10, Windows 11",
         "softwareVersion": ctx["version"], "downloadUrl": download, "installUrl": download,
         "fileFormat": "application/vnd.steinberg.vst3", "softwareRequirements": "A DAW that hosts VST3 plugins",
         "license": "https://www.gnu.org/licenses/agpl-3.0.html", "isAccessibleForFree": True,
         "offers": {"@type": "Offer", "price": "0", "priceCurrency": "USD"}, "author": {"@id": SITE + "#author"},
         "image": image, "screenshot": [image],
         "featureList": [x for _, items in t["feature_groups"] for x in items]},
        {"@type": "FAQPage", "@id": url + "#faq", "inLanguage": lang,
         "mainEntity": [{"@type": "Question", "name": q, "acceptedAnswer": {"@type": "Answer", "text": plain(a)}}
                        for q, a in faq]},
    ]
    jsonld = json.dumps({"@context": "https://schema.org", "@graph": graph}, ensure_ascii=False, indent=1).replace("</", "<\\/")

    o = []
    w = o.append
    w("<!doctype html>")
    w('<html lang="%s">' % lang)
    w("<head>")
    w('<meta charset="utf-8">')
    w('<meta name="viewport" content="width=device-width, initial-scale=1">')
    w("<title>%s</title>" % esc(t["title"]))
    w('<meta name="description" content="%s">' % esc(description))
    w('<meta name="robots" content="index, follow, max-image-preview:large">')
    w('<meta name="author" content="%s">' % esc(t["name"]))
    w('<link rel="canonical" href="%s">' % url)
    w('<link rel="alternate" hreflang="en" href="%s">' % SITE)
    w('<link rel="alternate" hreflang="hu" href="%s">' % (SITE + "hu/"))
    w('<link rel="alternate" hreflang="x-default" href="%s">' % SITE)
    for prop, value in (("og:type", "website"), ("og:site_name", "mnKarp"), ("og:title", t["title"]),
                        ("og:description", description), ("og:url", url), ("og:locale", t["locale"]),
                        ("og:locale:alternate", T["hu" if lang == "en" else "en"]["locale"]), ("og:image", image),
                        ("og:image:width", str(iw)), ("og:image:height", str(ih)), ("og:image:alt", t["og_alt"])):
        w('<meta property="%s" content="%s">' % (prop, esc(value)))
    w('<meta name="twitter:card" content="summary_large_image">')
    w('<meta name="twitter:title" content="%s">' % esc(t["title"]))
    w('<meta name="twitter:description" content="%s">' % esc(description))
    w('<meta name="twitter:image" content="%s">' % image)
    w('<meta name="color-scheme" content="dark">')
    w('<meta name="theme-color" content="#0b1510">')
    w('<link rel="icon" href="%s">' % FAVICON)
    w('<link rel="preload" href="%sassets/fonts/Montserrat.ttf" as="font" type="font/ttf" crossorigin>' % base)
    w("<style>")
    w(ctx["css"].replace("{{BASE}}", base))
    w("</style>")
    w('<script type="application/ld+json">')
    w(jsonld)
    w("</script>")
    w("</head>")
    w("<body>")
    w('<a class="skip" href="#main">%s</a>' % esc(t["skip"]))

    w('<div class="wrap">')
    w('  <header class="top">')
    w('    <a class="brand display" href="%s" aria-label="mnKarp">%smnKarp</a>' % (base or "./", logo_svg()))
    w('    <nav class="nav" aria-label="%s">' % ("Main" if lang == "en" else "Főmenü"))
    for href, label in t["nav"]:
        w('      <a class="hide-sm" href="%s">%s</a>' % (href, esc(label)))
    code_, name_, path_ = t["other_lang"]
    w('      <span class="lang" role="group" aria-label="Language / Nyelv"><span class="cur" aria-current="true">%s</span>'
      '<a href="%s" hreflang="%s" lang="%s" title="%s">%s</a></span>'
      % (t["this_lang"], path_, code_.lower(), code_.lower(), esc(name_), code_))
    w("    </nav>")
    w("  </header>")

    w('  <main id="main">')
    w('    <div class="hero">')
    w('      <p class="eyebrow">%s</p>' % esc(t["eyebrow"]))
    w('      <h1 class="display">mn<span>Karp</span><span class="sr-only">%s</span></h1>' % esc(t["h1_hidden"]))
    w('      <p class="tagline">%s</p>' % esc(t["tagline"]))
    w('      <p class="lede">%s</p>' % esc(t["lede"]))
    w('      <div class="cta">')
    w('        <a class="btn primary" href="%s"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" '
      'stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M12 3v12m0 0-5-5m5 5 5-5M4 20h16"/></svg>%s</a>'
      % (download, esc(t["cta_download"])))
    w('        <a class="btn" href="%s%s"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" '
      'stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M6 3h8l4 4v14H6z"/><path d="M14 3v4h4M9 13h6M9 17h6"/></svg>%s</a>'
      % (base, t["pdf"], esc(t["cta_guide"])))
    w('        <a class="btn" href="%s">%s%s</a>' % (REPO, GITHUB_ICON, esc(t["cta_source"])))
    w("      </div>")
    w('      <ul class="facts">')
    for fact in t["facts"]:
        w("        <li>%s</li>" % esc(f(fact)))
    w("      </ul>")
    w('      <div class="shot"><img src="%simages/main.png" width="%d" height="%d" alt="%s"></div>' % (base, iw, ih, esc(t["main_alt"])))
    w("    </div>")

    w('    <section aria-labelledby="how">')
    w('      <h2 id="how" class="display">%s</h2>' % esc(t["how_h"]))
    w('      <p class="sub">%s</p>' % esc(t["how_sub"]))
    w('      <ol class="steps">')
    for h, p in t["steps"]:
        w('        <li class="step"><h3>%s</h3><p>%s</p></li>' % (esc(h), esc(p)))
    w("      </ol>")
    w("    </section>")

    w('    <section id="features" aria-labelledby="feat">')
    w('      <h2 id="feat" class="display">%s</h2>' % esc(t["feat_h"]))
    w('      <p class="sub">%s</p>' % esc(t["feat_sub"]))
    w('      <ul class="cards">')
    for i, (h, p) in enumerate(t["cards"]):
        w('        <li class="card"><div class="ic"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" '
          'stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">%s</svg></div><h3>%s</h3><p>%s</p></li>'
          % (ICONS[i], esc(h), esc(p)))
    w("      </ul>")
    w('      <h3 class="listh">%s</h3>' % esc(t["list_h"]))
    w('      <p class="sub">%s</p>' % esc(f(t["list_sub"])))
    w('      <div class="featlists">')
    for gh, items in t["feature_groups"]:
        w('        <div class="fl"><h4>%s</h4><ul>' % esc(gh))
        for item in items:
            w("          <li>%s</li>" % esc(item))
        w("        </ul></div>")
    w("      </div>")
    w("    </section>")

    w('    <section id="requirements" aria-labelledby="req">')
    w('      <h2 id="req" class="display">%s</h2>' % esc(t["req_h"]))
    w('      <ul class="plain">')
    for r in t["reqs"]:
        w("        <li>%s</li>" % esc(r))
    w("      </ul>")
    w("    </section>")

    w('    <section id="faq" aria-labelledby="faqh">')
    w('      <h2 id="faqh" class="display">%s</h2>' % esc(t["faq_h"]))
    w('      <div class="faq">')
    for q, a in faq:
        w("        <details><summary>%s</summary><p>%s</p></details>" % (esc(q), a))
    w("      </div>")
    w("    </section>")

    w('    <section id="get" aria-labelledby="getit">')
    w('      <h2 id="getit" class="display">%s</h2>' % esc(t["get_h"]))
    w('      <p class="sub">%s</p>' % esc(t["get_sub"]))
    w('      <div class="get">')
    w('        <div class="box"><h3>%s</h3><ol>' % esc(t["get_win"]))
    for s in t["get_steps"]:
        w("          <li>%s</li>" % f(s))
    w('        </ol><a class="btn primary" href="%s">%s</a></div>' % (download, esc(t["get_btn"])))
    w('        <div class="box"><h3>%s</h3><ul>' % esc(t["docs_h"]))
    for href, label in t["links"]:
        w('          <li><a href="%s">%s</a></li>' % (esc(f(href)), esc(label)))
    w("        </ul><p>%s</p></div>" % esc(t["mac"]))
    w("      </div>")
    w("    </section>")
    w("  </main>")
    w("</div>")

    w('<footer class="author">')
    w('  <div class="wrap">')
    w('    <div class="tag"><svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" '
      'stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="m8 8-5 4 5 4M16 8l5 4-5 4M14 5l-4 14"/></svg>%s</div>'
      % esc(t["by"]))
    w('    <p class="name display">%s</p>' % t["name_html"])
    w("    <p>%s</p>" % esc(t["about"]))
    w('    <p class="legal">© %d %s · <a href="%s/blob/master/LICENSE">AGPL-3.0</a> · %s</p>'
      % (ctx["year"], esc(t["name"]), REPO, t["built"]))
    w("  </div>")
    w("</footer>")
    w("</body>")
    w("</html>")
    return "\n".join(o) + "\n"


def sitemap(lastmod):
    def entry(url, other, lang_self, lang_other):
        return ("  <url>\n    <loc>%s</loc>\n    <lastmod>%s</lastmod>\n    <changefreq>monthly</changefreq>\n"
                "    <priority>%s</priority>\n"
                '    <xhtml:link rel="alternate" hreflang="%s" href="%s"/>\n    <xhtml:link rel="alternate" hreflang="%s" href="%s"/>\n'
                '    <xhtml:link rel="alternate" hreflang="x-default" href="%s"/>\n  </url>\n'
                % (url, lastmod, "1.0" if url == SITE else "0.9", lang_self, url, lang_other, other, SITE))
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9" xmlns:xhtml="http://www.w3.org/1999/xhtml">\n'
            + entry(SITE, SITE + "hu/", "en", "hu") + entry(SITE + "hu/", SITE, "hu", "en") + "</urlset>\n")


def main():
    with open(KORD_CSS, encoding="utf-8") as f:
        css = f.read()

    ctx = dict(version=project_version(), css=css, image_size=png_size(os.path.join(DOCS, "images", "main.png")),
               year=datetime.date.today().year)

    os.makedirs(os.path.join(DOCS, "hu"), exist_ok=True)
    for lang, rel in (("en", "index.html"), ("hu", os.path.join("hu", "index.html"))):
        with open(os.path.join(DOCS, rel), "w", encoding="utf-8", newline="\n") as f:
            f.write(build_page(lang, ctx))
        print("wrote docs/" + rel.replace("\\", "/"))

    with open(os.path.join(DOCS, "sitemap.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write(sitemap(datetime.date.today().isoformat()))
    print("wrote docs/sitemap.xml  (version %s)" % ctx["version"])


if __name__ == "__main__":
    main()

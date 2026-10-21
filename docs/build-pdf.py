#!/usr/bin/env python3
"""Builds the PDF versions of the mnKarp user guides (docs/*.pdf) from the Markdown files.

It uses pdf_converter.py (mnKord's converter: a small Markdown converter, and Microsoft Edge or Chrome
printing the HTML to PDF), with mnKarp's own guides, cover and version.

    python docs/build-pdf.py
"""
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
KORD_SCRIPT = os.path.join(HERE, "pdf_converter.py")

COVER = {
    "en": {"title": "User guide", "tagline": "Chords and arpeggios in one rack.", "version": "Version",
           "by": "Vibecoded by", "author": "Miklós Németh",
           "license": "Free software under the AGPL-3.0"},
    "hu": {"title": "Felhasználói útmutató", "tagline": "Akkordok és arpeggiók egy rackben.", "version": "Verzió",
           "by": "Vibecoder", "author": "Németh Miklós",
           "license": "Szabad szoftver az AGPL-3.0 alatt"},
}


def project_version():
    with open(os.path.join(HERE, "..", "CMakeLists.txt"), encoding="utf-8") as f:
        m = re.search(r"project\(\s*mnKarp\s+VERSION\s+([0-9.]+)", f.read())
    return m.group(1) if m else ""


def cover(lang):
    c = COVER[lang]
    return """
<section class="cover">
  <svg class="logo" viewBox="0 0 32 32" aria-hidden="true"><rect width="32" height="32" rx="7" fill="#13211a"/><rect x="5" y="6" width="22" height="8" rx="2" fill="#3ddc84"/><rect x="5" y="18" width="22" height="8" rx="2" fill="#fbfcfb"/></svg>
  <div class="brand">mn<span>Karp</span></div>
  <div class="doc">%(title)s</div>
  <div class="tagline">%(tagline)s</div>
  <div class="meta" style="margin-top: 70mm">
    <div>%(version)s %(ver)s</div>
    <div class="by">%(by)s <b>%(author)s</b></div>
    <div class="small">%(license)s</div>
  </div>
</section>""" % dict(c, ver=project_version())


def main():
    spec = importlib.util.spec_from_file_location("mnkord_build_pdf", KORD_SCRIPT)
    kord = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(kord)

    kord.DOCS = HERE
    kord.GUIDES = [
        ("USER_GUIDE.md", "mnKarp-User-Guide.pdf", "en"),
        ("USER_GUIDE.hu.md", "mnKarp-Felhasznaloi-utmutato.pdf", "hu"),
    ]
    kord.cover = cover
    kord.project_version = project_version
    kord.main()


if __name__ == "__main__":
    if not os.path.exists(KORD_SCRIPT):
        sys.exit("docs/pdf_converter.py is missing.")
    main()

import sys
import xml.etree.ElementTree as ET
import json
import jaconv
import requests
from bs4 import BeautifulSoup
import pykakasi

kakasi = pykakasi.kakasi()
sys.stdout.reconfigure(encoding='utf-8')

# Conjugation keys (verbs + adjectives)
CONJUGATION_MAP = [
    ("Present Positive Informal", "PLAIN"),
    ("Present Positive Formal", "POLITE"),
    ("Present Negative Informal", "NEGATIVE"),
    ("Present Negative Formal", "POLITE_NEGATIVE"),
    ("Past Positive Informal", "PAST"),
    ("Past Positive Formal", "POLITE_PAST"),
    ("Past Negative Informal", "PAST_NEGATIVE"),
    ("Past Negative Formal", "POLITE_PAST_NEGATIVE"),
    ("Te Form Positive Informal", "TE_FORM"),
    ("Potential Positive Informal", "POTENTIAL"),
    ("Passive Positive Informal", "PASSIVE"),
    ("Causative Positive Informal", "CAUSATIVE"),
    ("Conditional Positive Informal", "CONDITIONAL"),
    ("Volitional Positive Informal", "VOLITIONAL"),
    ("Imperative Positive Informal", "IMPERATIVE"),
]

def to_hiragana(text):
    """Converts any input (kanji, katakana, hiragana, mixed) to hiragana."""
    result = kakasi.convert(text)
    return ''.join([chunk['hira'] for chunk in result])

def to_kana_if_romaji(word):
    # If ASCII, treat as romaji and convert to hiragana via jaconv
    if all(ord(ch) < 128 for ch in word):
        kana = jaconv.alphabet2kana(word.lower())
        hira = jaconv.kata2hira(kana)
        return hira
    return word

def to_romaji_if_kana(word):
    # For Reverso URL: always send romaji
    if any('\u3040' <= ch <= '\u30ff' for ch in word):
        return jaconv.kana2alphabet(word)
    return word

def detect_adjective_type(word):
    if word == "いい":
        return "ii_special"
    elif word.endswith("かっこいい"):
        return "double_ii"
    elif word.endswith("い") and not word.endswith("いい"):
        return "i"
    elif word.endswith("な"):
        return "na"
    return None

def conjugate_ii_special():
    return {
        "PLAIN":      "いい",
        "POLITE":     "いいです",
        "NEGATIVE":   "よくない",
        "POLITE_NEGATIVE": "よくないです",
        "PAST":       "よかった",
        "POLITE_PAST":"よかったです",
        "PAST_NEGATIVE": "よくなかった",
        "POLITE_PAST_NEGATIVE": "よくなかったです",
        "TE_FORM":    "よくて",
    }

def conjugate_double_ii(word):
    prefix = word[:-4]
    base = prefix + "かっこ"
    return {
        "PLAIN":      base + "いい",
        "POLITE":     base + "いいです",
        "NEGATIVE":   base + "よくない",
        "POLITE_NEGATIVE": base + "よくないです",
        "PAST":       base + "よかった",
        "POLITE_PAST":base + "よかったです",
        "PAST_NEGATIVE": base + "よくなかった",
        "POLITE_PAST_NEGATIVE": base + "よくなかったです",
        "TE_FORM":    base + "よくて",
    }

def conjugate_i_adjective(word):
    stem = word[:-1]
    return {
        "PLAIN":      stem + "い",
        "POLITE":     stem + "いです",
        "NEGATIVE":   stem + "くない",
        "POLITE_NEGATIVE": stem + "くないです",
        "PAST":       stem + "かった",
        "POLITE_PAST":stem + "かったです",
        "PAST_NEGATIVE": stem + "くなかった",
        "POLITE_PAST_NEGATIVE": stem + "くなかったです",
        "TE_FORM":    stem + "くて",
    }

def conjugate_na_adjective(word):
    stem = word[:-1]
    return {
        "PLAIN":      stem + "だ",
        "POLITE":     stem + "です",
        "NEGATIVE":   stem + "ではない",
        "POLITE_NEGATIVE": stem + "ではありません",
        "PAST":       stem + "だった",
        "POLITE_PAST":stem + "でした",
        "PAST_NEGATIVE": stem + "ではなかった",
        "POLITE_PAST_NEGATIVE": stem + "ではありませんでした",
        "TE_FORM":    stem + "で",
    }

def is_romaji(word):
    return all(ord(c) < 128 for c in word)

def is_verb_candidate(word):
    # A basic heuristic for Japanese verbs
    romaji_endings = ("u", "ru", "ku", "gu", "su", "tsu", "nu", "bu", "mu")
    kana_endings = "うくぐすつぬむぶる"
    if is_romaji(word):
        return word.endswith(romaji_endings)
    else:
        return word and word[-1] in kana_endings

def fetch_verb_conjugations(word_romaji, debug=False):
    """
    Fetches verb conjugations from Reverso using the romaji form of the word.
    """
    url = f"https://conjugator.reverso.net/conjugation-japanese-verb-{word_romaji}.html"
    headers = {
        "User-Agent": (
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
        )
    }
    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()
        soup = BeautifulSoup(response.text, 'html.parser')

        if debug:
            with open("debug_reverso.html", "w", encoding="utf-8") as f:
                f.write(soup.prettify())
            print("Debug: Saved raw HTML response to debug_reverso.html", file=sys.stderr)

        conjugations = parse_conjugations(soup, debug)
        if conjugations:
            return conjugations
        else:
            print("Error: No conjugations found for the given verb:", word_romaji, file=sys.stderr)
            return None

    except requests.exceptions.RequestException as e:
        print(f"Error: Unable to fetch data for {word_romaji}\nDetails: {e}", file=sys.stderr)
        return None

def parse_conjugations(soup, debug=False):
    """
    Extracts verb conjugations from Reverso's HTML and outputs hiragana only.
    """
    conjugations = {}
    tense_blocks = soup.find_all('div', class_='blue-box-wrap')
    if not tense_blocks:
        return None

    for block in tense_blocks:
        tense = block.get("mobile-title", "").strip()
        if not tense:
            continue
        key = None
        for label, mapkey in CONJUGATION_MAP:
            if tense == label:
                key = mapkey
                break
        if not key:
            continue
        kana = block.find('span', class_='ruby')
        if kana:
            raw = kana.get_text(strip=True)
            hira = to_hiragana(raw)
            conjugations[key] = hira
        else:
            romaji = block.find('div', class_='romaji')
            if romaji:
                kana_str = jaconv.alphabet2kana(romaji.get_text(strip=True).lower())
                hira = to_hiragana(kana_str)
                conjugations[key] = hira
            else:
                conjugations[key] = "N/A"
    if debug and conjugations:
        print("Debug: Extracted verb conjugations:", file=sys.stderr)
        for ctype, forms in conjugations.items():
            print(f"{ctype}: {forms}", file=sys.stderr)

    return conjugations if conjugations else None

def output_xml(word, conj_dict, map_struct=CONJUGATION_MAP):
    root = ET.Element("conjugations")
    ET.SubElement(root, "word").text = word
    for label, key in map_struct:
        tense = ET.SubElement(root, "tense", name=key)
        form = conj_dict.get(key, "N/A")
        ET.SubElement(tense, "kana").text = form
    return ET.tostring(root, encoding="unicode")

def output_json(word, conj_dict):
    return json.dumps({"word": word, "conjugations": conj_dict}, ensure_ascii=False, indent=2)

def main():
    if len(sys.argv) < 2:
        print("Usage: Conjugation.py <word_in_kana_or_kanji_or_romaji> [--json] [--debug]")
        sys.exit(1)

    word = sys.argv[1].strip()
    word = to_kana_if_romaji(word)
    output_json_mode = '--json' in sys.argv
    debug_mode = '--debug' in sys.argv

    # Try adjective logic first
    adj_type = detect_adjective_type(word)
    if adj_type:
        if debug_mode:
            print(f"DEBUG: recognized as adjective type {adj_type}", file=sys.stderr)
        if adj_type == "ii_special":
            conj = conjugate_ii_special()
        elif adj_type == "double_ii":
            conj = conjugate_double_ii(word)
        elif adj_type == "i":
            conj = conjugate_i_adjective(word)
        elif adj_type == "na":
            conj = conjugate_na_adjective(word)
        else:
            print("ERROR: Unexpected adjective type", file=sys.stderr)
            sys.exit(1)
        # All outputs as hiragana
        hira_conj = {k: to_hiragana(v) for k, v in conj.items()}
        if output_json_mode:
            print(output_json(word, hira_conj))
        else:
            print(output_xml(word, hira_conj))
        sys.exit(0)
    elif is_verb_candidate(word):
        word_romaji = to_romaji_if_kana(word)
        if debug_mode:
            print(f"DEBUG: recognized as verb candidate, using '{word_romaji}' for Reverso", file=sys.stderr)
        conj = fetch_verb_conjugations(word_romaji, debug=debug_mode)
        if not conj:
            print("この単語は動詞として認識できません (Not recognized as a verb)")
            sys.exit(1)
        if output_json_mode:
            print(output_json(word, conj))
        else:
            print(output_xml(word, conj))
        sys.exit(0)
    else:
        print("この単語は形容詞や動詞として認識できません (Not recognized as adjective or verb)", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()

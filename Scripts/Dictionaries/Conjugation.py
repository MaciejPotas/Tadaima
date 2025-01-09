import requests
from bs4 import BeautifulSoup
import xml.etree.ElementTree as ET
import sys
import time
import jaconv  # for romaji -> kana conversion

# Ensure UTF-8 output
sys.stdout.reconfigure(encoding='utf-8')

CONJUGATION_MAP = {
    "Present Positive Informal": "PLAIN",
    "Present Positive Formal": "POLITE",
    "Present Negative Informal": "NEGATIVE",
    "Present Negative Formal": "POLITE_NEGATIVE",
    "Past Positive Informal": "PAST",
    "Past Positive Formal": "POLITE_PAST",
    "Past Negative Informal": "PAST_NEGATIVE",
    "Past Negative Formal": "POLITE_PAST_NEGATIVE", 
    "Te Form Positive Informal": "TE_FORM",
    "Potential Positive Informal": "POTENTIAL",
    "Passive Positive Informal": "PASSIVE",
    "Causative Positive Informal": "CAUSATIVE",
    "Conditional Positive Informal": "CONDITIONAL",
    "Volitional Positive Informal": "VOLITIONAL",
    "Imperative Positive Informal": "IMPERATIVE",
}

def get_conjugations(word, debug=False):
    word = word.strip().lower()

    # If it ends with 'u', treat as verb
    if word.endswith('u'):
        return fetch_verb_conjugations(word, debug)

    # If it's exactly "ii"
    if word == 'ii':
        return conjugate_special_ii(debug=debug)
    # Else if it ends with "ii" (but not just "ii"), e.g., "atamagaii", "kakkoii"
    elif len(word) > 2 and word.endswith('ii'):
        # Check if it's a special "ii"-type adjective or a regular i-adjective
        if word[-3:] == 'aii':  # Regular i-adjective ending like "atarashii"
            return conjugate_i_adjective(word, debug)
        else:
            return conjugate_double_ii_adjective(word, debug)
    # Else if ends with 'i', treat as normal i‐adjective
    elif word.endswith('i'):
        return conjugate_i_adjective(word, debug)
    # If ends with 'na', treat as na‐adjective
    elif word.endswith('na'):
        return conjugate_na_adjective(word, debug)
    else:
        print("Error: The word does not match known patterns (not ending in 'u', 'ii', 'i', or 'na').")
        return None

# ----------------- Verb Handling ----------------- #

def fetch_verb_conjugations(word, debug=False):
    """
    Fetches verb conjugations from Reverso if available.
    """
    url = f"https://conjugator.reverso.net/conjugation-japanese-verb-{word}.html"
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
            print("Debug: Saved raw HTML response to debug_reverso.html")

        conjugations = parse_conjugations(soup, debug)
        if conjugations:
            return conjugations
        else:
            print("Error: No conjugations found for the given verb:", word)
            return None

    except requests.exceptions.RequestException as e:
        print(f"Error: Unable to fetch data for {word}\nDetails: {e}")
        return None

def parse_conjugations(soup, debug=False):
    """
    Extracts the verb conjugations from Reverso's HTML.
    """
    conjugations = {}
    tense_blocks = soup.find_all('div', class_='blue-box-wrap')
    if not tense_blocks:
        return None

    for block in tense_blocks:
        tense = block.get("mobile-title", "").strip()
        if not tense or tense not in CONJUGATION_MAP:
            continue

        romaji = block.find('div', class_='romaji')
        kana = block.find('span', class_='ruby')

        if romaji and kana:
            conjugations[CONJUGATION_MAP[tense]] = {
                "kana": kana.get_text(strip=True),
                "romaji": romaji.get_text(strip=True),
            }

    if debug and conjugations:
        print("Debug: Extracted verb conjugations:")
        for ctype, forms in conjugations.items():
            print(f"{ctype}: Kana: {forms['kana']}, Romaji: {forms['romaji']}")

    return conjugations if conjugations else None

# ----------------- Adjective Handling ----------------- #

def conjugate_special_ii(debug=False):
    """
    Conjugations for the stand-alone adjective 'ii' (good).
    Negative => yokunai, Past => yokatta, etc.
    """
    def to_kana(r):
        katakana = jaconv.alphabet2kana(r)
        return jaconv.kata2hira(katakana)

    base_forms = {
        "PLAIN": "ii",
        "POLITE": "ii desu",
        "NEGATIVE": "yokunai",
        "POLITE_NEGATIVE": "yokunai desu",
        "PAST": "yokatta",
        "POLITE_PAST": "yokatta desu",
        "PAST_NEGATIVE": "yokunakatta",
        "POLITE_PAST_NEGATIVE": "yokunakatta desu",
        "TE_FORM": "yokute",
    }

    # Irrelevant forms for adjectives
    na_forms = ["POTENTIAL", "PASSIVE", "CAUSATIVE", "CONDITIONAL", "VOLITIONAL", "IMPERATIVE"]
    for f in na_forms:
        base_forms[f] = "N/A"

    # Convert to final dictionary
    conjugations = {}
    for ctype in CONJUGATION_MAP.values():
        rom = base_forms.get(ctype, "N/A")
        if rom == "N/A":
            kana = "N/A"
        else:
            kana = to_kana(rom)
        conjugations[ctype] = {"romaji": rom, "kana": kana}

    if debug:
        print("Debug: Generated 'ii' special adjective conjugations:")
        for ctype, form_data in conjugations.items():
            print(f"{ctype}: Romaji: {form_data['romaji']}, Kana: {form_data['kana']}")

    return conjugations

def conjugate_double_ii_adjective(word, debug=False):
    """
    For adjectives ending with 'ii' (but not just 'ii'), e.g. 'atamagaii', 'kakkoii'.
    They follow a similar pattern to 'ii' => negative uses 'yoku', etc.
    """
    # e.g., 'atamagaii' => prefix = 'atamagai'
    prefix = word[:-2]

    def to_kana(r):
        katakana = jaconv.alphabet2kana(r)
        return jaconv.kata2hira(katakana)

    # The logic is the same as for 'ii', but we prepend the prefix
    base_forms = {
        "PLAIN": prefix + "ii",
        "POLITE": prefix + "ii desu",
        "NEGATIVE": prefix + "yokunai",
        "POLITE_NEGATIVE": prefix + "yokunai desu",
        "PAST": prefix + "yokatta",
        "POLITE_PAST": prefix + "yokatta desu",
        "PAST_NEGATIVE": prefix + "yokunakatta",
        "POLITE_PAST_NEGATIVE": prefix + "yokunakatta desu",
        "TE_FORM": prefix + "yokute",
    }

    # Irrelevant forms for adjectives
    na_forms = ["POTENTIAL", "PASSIVE", "CAUSATIVE", "CONDITIONAL", "VOLITIONAL", "IMPERATIVE"]
    for f in na_forms:
        base_forms[f] = "N/A"

    # Build final conjugation dictionary
    conjugations = {}
    for ctype in CONJUGATION_MAP.values():
        rom = base_forms.get(ctype, "N/A")
        if rom == "N/A":
            kana = "N/A"
        else:
            kana = to_kana(rom)
        conjugations[ctype] = {"romaji": rom, "kana": kana}

    if debug:
        print("Debug: Generated double-'ii' adjective conjugations:")
        for ctype, data in conjugations.items():
            print(f"{ctype}: Romaji: {data['romaji']}, Kana: {data['kana']}")

    return conjugations

def conjugate_i_adjective(word, debug=False):
    """
    Standard i-adjectives that end with a single 'i' (e.g., 'kawaii', 'hayai').
    """
    stem = word[:-1]  # remove the last 'i'

    def to_kana(r):
        katakana = jaconv.alphabet2kana(r)
        return jaconv.kata2hira(katakana)

    base_forms = {
        "PLAIN": stem + "i",
        "POLITE": stem + "i desu",
        "NEGATIVE": stem + "kunai",
        "POLITE_NEGATIVE": stem + "kunai desu",
        "PAST": stem + "katta",
        "POLITE_PAST": stem + "katta desu",
        "PAST_NEGATIVE": stem + "kunakatta",
        "POLITE_PAST_NEGATIVE": stem + "kunakatta desu",
        "TE_FORM": stem + "kute",
    }

    # Set forms that don't apply to adjectives
    na_forms = ["POTENTIAL", "PASSIVE", "CAUSATIVE", "CONDITIONAL", "VOLITIONAL", "IMPERATIVE"]
    for f in na_forms:
        base_forms[f] = "N/A"

    # Build final dictionary
    conjugations = {}
    for ctype in CONJUGATION_MAP.values():
        rom = base_forms.get(ctype, "N/A")
        if rom == "N/A":
            kana = "N/A"
        else:
            kana = to_kana(rom)
        conjugations[ctype] = {"romaji": rom, "kana": kana}

    if debug:
        print("Debug: Generated standard i-adjective conjugations:")
        for ctype, form_data in conjugations.items():
            print(f"{ctype}: Romaji: {form_data['romaji']}, Kana: {form_data['kana']}")

    return conjugations

def conjugate_na_adjective(word, debug=False):
    """
    Conjugates na-adjectives (ending in 'na').
    """
    stem = word[:-2]  # remove 'na'

    def to_kana(r):
        katakana = jaconv.alphabet2kana(r)
        return jaconv.kata2hira(katakana)

    base_forms = {
        "PLAIN": stem + "da",
        "POLITE": stem + " desu",
        "NEGATIVE": stem + " janai",
        "POLITE_NEGATIVE": stem + " janai desu",
        "PAST": stem + "datta",
        "POLITE_PAST": stem + " deshita",
        "PAST_NEGATIVE": stem + " janakatta",
        "POLITE_PAST_NEGATIVE": stem + " janakatta desu",
        "TE_FORM": stem + "de",
    }

    # Set forms that don't apply to adjectives
    na_forms = ["POTENTIAL", "PASSIVE", "CAUSATIVE", "CONDITIONAL", "VOLITIONAL", "IMPERATIVE"]
    for f in na_forms:
        base_forms[f] = "N/A"

    # Build final dictionary
    conjugations = {}
    for ctype in CONJUGATION_MAP.values():
        rom = base_forms.get(ctype, "N/A")
        if rom == "N/A":
            kana = "N/A"
        else:
            kana = to_kana(rom)
        conjugations[ctype] = {"romaji": rom, "kana": kana}

    if debug:
        print("Debug: Generated na-adjective conjugations:")
        for ctype, form_data in conjugations.items():
            print(f"{ctype}: Romaji: {form_data['romaji']}, Kana: {form_data['kana']}")

    return conjugations

# ----------------- XML Output ----------------- #

def format_output(word, conjugations):
    root = ET.Element("conjugations")
    ET.SubElement(root, "word").text = word

    for tense, name in CONJUGATION_MAP.items():
        tense_element = ET.SubElement(root, "tense", name=name)
        forms = conjugations.get(name, {"kana": "N/A", "romaji": "N/A"})
        ET.SubElement(tense_element, "kana").text = forms.get("kana", "N/A")
        ET.SubElement(tense_element, "romaji").text = forms.get("romaji", "N/A")

    return ET.tostring(root, encoding='unicode')

# ----------------- Main Entry ----------------- #

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: script.py <word> [-debug]")
        sys.exit(1)

    word = sys.argv[1].strip()
    debug_mode = '-debug' in sys.argv

    conjugations = get_conjugations(word, debug=debug_mode)
    if conjugations:
        xml_output = format_output(word, conjugations)
        print(xml_output)

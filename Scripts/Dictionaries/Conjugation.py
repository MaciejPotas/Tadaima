import sys
import xml.etree.ElementTree as ET
import json
import jaconv

sys.stdout.reconfigure(encoding='utf-8')

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
    # Non-adjective forms omitted
]

def detect_adjective_type(word):
    """
    Returns: 'ii_special', 'double_ii', 'i', 'na', or None
    """
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
    # 'いい' is irregular: negative is よくない, past is よかった, etc.
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
    # Only works for words ending in 'かっこいい'
    prefix = word[:-4]  # Remove 'かっこいい'
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
    # Remove last 'い'
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
    # Remove last 'な'
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

def output_xml(word, conj_dict):
    root = ET.Element("conjugations")
    ET.SubElement(root, "word").text = word
    for label, key in CONJUGATION_MAP:
        tense = ET.SubElement(root, "tense", name=key)
        form = conj_dict.get(key, "N/A")
        ET.SubElement(tense, "kana").text = form
    return ET.tostring(root, encoding="unicode")

def output_json(word, conj_dict):
    return json.dumps({"word": word, "conjugations": conj_dict}, ensure_ascii=False, indent=2)

def to_kana_if_romaji(word):
    if all(ord(ch) < 128 for ch in word):
        kana = jaconv.alphabet2kana(word.lower())
        return jaconv.kata2hira(kana)
    return word
    
def main():
    if len(sys.argv) < 2:
        print("Usage: conj.py <word_in_kana_or_romaji> [--json]")
        sys.exit(1)

    word = sys.argv[1].strip()
    word = to_kana_if_romaji(word)
    #print("AFTER ROMAJI CONVERSION:", repr(word), file=sys.stderr)

    output_json_mode = '--json' in sys.argv

    adj_type = detect_adjective_type(word)
    if adj_type == "ii_special":
        conj = conjugate_ii_special()
    elif adj_type == "double_ii":
        conj = conjugate_double_ii(word)
    elif adj_type == "i":
        conj = conjugate_i_adjective(word)
    elif adj_type == "na":
        conj = conjugate_na_adjective(word)
    else:
        print("AFTER ROMAJI CONVERSION:", repr(word), file=sys.stderr)
        print("この単語は形容詞として認識できません (Not recognized as an adjective)")
        sys.exit(1)

    if output_json_mode:
        print(output_json(word, conj))
    else:
        print(output_xml(word, conj))


if __name__ == "__main__":
    main()

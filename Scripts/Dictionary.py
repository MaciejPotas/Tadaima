import sys
import xml.etree.ElementTree as ET
from googletrans import Translator
from pykakasi import kakasi

# Ensure UTF-8 output
sys.stdout.reconfigure(encoding='utf-8')

# Initialize Google Translator and Kakasi only once
translator = Translator()
kakasi_instance = kakasi()

def translate_english_to_japanese(english_word):
    # Translate trs to Japanese
    translation = translator.translate(english_word, src='pl', dest='ja')
    japanese_text = translation.text
    
    # Convert to hiragana and romaji using the new Kakasi API
    converter = kakasi_instance.convert(japanese_text)
    hiragana = ''.join([item['hira'] for item in converter])
    romaji = ' '.join([item['hepburn'] for item in converter])
    
    return japanese_text, hiragana, romaji

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python translate.py <english_word>")
        sys.exit(1)
    
    english_word = sys.argv[1]
    japanese_text, hiragana, romaji = translate_english_to_japanese(english_word)
    
    # Create XML structure
    root = ET.Element("translation")
    ET.SubElement(root, "trs").text = english_word
    ET.SubElement(root, "japanese").text = japanese_text
    ET.SubElement(root, "hiragana").text = hiragana
    ET.SubElement(root, "romaji").text = romaji

    # Output XML as UTF-8
    xml_str = ET.tostring(root, encoding='unicode')
    print(xml_str)

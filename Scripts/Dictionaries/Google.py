import requests
from bs4 import BeautifulSoup
import xml.etree.ElementTree as ET
import pykakasi
import sys

def get_translation(word, direction='jp'):
    # URL encode the word for the search query
    url = f"https://jisho.org/search/{word}"
    
    # Make a request to the website
    response = requests.get(url)
    if response.status_code != 200:
        print("Error: Could not fetch data from Jisho.org")
        return None
    
    # Parse the HTML content
    soup = BeautifulSoup(response.text, 'html.parser')

    if direction == 'jp':
        return parse_japanese_to_english(soup, word)
    elif direction == 'en':
        return parse_english_to_japanese(soup, word)
    else:
        print("Invalid direction. Use 'jp' for Japanese-to-English and 'en' for English-to-Japanese.")
        return None

def parse_japanese_to_english(soup, word):
    # Find the first result block
    first_result = soup.find('div', class_='concept_light clearfix')
    if not first_result:
        print("Error: No results found on Jisho.org")
        return None
    
    # Extract the kanji word
    kanji_element = first_result.find('span', class_='text')
    kanji = kanji_element.text.strip() if kanji_element else "N/A"
    
    # Extract the kana (reading) word
    kana_element = first_result.find('span', class_='furigana')
    kana = kana_element.text.strip() if kana_element else "N/A"
    
    # Extract the romaji word (not directly available, so we convert kana to romaji)
    romaji = kana_to_romaji(kana) if kana != "N/A" else "N/A"
    
    # Extract an English translation
    english_element = first_result.find('span', class_='meaning-meaning')
    english_translation = english_element.text.strip() if english_element else "N/A"
    
    return format_output(english_translation, kana, kanji, romaji)

def parse_english_to_japanese(soup, word):
    # Find the first result block
    first_result = soup.find('div', class_='concept_light clearfix')
    if not first_result:
        print("Error: No results found on Jisho.org")
        return None

    # Extract the Japanese text (kanji or kana)
    japanese_element = first_result.find('span', class_='text')
    japanese_text = japanese_element.text.strip() if japanese_element else "N/A"

    # Extract the kana (reading)
    kana_element = first_result.find('span', class_='furigana')
    kana = kana_element.text.strip() if kana_element else "N/A"

    # Convert kana to romaji
    romaji = kana_to_romaji(kana) if kana != "N/A" else "N/A"

    return format_output(word, kana, japanese_text, romaji)

def kana_to_romaji(kana):
    # Convert kana to romaji using pykakasi
    kakasi = pykakasi.kakasi()
    result = kakasi.convert(kana)
    
    # Ensure proper conversion
    if result:
        return ''.join([item['hepburn'] for item in result]).strip()
    return "N/A"

def format_output(english_word, hiragana, kanji, romaji):
    # Create XML structure
    root = ET.Element("translation")
    ET.SubElement(root, "trs").text = english_word
    ET.SubElement(root, "hiragana").text = hiragana
    ET.SubElement(root, "kanji").text = kanji
    ET.SubElement(root, "romaji").text = romaji
    ET.SubElement(root, "example").text = ""  # Placeholder for an example sentence

    # Output XML as UTF-8
    xml_str = ET.tostring(root, encoding='unicode')
    return xml_str

if __name__ == "__main__":
    # Check if sufficient arguments are provided
    if len(sys.argv) != 3:
        print("Usage: script.py <direction> <word>")
        print("Example: script.py en dog")
        sys.exit(1)
    
    # Get the direction and word from command line arguments
    direction = sys.argv[1].strip().lower()
    word = sys.argv[2].strip()

    # Call the translation function
    xml_output = get_translation(word, direction)
    if xml_output:
        print(xml_output)

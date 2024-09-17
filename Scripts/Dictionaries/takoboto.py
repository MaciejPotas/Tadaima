import requests
from bs4 import BeautifulSoup
import xml.etree.ElementTree as ET
import sys
import pykakasi
import re
from googletrans import Translator
import time

# Ensure UTF-8 output
sys.stdout.reconfigure(encoding='utf-8')

def get_translation(word, debug=False):
    # URL for the search query on Takoboto
    url = f"https://takoboto.jp/?q={word}"
    
    start_time = time.time()  # Start the timer
    
    # Make a request to the website
    response = requests.get(url)
    response.encoding = 'utf-8'  # Ensure response is interpreted as UTF-8
    if response.status_code != 200:
        print("Error: Could not fetch data from Takoboto")
        return None
    
    # Parse the HTML content
    soup = BeautifulSoup(response.text, 'html.parser')

    # Extract the first block of the result
    result = parse_takoboto_result(soup, debug)
    
    if debug:
        end_time = time.time()  # End the timer
        print(f"Debug: Script execution time: {end_time - start_time:.2f} seconds")
    
    return result

def parse_takoboto_result(soup, debug=False):
    # Find the first result block inside #SearchResultContent div
    result_block = soup.find('div', class_='ResultDiv')
    
    if not result_block:
        print("Error: No results found on Takoboto")
        return None

    # Extract Kana and Kanji
    kana, kanji = extract_kana_and_kanji(result_block, debug)
    
    # Extract the English translation
    english_translation = extract_translation(result_block, debug)
    
    # Convert Kana to Romaji
    romaji = convert_kana_to_romaji(kana)
    
    # Extract the example sentence and translate it to English
    example_sentence, example_translation = extract_and_translate_example(result_block, debug)
    
    # Construct the output in XML format
    return format_output(english_translation, kana, kanji, romaji, example_sentence, example_translation)

def extract_kana_and_kanji(result_block, debug=False):
    # Find the first div that contains Kana and Kanji information
    word_line = result_block.find('div', lang='ja')
    kana = "N/A"
    kanji = "N/A"
    
    if word_line:
        # The first span is Kana or Kanji, following spans may contain the other
        spans = word_line.find_all('span')
        if len(spans) > 0:
            # Determine if the first element is Kanji or Kana
            first_text = spans[0].get_text(strip=True)
            if contains_kanji(first_text):
                kanji = first_text
                # The following spans might contain Kana
                if len(spans) > 1:
                    kana = spans[1].get_text(strip=True)
            else:
                kana = first_text
                # The following spans might contain Kanji
                if len(spans) > 1:
                    # Iterate to find a span with Kanji
                    kanji_candidates = [span.get_text(strip=True) for span in spans[1:]]
                    kanji = next((extract_kanji(text) for text in kanji_candidates if extract_kanji(text)), "N/A")
    
    # Clean up Kanji and Kana by removing commas or leading/trailing spaces
    kanji = kanji.replace(',', '').strip()
    kana = kana.replace(',', '').strip()

    if debug:
        print(f"Debug: Extracted Kana: {kana}")
        print(f"Debug: Extracted Kanji: {kanji}")
    
    return kana, kanji

def contains_kanji(text):
    # Check if the text contains any Kanji characters
    return any('\u4e00' <= char <= '\u9faf' for char in text)

def extract_kanji(text):
    # Remove any characters that are not kanji (kanji ranges are \u4e00-\u9faf)
    kanji_only = ''.join(re.findall(r'[\u4e00-\u9faf]', text))
    return kanji_only

def extract_translation(result_block, debug=False):
    # Convert the result block to a BeautifulSoup object for easier parsing
    soup = BeautifulSoup(str(result_block), 'html.parser')
    
    # Look for the '1.' followed by the relevant translation word
    first_entry_divs = soup.find_all('div', style=re.compile(r'padding-left:2px;padding-bottom:2px'))
    
    # Iterate through the divs to find the right one
    for div in first_entry_divs:
        # Skip divs containing spans with color:#A0A0A0 (usually meta-information like "Usually in kana")
        if div.find('span', style="color:#A0A0A0"):
            if debug:
                print(f"Debug: Skipping div due to meta-information: {div.get_text(separator=' ', strip=True)}")
            continue

        # Extract the text within this div
        text = div.get_text(separator=' ', strip=True)

        # Check for the "1." style entry
        match = re.search(r'1\.\s*([a-zA-Z]+)', text)
        if match:
            translation = match.group(1)
            if debug:
                print(f"Debug: Extracted Translation (1. style): {translation}")
            return translation

        # If "1." style isn't found, look for any English word directly
        # Typically English translations won't be in a span with specific styles
        words = text.split()
        for word in words:
            if re.match(r'^[a-zA-Z]+$', word):
                if debug:
                    print(f"Debug: Extracted Translation (direct style): {word}")
                return word

    # Debug log: If no match or div found
    if debug:
        print("Debug: No match found for translation.")
    return "N/A"

def convert_kana_to_romaji(kana):
    # Convert kana to romaji using pykakasi
    kakasi = pykakasi.kakasi()
    result = kakasi.convert(kana)
    
    # Ensure proper conversion
    if result:
        # Join all the converted romaji parts together
        return ''.join([item['hepburn'] for item in result]).strip()
    return "N/A"

def extract_and_translate_example(result_block, debug=False):
    # Look for the example sentence div immediately following the translation divs
    translation_divs = result_block.find_all('div', style="padding-left:2px;padding-bottom:2px")
    example_sentence = "N/A"
    example_translation = "N/A"
    
    if translation_divs:
        # The example sentence is usually located directly after the main translation block
        for i in range(len(translation_divs) - 1):
            next_div = translation_divs[i + 1]
            # Look for the example in this div
            example_span = next_div.find('span', style="color:#606060")
            if example_span and "。" in example_span.get_text(strip=True):
                example_sentence = example_span.get_text(strip=True)
                # Translate the example sentence to English using Google Translate
                example_translation = translate_to_english(example_sentence)
                break
    
    if debug:
        print(f"Debug: Extracted Example Sentence: {example_sentence}")
        print(f"Debug: Translated Example: {example_translation}")
    
    return example_sentence, example_translation

def translate_to_english(japanese_text):
    try:
        # Use Google Translate to translate the Japanese text to English
        translator = Translator()
        translation = translator.translate(japanese_text, src='ja', dest='en')
        return translation.text
    except Exception as e:
        print(f"Error during translation: {e}")
        return "N/A"

def format_output(english_word, hiragana, kanji, romaji, example_sentence, example_translation):
    # Combine example sentence and its translation
    combined_example = f"{example_sentence} - {example_translation}" if example_sentence != "N/A" and example_translation != "N/A" else example_sentence

    # Create XML structure
    root = ET.Element("translation")
    ET.SubElement(root, "trs").text = english_word
    ET.SubElement(root, "hiragana").text = hiragana
    ET.SubElement(root, "kanji").text = kanji
    ET.SubElement(root, "romaji").text = romaji
    ET.SubElement(root, "example").text = combined_example

    # Output XML as UTF-8
    xml_str = ET.tostring(root, encoding='unicode')
    return xml_str.encode('utf-8').decode('utf-8')

if __name__ == "__main__":
    # Check if sufficient arguments are provided
    if len(sys.argv) < 2:
        print("Usage: script.py <word> [-debug]")
        sys.exit(1)
    
    # Get the word from command line arguments
    word = sys.argv[1].strip()
    
    # Check if debug mode is enabled
    debug_mode = '-debug' in sys.argv

    # Call the translation function
    xml_output = get_translation(word, debug=debug_mode)
    if xml_output:
        print(xml_output)

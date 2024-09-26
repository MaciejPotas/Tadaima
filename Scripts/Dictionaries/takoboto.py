import requests
from bs4 import BeautifulSoup
import xml.etree.ElementTree as ET
import sys
import pykakasi
import re
from googletrans import Translator
import time
import unicodedata

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
    # Find the div that contains Kana and Kanji information
    word_line = result_block.find('div', lang='ja')
    kana = "N/A"
    kanji = "N/A"

    if word_line:
        # Get all text from the div
        text = word_line.get_text(separator=' ', strip=True)
        # Split the text by commas and spaces
        variants = [variant.strip() for variant in re.split(r'[,\s]+', text)]
        # Separate kana and kanji
        kana_variants = [variant for variant in variants if is_kana(variant)]
        kanji_variants = [variant for variant in variants if contains_kanji(variant)]

        # Select the first Kana variant if exists
        kana = kana_variants[0] if kana_variants else "N/A"
        # Select the first Kanji variant if exists
        kanji = kanji_variants[0] if kanji_variants else "N/A"

    if debug:
        print(f"Debug: Extracted Kana: {kana}")
        print(f"Debug: Extracted Kanji: {kanji}")

    return kana, kanji

def is_kana(text):
    # Remove any spaces or punctuation
    text = re.sub(r'[\s,]', '', text)
    # Check if the text consists only of kana characters (Hiragana or Katakana)
    return all('HIRAGANA' in unicodedata.name(char) or 'KATAKANA' in unicodedata.name(char) for char in text if char != '')

def contains_kanji(text):
    # Check if the text contains any Kanji characters
    return any('\u4e00' <= char <= '\u9faf' for char in text)

def extract_translation(result_block, debug=False):
    from bs4 import NavigableString, Tag  # Import inside function to avoid scope issues
    # Convert the result block to a BeautifulSoup object for easier parsing
    soup = BeautifulSoup(str(result_block), 'html.parser')

    # Look for all divs containing translations
    first_entry_divs = soup.find_all('div', style=re.compile(r'padding-left:2px;padding-bottom:2px'))

    for div in first_entry_divs:
        # Skip divs that contain any spans with color:#A0A0A0 (meta-information like "Adverb")
        meta_span = div.find('span', style="color:#A0A0A0")
        if meta_span:
            # Skip this div if it's meta-information (unless it starts with a number)
            if not re.match(r'^\d\.', div.get_text(strip=True)):
                if debug:
                    print(f"Debug: Skipping div due to meta-information: {meta_span.get_text(strip=True)}")
                continue

        # Extract text from the div, excluding any meta-information spans
        texts = []
        for content in div.contents:
            if isinstance(content, NavigableString):
                text = content.strip()
                if text:
                    texts.append(text)
            elif isinstance(content, Tag):
                span_style = content.get('style', '')
                if 'color:#A0A0A0' not in span_style:
                    text = content.get_text(strip=True)
                    if text:
                        texts.append(text)

        # Combine the texts to form the translation
        translation = ' '.join(texts).strip()

        # Now we trim the translation to the first non-letter-digit character, except spaces
        trimmed_translation = re.match(r'^[a-zA-Z0-9 ]+', translation)
        if trimmed_translation:
            translation = trimmed_translation.group(0).strip()

        if translation and not re.search(r'^\W*$', translation):  # Not empty or only non-word characters
            if debug:
                print(f"Debug: Extracted Translation (processed): {translation}")
            return translation

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
    # Look for the example sentence spans
    example_spans = result_block.find_all('span', style="color:#606060")
    example_sentence = "N/A"
    example_translation = "N/A"

    if example_spans:
        for span in example_spans:
            sentence = span.get_text(strip=True)
            if sentence and "。" in sentence:
                example_sentence = sentence
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

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

# Global variable to control debug printing
DEBUG = False

def debug_print(message):
    if DEBUG:
        print(message)

def get_translation(word):
    start_time = time.time()  # Start time for the entire function
    
    # URL for the search query on Takoboto
    url = f"https://takoboto.jp/?q={word}"
    
    # Make a request to the website
    response = requests.get(url)
    response.encoding = 'utf-8'  # Ensure response is interpreted as UTF-8
    if response.status_code != 200:
        print("Error: Could not fetch data from Takoboto")
        return None
    
    # Parse the HTML content
    soup = BeautifulSoup(response.text, 'html.parser')

    # Measure time to make the request and parse HTML
    after_request_time = time.time()
    debug_print(f"Time to fetch and parse HTML: {after_request_time - start_time:.4f} seconds")

    # Extract the first block of the result
    result = parse_takoboto_result(soup)

    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for get_translation: {end_time - start_time:.4f} seconds")
    return result

def parse_takoboto_result(soup):
    start_time = time.time()  # Start time for this function
    
    # Find the first result block inside #SearchResultContent div
    result_block = soup.find('div', class_='ResultDiv')
    
    if not result_block:
        print("Error: No results found on Takoboto")
        return None

    # Extract Kana and Kanji
    kana, kanji = extract_kana_and_kanji(result_block)
    
    # Extract the English translation
    english_translation = extract_translation(result_block)
    
    # Convert Kana to Romaji
    romaji = convert_kana_to_romaji(kana)
    
    # Extract the example sentence and translate it to English
    example_sentence, example_translation = extract_and_translate_example(result_block)
    
    # Construct the output in XML format
    result = format_output(english_translation, kana, kanji, romaji, example_sentence, example_translation)
    
    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for parse_takoboto_result: {end_time - start_time:.4f} seconds")
    return result

def extract_kana_and_kanji(result_block):
    start_time = time.time()  # Start time for this function

    # Find the first div that contains Kana and Kanji information
    word_line = result_block.find('div', lang='ja')
    kana = "N/A"
    kanji = "N/A"
    
    if word_line:
        # The first span is Kana, following spans may contain Kanji
        spans = word_line.find_all('span')
        if len(spans) > 0:
            kana = spans[0].get_text(strip=True)
        if len(spans) > 1:
            # The rest could contain Kanji or additional notations
            kanji_candidates = [span.get_text(strip=True) for span in spans[1:]]
            # Extract only kanji characters, ignoring kana or punctuation
            kanji = next((extract_kanji(text) for text in kanji_candidates if extract_kanji(text)), "N/A")
    
    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for extract_kana_and_kanji: {end_time - start_time:.4f} seconds")
    return kana, kanji

def extract_kanji(text):
    # Remove any characters that are not kanji (kanji ranges are \u4e00-\u9faf)
    kanji_only = ''.join(re.findall(r'[\u4e00-\u9faf]', text))
    return kanji_only

def extract_translation(result_block):
    start_time = time.time()  # Start time for this function

    # Convert the result block to a BeautifulSoup object for easier parsing
    soup = BeautifulSoup(str(result_block), 'html.parser')
    
    # Look for the '1.' followed by the relevant translation word
    # Directly target the span element after the number '1.'
    first_entry_divs = soup.find_all('div', style=re.compile(r'padding-left:2px;padding-bottom:2px'))
    
    # Iterate through the divs to find the right one
    for div in first_entry_divs:
        # Extract the text within this div
        text = div.get_text(separator=' ', strip=True)

        # Use regex to capture the first word after '1.'
        match = re.search(r'1\.\s*([a-zA-Z]+)', text)
        if match:
            # Total time for this function
            end_time = time.time()
            debug_print(f"Total time for extract_translation: {end_time - start_time:.4f} seconds")
            return match.group(1)
    
    # Debug log: If no match or div found
    debug_print("Debug: No match found for translation.")
    
    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for extract_translation: {end_time - start_time:.4f} seconds")
    return "N/A"

def convert_kana_to_romaji(kana):
    start_time = time.time()  # Start time for this function

    # Convert kana to romaji using pykakasi
    kakasi = pykakasi.kakasi()
    result = kakasi.convert(kana)
    
    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for convert_kana_to_romaji: {end_time - start_time:.4f} seconds")

    # Ensure proper conversion
    if result:
        # Join all the converted romaji parts together
        return ''.join([item['hepburn'] for item in result]).strip()
    return "N/A"

def extract_and_translate_example(result_block):
    start_time = time.time()  # Start time for this function

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
    
    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for extract_and_translate_example: {end_time - start_time:.4f} seconds")
    return example_sentence, example_translation

def translate_to_english(japanese_text):
    start_time = time.time()  # Start time for this function

    try:
        # Use Google Translate to translate the Japanese text to English
        translator = Translator()
        translation = translator.translate(japanese_text, src='ja', dest='en')

        # Total time for this function
        end_time = time.time()
        debug_print(f"Total time for translate_to_english: {end_time - start_time:.4f} seconds")
        return translation.text
    except Exception as e:
        print(f"Error during translation: {e}")
        return "N/A"

def format_output(english_word, hiragana, kanji, romaji, example_sentence, example_translation):
    start_time = time.time()  # Start time for this function

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

    # Total time for this function
    end_time = time.time()
    debug_print(f"Total time for format_output: {end_time - start_time:.4f} seconds")
    return xml_str.encode('utf-8').decode('utf-8')

if __name__ == "__main__":
    # Check if sufficient arguments are provided
    if len(sys.argv) < 2:
        print("Usage: script.py <word> [-debug]")
        sys.exit(1)
    
    # Get the word from command line arguments
    word = sys.argv[1].strip()
    
    # Check for the debug flag
    if len(sys.argv) > 2 and sys.argv[2] == '-debug':
        DEBUG = True

    # Call the translation function
    xml_output = get_translation(word)
    if xml_output:
        print(xml_output)

import argparse
import sqlite3
import zipfile
import os
import shutil
import xml.etree.ElementTree as ET
from googletrans import Translator
from tqdm import tqdm
import jaconv
import re

# Argument parser setup
parser = argparse.ArgumentParser(description='Process an Anki .apkg file and extract vocabulary.')
parser.add_argument('apkg_file', help='Path to the .apkg file')
args = parser.parse_args()

# Define the path to your .apkg file and the extraction directory
apkg_path = args.apkg_file
extract_dir = 'extracted_apkg'

# Extract the lesson name from the filename (without extension)
lesson_name = os.path.splitext(os.path.basename(apkg_path))[0]
subname_name = 'vocabulary'  # Set the subname as desired

# Create the extraction directory if it does not exist
if not os.path.exists(extract_dir):
    os.makedirs(extract_dir)

# Extract the .apkg file
with zipfile.ZipFile(apkg_path, 'r') as zip_ref:
    zip_ref.extractall(extract_dir)

# Path to the extracted SQLite database
anki_db_path = os.path.join(extract_dir, 'collection.anki2')

# Connect to the Anki database
conn = sqlite3.connect(anki_db_path)
cursor = conn.cursor()

# Fetch all notes
cursor.execute("SELECT id, flds FROM notes")
notes = cursor.fetchall()

# Close the connection to the Anki database
conn.close()

# Print the first few notes to debug
if notes:
    print("Sample notes:", notes[:5])
else:
    print("No notes found in the database.")

# Initialize the translator
translator = Translator()

# Define a function to clean up the English translation
def clean_translation(translation):
    # Remove leading and trailing unwanted characters (spaces, dots, semicolons, etc.)
    return re.sub(r'^[\s.;]+|[\s.;]+$', '', translation).lower()

# Define a function to clean up kana and romaji
def clean_kana_romaji(text):
    # Remove leading '～' and any leading or trailing spaces
    return re.sub(r'^[\s～]+|[\s]+$', '', text).lower()

# Function to process field value
def process_field(field):
    # Split the field by semicolon and take the first part
    return field.split(';')[0]

# Function to convert kana to romaji, ensuring the result is accurate for both hiragana and katakana
def convert_kana_to_romaji(kana):
    # Remove exactly '(ゅ)' from kana if found
    kana = kana.replace('(ゅ)', '')

    # Normalize kana to ensure consistency
    normalized_kana = jaconv.normalize(kana)
    # Convert kana to romaji
    romaji = jaconv.kana2alphabet(normalized_kana)
    
    # Handling small kana characters and double consonants manually
    romaji = jaconv.hira2kata(romaji)  # Ensure everything is in katakana for consistency
    romaji = jaconv.kata2alphabet(romaji)  # Convert to romaji
    
    # Additional cleanup for common issues
    romaji = re.sub(r'ッ([kstnhmyrw])', r'\1\1', romaji)  # Double consonant handling (small tsu)
    romaji = re.sub(r'([aeiou])ー', r'\1\1', romaji)  # Prolonged sound mark to same vowel
    
    # Specific replacements for small kana characters
    replacements = {
        'shiyu': 'shu',
        'chiyu': 'chu',
        'jiyu': 'ju',
        'shiyo': 'sho',
        'chiyo': 'cho',
        'jiyo': 'jo',
        'shiya': 'sha',
        'chiya': 'cha',
        'jiya': 'ja'
    }
    for key, value in replacements.items():
        romaji = romaji.replace(key, value)
    
    return romaji

# Transform the notes into the desired XML format if there are any notes
if notes:
    lessons_root = ET.Element("lessons")
    lesson = ET.SubElement(lessons_root, "lesson", name=lesson_name)
    subname = ET.SubElement(lesson, "subname", name=subname_name)
    
    for note in tqdm(notes, desc="Translating and creating XML"):
        note_id, fields = note
        # Split fields by the separator (Anki uses '\x1f' by default)
        field_list = fields.split('\x1f')
        print(field_list)
        # Assuming the format is Japanese (kana), English (translation)
        if len(field_list) >= 2:
            kana = clean_kana_romaji(process_field(field_list[0]))  # Clean and process kana
            english_translation = clean_translation(process_field(field_list[1]))  # Clean and process translation
            
            # Convert kana to romaji and clean it
            romaji = clean_kana_romaji(convert_kana_to_romaji(kana))
            print(romaji)
            # Translate the English translation to Polish
            polish_translation = translator.translate(english_translation, src='en', dest='pl').text.lower()
            comments = english_translation
            ET.SubElement(subname, "word", translation=polish_translation, romaji=romaji, kana=kana, comments=comments)

    # Create a tree from the root element and write it to an XML file
    xml_output_path = f'{lesson_name}.xml'
    tree = ET.ElementTree(lessons_root)
    tree.write(xml_output_path, encoding='utf-8', xml_declaration=True)

    print(f"Vocabulary extracted and saved to '{xml_output_path}'.")

    # Clean up the extraction directory
    shutil.rmtree(extract_dir)
    print(f"Cleaned up extraction directory '{extract_dir}'.")
else:
    print("No data to save.")

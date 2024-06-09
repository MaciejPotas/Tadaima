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

# Transform the notes into the desired XML format if there are any notes
if notes:
    lessons_root = ET.Element("lessons")
    lesson = ET.SubElement(lessons_root, "lesson", name=lesson_name)
    subname = ET.SubElement(lesson, "subname", name=subname_name)
    
    for note in tqdm(notes, desc="Translating and creating XML"):
        note_id, fields = note
        # Split fields by the separator (Anki uses '\x1f' by default)
        field_list = fields.split('\x1f')
        # Assuming the format is Japanese (kana), English (translation)
        if len(field_list) >= 2:
            kana = clean_kana_romaji(field_list[0].split()[0])  # Use the first word in kana and clean it
            english_translation = clean_translation(field_list[1].split()[0])  # Use the first word in translation and clean it
            
            # Convert kana to romaji and clean it
            romaji = clean_kana_romaji(jaconv.kana2alphabet(kana))
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

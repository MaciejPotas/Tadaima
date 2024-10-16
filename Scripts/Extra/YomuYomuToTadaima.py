import csv
import xml.etree.ElementTree as ET
import sys

def parse_csv_to_xml(input_file, output_file):
    # Create the base structure of the XML
    lessons = ET.Element("lessons")
    lesson = ET.SubElement(lessons, "lesson", name="Yomu")
    subname = ET.SubElement(lesson, "subname", name="Lekcja")

    # Open and read the CSV file, skipping the first two rows
    with open(input_file, 'r', encoding='utf-8') as csvfile:
        reader = csv.reader(csvfile)
        
        # Skip the first two rows
        next(reader)
        next(reader)
        
        # Parse each row from the third one
        for row in reader:
            if len(row) < 4:
                continue  # Skip rows that don't have enough columns
            
            kanji, romaji, kana, translation = row[0], row[1], row[2], row[3]
            # Remove any newline characters from the translation
            translation = translation.replace('\n', ';').replace('\r', ';')
            ET.SubElement(subname, "word", translation=translation, romaji=romaji, kana=kana, kanji=kanji, example="")

    # Write the XML structure to the output file
    tree = ET.ElementTree(lessons)
    with open(output_file, 'wb') as xmlfile:
        tree.write(xmlfile, encoding='utf-8', xml_declaration=True)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_csv_file> <output_xml_file>")
    else:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
        parse_csv_to_xml(input_file, output_file)
        print(f"XML file generated: {output_file}")

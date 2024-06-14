import random

def generate_random_phone_number():
    hiragana_numerals = {
        "0": 'ゼロ',
        "1": 'いち',
        "2": 'に',
        "3": 'さん',
        "4": 'よん',
        "5": 'ご',
        "6": 'ろく',
        "7": 'なな',
        "8": 'はち',
        "9": 'きゅう',
        "-": "の",
        " ": " "
    }

    phone_number = '0'
    
    for i in range(9):
        digit = str(random.randint(0, 9))
        
        phone_number += digit
        
        if i in (2, 5):
            phone_number += ' - ' 
    
    hiragana_phone_number = ''.join(hiragana_numerals[digit] for digit in phone_number)
    
    return phone_number, hiragana_phone_number

def play_phone_number_game():
    print("Witaj w grze z losowymi japonskimi numerami telefonow!")
    print("Sprobuj odgadnać losowy numer telefonu wyswietlony w hiraganie.")
    print("Wpisz 'q', aby zakonczyc gre.\n")
    
    while True:
        random_phone_number, hiragana_phone_number = generate_random_phone_number()
        
        print(f"Losowy numer telefonu: {hiragana_phone_number}")
        
        guess = input("Podaj swoja odpowiedz: ")
        
        if guess.lower() == 'q':
            print("Dziekuje za gre!")
            break
        
        if guess == random_phone_number:
            print("Gratulacje! Odgadles poprawny numer telefonu!")
        else:
            print(f"Przykro mi, poprawny numer telefonu to: {random_phone_number}")

play_phone_number_game()

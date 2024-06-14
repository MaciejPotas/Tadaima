import random
import time

# Romaji representations for numbers with exceptions
romaji_numbers = {
    0: "zero", 1: "ichi", 2: "ni", 3: "san", 4: "yon", 5: "go",
    6: "roku", 7: "nana", 8: "hachi", 9: "kyuu", 10: "juu",
    20: "nijuu", 30: "sanjuu", 40: "yonjuu", 50: "gojuu",
    60: "rokujuu", 70: "nanajuu", 80: "hachijuu", 90: "kyuujuu",
    100: "hyaku"
}

exceptions = {
    1: "issai", 8: "hassai", 10: "jussai",
    20: "nijussai", 30: "sanjussai", 40: "yonjussai",
    50: "gojussai", 60: "rokujussai", 70: "nanajussai",
    80: "hachijussai", 90: "kyuujussai", 100: "hyakusai"
}

def number_to_romaji(number):
    if number in exceptions:
        return exceptions[number]
    else:
        tens = (number // 10) * 10
        ones = number % 10
        if tens == 0:
            return romaji_numbers[ones] + "sai"
        elif ones == 0:
            return romaji_numbers[tens] + "sai"
        else:
            return romaji_numbers[tens] + romaji_numbers[ones] + "sai"

def romaji_guess_game():
    print("Welcome to the Romaji Age Guessing Game!", flush=True)
    print("Try to write the shown age in Romaji (don't forget to add 'sai' at the end).", flush=True)
    
    correct_count = 0

    while True:
        age = random.randint(0, 100)
        print(f"\nWhat is the Romaji for the age {age}?", flush=True)
        
        answer = input("Your answer: ").strip().lower()

        correct_romaji = number_to_romaji(age)
              
        if answer == correct_romaji:
            correct_count += 1
            print(f"Correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_romaji}'.", flush=True)
            break
    
    print(f"Game over! You correctly answered {correct_count} ages.", flush=True)
    time.sleep(1) # Sleep for 1 second  
if __name__ == "__main__":
    romaji_guess_game()

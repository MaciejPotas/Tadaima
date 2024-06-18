import random
import time

# Romaji representations for minutes with exceptions
romaji_minutes = {
    1: "ippun", 2: "nifun", 3: "sanpun", 4: "yonpun", 5: "gofun",
    6: "roppun", 7: "nanafun", 8: "happun", 9: "kyuufun", 10: "juppun"
}

def number_to_romaji_minute(number):
    return romaji_minutes[number]

def romaji_minute_guess_game():
    print("Welcome to the Romaji Minute Guessing Game!", flush=True)
    print("Try to write the shown minute in Romaji (e.g., 'ippun' for 1 minute).", flush=True)
    
    correct_count = 0

    while True:
        minute = random.randint(1, 10)
        print(f"\nWhat is the Romaji for {minute} minute(s)?", flush=True)
        
        answer = input("Your answer: ").strip().lower()

        correct_romaji = number_to_romaji_minute(minute)
              
        if answer == correct_romaji:
            correct_count += 1
            print(f"Correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_romaji}'.", flush=True)
        
        # Optionally add a delay before the next question
        time.sleep(1)
    
if __name__ == "__main__":
    romaji_minute_guess_game()

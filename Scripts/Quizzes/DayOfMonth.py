import random
import time

# Romaji representations for days of the month with exceptions
romaji_days = {
    1: "tsuitachi", 2: "futsuka", 3: "mikka", 4: "yokka", 5: "itsuka",
    6: "muika", 7: "nanoka", 8: "youka", 9: "kokonoka", 10: "touka",
    11: "juuichinichi", 12: "juuninichi", 13: "juusannichi", 14: "juuyokka", 15: "juugonichi",
    16: "juurokunichi", 17: "juushichinichi", 18: "juuhachinichi", 19: "juukunichi", 20: "hatsuka",
    21: "nijuuichinichi", 22: "nijuuninichi", 23: "nijuusannichi", 24: "nijuuyokka", 25: "nijuugonichi",
    26: "nijuurokunichi", 27: "nijuushichinichi", 28: "nijuuhachinichi", 29: "nijuukunichi", 30: "sanjuunichi",
    31: "sanjuuichinichi"
}

def number_to_romaji_day(day):
    return romaji_days[day]

def romaji_day_guess_game():
    print("Welcome to the Romaji Day Guessing Game!", flush=True)
    print("Try to write the shown day of the month in Romaji (e.g., 'tsuitachi' for 1st).", flush=True)
    
    correct_count = 0

    while True:
        day = random.randint(1, 31)
        print(f"\nWhat is the Romaji for the {day}th day of the month?", flush=True)
        
        answer = input("Your answer: ").strip().lower()

        correct_romaji = number_to_romaji_day(day)
              
        if answer == correct_romaji:
            correct_count += 1
            print(f"{correct_romaji} is correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_romaji}', not '{answer}'.", flush=True)
        
        # Optionally add a delay before the next question
        time.sleep(1)
    
if __name__ == "__main__":
    romaji_day_guess_game()

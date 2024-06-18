import random
import time

# Romaji representations for hours with exceptions
romaji_hours = {
    1: "ichiji", 2: "niji", 3: "sanji", 4: "yoji", 5: "goji",
    6: "rokuji", 7: "shichiji", 8: "hachiji", 9: "kuji", 10: "juuji",
    11: "juuichiji", 12: "juuniji"
}

def number_to_romaji_hour(hour, period):
    if period == "AM":
        return "gozen " + romaji_hours[hour]
    else:
        return "gogo " + romaji_hours[hour]

def romaji_hour_guess_game():
    print("Welcome to the Romaji Hour Guessing Game!", flush=True)
    print("Try to write the shown hour in Romaji including 'gozen' for AM or 'gogo' for PM.", flush=True)
    
    correct_count = 0

    while True:
        hour = random.randint(1, 12)
        period = random.choice(["AM", "PM"])
        print(f"\nWhat is the Romaji for {hour} o'clock {period}?", flush=True)
        
        answer = input("Your answer: ").strip().lower()

        correct_romaji = number_to_romaji_hour(hour, period)
              
        if answer == correct_romaji:
            correct_count += 1
            print(f"Correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_romaji}'.", flush=True)
        
        # Optionally add a delay before the next question
        time.sleep(1)
    
if __name__ == "__main__":
    romaji_hour_guess_game()

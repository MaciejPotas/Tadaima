import random
import time

# Romaji representations for minutes with exceptions
romaji_minutes = {
    1: "ippun", 2: "nifun", 3: "sanpun", 4: "yonpun", 5: "gofun",
    6: "roppun", 7: "nanafun", 8: "happun", 9: "kyuufun", 10: "juppun",
    11: "juuippun", 12: "juunifun", 13: "juusanpun", 14: "juuyonpun", 15: "juugofun",
    16: "juuroppun", 17: "juunanafun", 18: "juuhappun", 19: "juukyuufun", 20: "nijuppun",
    21: "nijuuippun", 22: "nijuunifun", 23: "nijuusanpun", 24: "nijuuyonpun", 25: "nijuugofun",
    26: "nijuuroppun", 27: "nijuunanafun", 28: "nijuuhappun", 29: "nijuukyuufun", 30: "sanjuppun",
    31: "sanjuuippun", 32: "sanjuunifun", 33: "sanjuusanpun", 34: "sanjuuyonpun", 35: "sanjuugofun",
    36: "sanjuuroppun", 37: "sanjuunanafun", 38: "sanjuuhappun", 39: "sanjuukyuufun", 40: "yonjuppun",
    41: "yonjuuippun", 42: "yonjuunifun", 43: "yonjuusanpun", 44: "yonjuuyonpun", 45: "yonjuugofun",
    46: "yonjuuroppun", 47: "yonjuunanafun", 48: "yonjuuhappun", 49: "yonjuukyuufun", 50: "gojuppun",
    51: "gojuuippun", 52: "gojuunifun", 53: "gojuusanpun", 54: "gojuuyonpun", 55: "gojuugofun",
    56: "gojuuroppun", 57: "gojuunanafun", 58: "gojuuhappun", 59: "gojuukyuufun"
}

# Romaji representations for hours with exceptions
romaji_hours = {
    1: "ichiji", 2: "niji", 3: "sanji", 4: "yoji", 5: "goji",
    6: "rokuji", 7: "shichiji", 8: "hachiji", 9: "kuji", 10: "juuji",
    11: "juuichiji", 12: "juuniji"
}

def number_to_romaji_minute(number):
    return romaji_minutes[number]

def number_to_romaji_hour(hour, period):
    if period == "AM":
        return "gozen " + romaji_hours[hour]
    else:
        return "gogo " + romaji_hours[hour]

def number_to_romaji_time(hour, minute, period):
    return number_to_romaji_hour(hour, period) +" "+ number_to_romaji_minute(minute)

def romaji_time_guess_game():
    print("Welcome to the Romaji Time Guessing Game!", flush=True)
    print("Try to write the shown time in Romaji (e.g., 'gozen ichiji ippun' for 1:01 AM).", flush=True)
    
    correct_count = 0

    while True:
        hour = random.randint(1, 12)
        minute = random.randint(1, 60)
        period = random.choice(["AM", "PM"])
        print(f"\nWhat is the Romaji for {hour}:{minute:02d} {period}?", flush=True)
        
        answer = input("Your answer: ").strip().lower()

        correct_romaji = number_to_romaji_time(hour, minute, period)
              
        if answer == correct_romaji:
            correct_count += 1
            print(f"{correct_romaji} is correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_romaji}, not {answer}'.", flush=True)
        
        # Optionally add a delay before the next question
        time.sleep(1)
    
if __name__ == "__main__":
    romaji_time_guess_game()

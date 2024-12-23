import random
import sys
import time

try:
    sys.stdout.reconfigure(encoding='utf-8')
except AttributeError:
    pass

adjectives = {
    # i-adjectives
    "takai": {
        "type": "i",
        "meaning": "expensive",
        "present": "takai",
        "negative": "takakunai",
        "past": "takakatta",
        "past_negative": "takakunakatta"
    },
    "oishii": {
        "type": "i",
        "meaning": "delicious",
        "present": "oishii",
        "negative": "oishikunai",
        "past": "oishikatta",
        "past_negative": "oishikunakatta"
    },
    "yasui": {
        "type": "i",
        "meaning": "cheap",
        "present": "yasui",
        "negative": "yasukunai",
        "past": "yasukatta",
        "past_negative": "yasukunakatta"
    },
    "samui": {
        "type": "i",
        "meaning": "cold (weather)",
        "present": "samui",
        "negative": "samukunai",
        "past": "samukatta",
        "past_negative": "samukunakatta"
    },
    "atarashii": {
        "type": "i",
        "meaning": "new",
        "present": "atarashii",
        "negative": "atarashikunai",
        "past": "atarashikatta",
        "past_negative": "atarashikunakatta"
    },
    "furui": {
        "type": "i",
        "meaning": "old",
        "present": "furui",
        "negative": "furukunai",
        "past": "furukatta",
        "past_negative": "furukunakatta"
    },
    "tanoshii": {
        "type": "i",
        "meaning": "fun",
        "present": "tanoshii",
        "negative": "tanoshikunai",
        "past": "tanoshikatta",
        "past_negative": "tanoshikunakatta"
    },
    "omoshiroi": {
        "type": "i",
        "meaning": "interesting",
        "present": "omoshiroi",
        "negative": "omoshirokunai",
        "past": "omoshirokatta",
        "past_negative": "omoshirokunakatta"
    },
    "kawaii": {
        "type": "i",
        "meaning": "cute",
        "present": "kawaii",
        "negative": "kawaikunai",
        "past": "kawaikatta",
        "past_negative": "kawaikunakatta"
    },

    # na-adjectives
    "kirei": {
        "type": "na",
        "meaning": "beautiful/clean",
        "present": "kirei desu",
        "negative": "kirei dewa arimasen",
        "past": "kirei deshita",
        "past_negative": "kirei dewa arimasen deshita"
    },
    "shizuka": {
        "type": "na",
        "meaning": "quiet",
        "present": "shizuka desu",
        "negative": "shizuka dewa arimasen",
        "past": "shizuka deshita",
        "past_negative": "shizuka dewa arimasen deshita"
    },
    "genki": {
        "type": "na",
        "meaning": "healthy/energetic",
        "present": "genki desu",
        "negative": "genki dewa arimasen",
        "past": "genki deshita",
        "past_negative": "genki dewa arimasen deshita"
    },
    "kantan": {
        "type": "na",
        "meaning": "easy/simple",
        "present": "kantan desu",
        "negative": "kantan dewa arimasen",
        "past": "kantan deshita",
        "past_negative": "kantan dewa arimasen deshita"
    },
    "suki": {
        "type": "na",
        "meaning": "like",
        "present": "suki desu",
        "negative": "suki dewa arimasen",
        "past": "suki deshita",
        "past_negative": "suki dewa arimasen deshita"
    },
    "kirai": {
        "type": "na",
        "meaning": "dislike",
        "present": "kirai desu",
        "negative": "kirai dewa arimasen",
        "past": "kirai deshita",
        "past_negative": "kirai dewa arimasen deshita"
    },
    "yuumei": {
        "type": "na",
        "meaning": "famous",
        "present": "yuumei desu",
        "negative": "yuumei dewa arimasen",
        "past": "yuumei deshita",
        "past_negative": "yuumei dewa arimasen deshita"
    },
    "benri": {
        "type": "na",
        "meaning": "convenient",
        "present": "benri desu",
        "negative": "benri dewa arimasen",
        "past": "benri deshita",
        "past_negative": "benri dewa arimasen deshita"
    }
}

# New patterns:
# The first adjective is always present (connect form).
# The second adjective can be present, negative, past, or past_negative.
patterns = [
    {
        "description": "Is {A_meaning} and {B_meaning}.",
        "A_form": ("present", "connect"),
        "B_form": ("present", "final")
    },
    {
        "description": "Is {A_meaning} and not {B_meaning}.",
        "A_form": ("present", "connect"),
        "B_form": ("negative", "final")
    },
    {
        "description": "Is {A_meaning} and was {B_meaning}.",
        "A_form": ("present", "connect"),
        "B_form": ("past", "final")
    },
    {
        "description": "Is {A_meaning} and was not {B_meaning}.",
        "A_form": ("present", "connect"),
        "B_form": ("past_negative", "final")
    }
]

def to_connect_form(adj_key, form_type):
    adj = adjectives[adj_key]
    base_form = adj[form_type]

    if adj["type"] == "i":
        # For i-adjectives connecting form from present: [root]i -> [root]ikute
        # Since first adjective is always present form, it's always base [present] form
        # and we just make it kute form from the dictionary (present) form.
        # If it were negative we would do negative -> negative + kute, but we decided first is always present.
        return base_form[:-1] + "kute"
    else:
        # For na-adjectives, always root + "de"
        return adj_key + " de"

def final_form(adj_key, form_type):
    adj = adjectives[adj_key]
    base_form = adj[form_type]

    if adj["type"] == "i":
        # Add "desu" for i-adjective final forms
        return base_form + " desu"
    else:
        # na-adjectives already include 'desu' forms
        return base_form

def quiz_game():
    print("Welcome to the Combined Adjective Quiz!")
    print("The first adjective is always in present form and connected with 'kute' (i-adj) or 'de' (na-adj).")
    print("The second adjective can be present, negative, past, or past-negative.")
    print("You will be given a description in English, and you must form the Japanese sentence in romanji.")
    print()

    keys = list(adjectives.keys())
    correct_count = 0

    while True:
        pattern = random.choice(patterns)
        A, B = random.sample(keys, 2)

        desc = pattern["description"].format(
            A_meaning=adjectives[A]["meaning"],
            B_meaning=adjectives[B]["meaning"]
        )

        print("\n" + desc)
        
        A_form_type, A_position = pattern["A_form"]
        B_form_type, B_position = pattern["B_form"]

        if A_position == "connect":
            A_part = to_connect_form(A, A_form_type)
        else:
            A_part = final_form(A, A_form_type)

        if B_position == "connect":
            B_part = to_connect_form(B, B_form_type)
        else:
            B_part = final_form(B, B_form_type)

        sentence = A_part + " " + B_part

        answer = input("Your answer: ").strip().lower()

        if answer == sentence:
            correct_count += 1
            print(f"Correct! You've got {correct_count} correct so far.")
        else:
            print(f"Wrong. The correct answer is '{sentence}'.")

        time.sleep(1)

if __name__ == "__main__":
    quiz_game()

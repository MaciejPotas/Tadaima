import random
import sys
import time

# Set the default encoding to UTF-8 if possible
try:
    sys.stdout.reconfigure(encoding='utf-8')
except AttributeError:
    pass

def get_popular_adjectives():
    """
    Returns a predefined list of popular Japanese adjectives (both i- and na-adjectives)
    with their various conjugated forms in romanji.
    Each entry includes: (roman, hiragana, meaning, present, negative, past, past_negative).
    """

    # i-adjectives
    i_adjectives = [
        ("kawaii", "かわいい", "cute", "kawaii", "kawaikunai", "kawaikatta", "kawaikunakatta"),
        ("atsui", "あつい", "hot", "atsui", "atsukunai", "atsukatta", "atsukunakatta"),
        ("samui", "さむい", "cold", "samui", "samukunai", "samukatta", "samukunakatta"),
        ("oishii", "おいしい", "delicious", "oishii", "oishikunai", "oishikatta", "oishikunakatta"),
        ("takai", "たかい", "expensive/high", "takai", "takakunai", "takakatta", "takakunakatta"),
        ("yasui", "やすい", "cheap", "yasui", "yasukunai", "yasukatta", "yasukunakatta"),
        ("omoshiroi", "おもしろい", "interesting", "omoshiroi", "omoshirokunai", "omoshirokatta", "omoshirokunakatta"),
        ("tanoshii", "たのしい", "fun", "tanoshii", "tanoshikunai", "tanoshikatta", "tanoshikunakatta")
    ]

    # na-adjectives
    na_adjectives = [
        ("kirei", "きれい", "beautiful/clean", "kireidesu", "kireidewaarimasen", "kireideshita", "kireidewaarimasendeshita"),
        ("shizuka", "しずか", "quiet", "shizukadesu", "shizukadewaarimasen", "shizukadeshita", "shizukadewaarimasendeshita"),
        ("genki", "げんき", "healthy/energetic", "genkidesu", "genkidewaarimasen", "genkideshita", "genkidewaarimasendeshita"),
        ("kantan", "かんたん", "easy/simple", "kantandesu", "kantandewaarimasen", "kantandeshita", "kantandewaarimasendeshita"),
        ("suki", "すき", "like", "sukidesu", "sukidewaarimasen", "sukideshita", "sukidewaarimasendeshita")
    ]

    return i_adjectives + na_adjectives

def quiz_game():
    """
    Main function to run the Japanese Adjective Quiz.
    Randomly asks for forms (present, negative, past, past negative) of popular Japanese adjectives.
    """
    print("Welcome to the Japanese Adjective Quiz!", flush=True)
    print("Answer the questions in romanji for the correct forms of the adjectives.", flush=True)
    print("For i-adjectives (e.g., kawaii): kawaii, kawaikunai, kawaikatta, kawaikunakatta", flush=True)
    print("For na-adjectives (e.g., kirei): kireidesu, kireidewaarimasen, kireideshita, kireidewaarimasendeshita", flush=True)
    
    adjectives = get_popular_adjectives()
    correct_count = 0

    while True:
        # Randomly select an adjective and form
        (adj_romaji, adj_hiragana, meaning, present, neg, past, past_neg) = random.choice(adjectives)
        
        forms = {
            "present": (present, f"What is the present form of '{adj_hiragana}' ({meaning})?"),
            "negative": (neg, f"What is the present negative form of '{adj_hiragana}' ({meaning})?"),
            "past": (past, f"What is the past form of '{adj_hiragana}' ({meaning})?"),
            "past_negative": (past_neg, f"What is the past negative form of '{adj_hiragana}' ({meaning})?")
        }
        
        form_type = random.choice(list(forms.keys()))
        correct_answer, question = forms[form_type]
        
        print(f"\n{question}", flush=True)
        answer = input("Your answer: ").strip().lower()
        
        if answer == correct_answer:
            correct_count += 1
            print(f"Correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_answer}', not '{answer}'.", flush=True)

        # Optional: delay before the next question
        time.sleep(1)

if __name__ == "__main__":
    quiz_game()

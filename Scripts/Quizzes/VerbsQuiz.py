import random
import sys
import time

# Set the default encoding to UTF-8 if possible
try:
    sys.stdout.reconfigure(encoding='utf-8')
except AttributeError:
    pass

def get_popular_verbs():
    """
    Returns a predefined list of popular Japanese verbs with their forms and translations.
    Each entry includes verb (plain), translation, masu, negative-masu, past-masu, and past-negative-masu forms in romanji.
    """
    verbs = [
        ("taberu", "たべる", "to eat", "tabemasu", "tabemasen", "tabemashita", "tabemasendeshita"),
        ("nomu", "のむ", "to drink", "nomimasu", "nomimasen", "nomimashita", "nomimasendeshita"),
        ("kiku", "きく", "to listen", "kikimasu", "kikimasen", "kikimashita", "kikimasendeshita"),
        ("hanasu", "はなす", "to speak", "hanashimasu", "hanashimasen", "hanashimashita", "hanashimasendeshita"),
        ("miru", "みる", "to see", "mimasu", "mimasen", "mimashita", "mimasendeshita"),
        ("kuru", "くる", "to come", "kimasu", "kimasen", "kimashita", "kimasendeshita"),
        ("suru", "する", "to do", "shimasu", "shimasen", "shimashita", "shimasendeshita"),
        ("iku", "いく", "to go", "ikimasu", "ikimasen", "ikimashita", "ikimasendeshita"),
        ("kaeru", "かえる", "to return", "kaerimasu", "kaerimasen", "kaerimashita", "kaerimasendeshita"),
        ("aru", "ある", "to exist (inanimate)", "arimasu", "arimasen", "arimashita", "arimasendeshita"),
        ("iru", "いる", "to exist (animate)", "imasu", "imasen", "imashita", "imasendeshita"),
        ("yomu", "よむ", "to read", "yomimasu", "yomimasen", "yomimashita", "yomimasendeshita"),
        ("asobu", "あそぶ", "to play", "asobimasu", "asobimasen", "asobimashita", "asobimasendeshita"),
        ("hashiru", "はしる", "to run", "hashirimasu", "hashirimasen", "hashirimashita", "hashirimasendeshita"),
        ("au", "あう", "to meet", "aimasu", "aimasen", "aimashita", "aimasendeshita"),
        ("tsukuru", "つくる", "to make", "tsukurimasu", "tsukurimasen", "tsukurimashita", "tsukurimasendeshita"),
        ("oyogu", "およぐ", "to swim", "oyogimasu", "oyogimasen", "oyogimashita", "oyogimasendeshita"),
        ("aruku", "あるく", "to walk", "arukimasu", "arukimasen", "arukimashita", "arukimasendeshita"),
        ("kiru", "きる", "to wear", "kimasu", "kimasen", "kimashita", "kimasendeshita"),
        ("kaku", "かく", "to write", "kakimasu", "kakimasen", "kakimashita", "kakimasendeshita")
    ]
    return verbs

def quiz_game():
    """
    Main function to run the Japanese verb quiz.
    Randomly asks for -masu forms (positive/negative, past/present) of popular Japanese verbs.
    """
    print("Welcome to the Japanese Verb Quiz!", flush=True)
    print("Answer the questions in romanji for the correct forms of the verbs.", flush=True)
    print("For example: tabemasu, tabemasen, tabemashita, tabemasendeshita.", flush=True)
    
    verbs = get_popular_verbs()
    correct_count = 0

    while True:
        # Randomly select a verb and form
        verb, verb_hiragana, meaning, masu, neg_masu, past_masu, past_neg_masu = random.choice(verbs)
        forms = {
            "masu": (masu, "What is the present form of '{}' ({})?".format(verb_hiragana, meaning)),
            "neg_masu": (neg_masu, "What is the present negative form of '{}' ({})?".format(verb_hiragana, meaning)),
            "past_masu": (past_masu, "What is the past form of '{}' ({})?".format(verb_hiragana, meaning)),
            "past_neg_masu": (past_neg_masu, "What is the past negative form of '{}' ({})?".format(verb_hiragana, meaning))
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

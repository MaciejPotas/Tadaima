import random
import time
import sys

# Set the default encoding to UTF-8 if possible
try:
    sys.stdout.reconfigure(encoding='utf-8')
except AttributeError:
    # Reconfigure might not be available in older Python versions, so we skip it
    pass

# Explanations:
# Dore (どれ): "Which one" – Used when selecting from multiple items (e.g., "Which one is the red apple?").
# Dono (どの): "Which" – Used to specify which item from a known group (e.g., "Which book is yours?").
# Doko (どこ): "Where" – Used to ask about a location (e.g., "Where is the station?").
# Dare (だれ): "Who" – Used to ask about a person (e.g., "Who is the teacher?").
# Dareno (だれの): "Whose" – Used to ask about ownership (e.g., "Whose pen is this?").

subjects = ["apple", "book", "pen", "car", "house"]
locations = ["station", "school", "park", "hospital", "restaurant"]
people = ["teacher", "student", "doctor", "driver", "chef"]

def generate_question():
    question_type = random.choice(["which_one", "which", "where", "who", "whose"])
    if question_type == "which_one":
        subject = random.choice(subjects)
        question = f"Which one is the red {subject}?"
        answer = "dore"
    elif question_type == "which":
        subject = random.choice(subjects)
        question = f"Which {subject} is yours?"
        answer = "dono"
    elif question_type == "where":
        location = random.choice(locations)
        question = f"Where is the {location}?"
        answer = "doko"
    elif question_type == "who":
        person = random.choice(people)
        question = f"Who is the {person}?"
        answer = "dare"
    elif question_type == "whose":
        subject = random.choice(subjects)
        question = f"Whose {subject} is this?"
        answer = "dareno"
    
    choices = ["dore", "dono", "doko", "dare", "dareno"]
    random.shuffle(choices)
    
    return {"question": question, "answer": answer, "choices": choices}

def japanese_demonstratives_quiz_game():
    print("Welcome to the Japanese Demonstratives Quiz Game!", flush=True)
    print("Try to answer the questions using 'dore', 'dono', 'doko', 'dare', or 'dareno'.", flush=True)
    
    # Print explanations
    print("\nExplanations:", flush=True)
    print("Dore (どれ): 'Which one' – Used when selecting from multiple items (e.g., 'Which one is the red apple?').", flush=True)
    print("Dono (どの): 'Which' – Used to specify which item from a known group (e.g., 'Which book is yours?').", flush=True)
    print("Doko (どこ): 'Where' – Used to ask about a location (e.g., 'Where is the station?').", flush=True)
    print("Dare (だれ): 'Who' – Used to ask about a person (e.g., 'Who is the teacher?').", flush=True)
    print("Dareno (だれの): 'Whose' – Used to ask about ownership (e.g., 'Whose pen is this?').", flush=True)
    
    correct_count = 0

    while True:
        question_data = generate_question()
        print("\nQuestion:", question_data["question"])
        
        answer = input("Your answer: ").strip().lower()

        if answer == question_data["answer"]:
            correct_count += 1
            print(f"{answer} is correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{question_data['answer']}', not '{answer}'.", flush=True)
        
        # Optionally add a delay before the next question
        time.sleep(1)

if __name__ == "__main__":
    japanese_demonstratives_quiz_game()

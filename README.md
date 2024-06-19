


# Tadaima!

Tadaima is an application used to help learning vocabulary, especially Japanese.

The Japanese Vocabulary and Grammar Learning Tool is a desktop application designed to assist users in learning Japanese through structured lessons, interactive flashcards, and quizzes. The application is built using C++ with the ImGui library for the graphical user interface. It supports Windows and integrates with a SQLite database for persistent storage of lessons and user progress.

## Features implemented at the moment ( at least, pariatlly :-)) )

### Lesson Management
- **Custom Lessons**: Users can create custom dictionary-like lessons with an unlimited number of words.
- **Structured Organization**: Lessons can be grouped under main names (e.g., "Grammar", "Vocabulary") with sub-names (e.g., "Basics", "Lesson 1").
- **Lesson Overview**: A tree view widget provides an overview of all lessons, grouped by their main names. Users can expand each main lesson to view sub-lessons and the words they contain.
- **Translation Support**: Use a Python script to translate English words to Japanese and get their Hiragana and Romaji representations.

### Flashcards
- **Interactive Flashcards**: Each flashcard includes a word in Kana, its Japanese translation, pronunciation in Romaji, and an example sentence.
- **Easy Addition**: Users can add new words to lessons with fields for Kana, translation, Romaji, and example sentences all available at once.

### Quizzes
- **Multiple Quiz Types**: The application includes multiple-choice, fill-in-the-blank, and matching quizzes.
- **Bidirectional Learning**: Quizzes can test knowledge from Japanese to English and vice versa.

### Spaced Repetition System
- **Leitner System**: Words are moved between different levels based on user performance, with higher levels reviewed less frequently.
- **SuperMemo and Anki Algorithms**: Users can choose from different spaced repetition algorithms to optimize their learning.

### Progress Tracking
- **Dashboard**: A main dashboard displays all lessons and key statistics, such as daily study time, words learned, accuracy rate, and last date of learning.
- **Detailed Stats**: Track progress for each lesson individually, helping users identify strengths and areas for improvement.

### User Interface
- **Intuitive Design**: The application features a clean and intuitive interface with interactive elements like buttons and tree nodes.
- **Customization**: Users can add, edit, and delete lessons and words with ease. The interface also supports opening modals for adding new lessons or editing existing ones.

### Extra Features
- **Python Script Execution**: The application supports running extra python scripts for additional quizes.
Flexible Script Support: The application can run any script, Python, that adheres to the input/output requirements.
Interactive Execution: Users can interact with the running scripts in real-time, sending inputs and receiving outputs dynamically.
Extensibility: Custom scripts can be added to extend the application's functionality without modifying the core application code.
Seamless Integration: The output from the scripts can be displayed within the application interface, making it easy to incorporate script results into the user experience.

## Usage
- **Creating Lessons**: Users can create new lessons by clicking the "Add New Lesson" button and filling in the lesson details and words.
- **Viewing Lessons**: The tree view widget displays all lessons. Users can expand each main lesson node to view sub-lessons and their words.
- **Editing and Deleting Lessons**: Options to edit or delete lessons are available directly from the tree view.
- **Taking Quizzes**: Users can take various types of quizzes to test their knowledge and reinforce learning.
- **Tracking Progress**: The dashboard provides insights into learning progress and performance over time.

### How to:

Before Tadaima can reach over 9000 power, few steps to be done before rhe run :)
* Grab installer from the repo ( Installer dir ).
* You need to have a Python 3 installed on your device,
* When Tadaima is installed, go to the scripts directory and run Setup.bat. It creates virtual environments so dictionary script, which is written in python can be used without any more concerns.
## Main window
![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/7f020337-820f-40ca-81e7-4d7629917704)

Very basic application window. Dashboard is still in the development, but version 1.0.0 doesn't include it.

## Application settings
![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/cc682b08-a220-4c86-b845-929bb1d08c29)

General Settings Tab:

- Username: Enter your username in the provided text field labeled "User name".
- Dictionary Path: Enter the path to your Japanese word files directory.
- Script Paths: Enter the path to the directory containing your scripted quizzes.
- Show Logs: Check or uncheck the box to enable or disable logging.
  
Quiz Settings Tab:

- Option: Choose the word type you will use for input during quizzes (BaseWord, Kana, Romaji).
- Translation Word Option: Select the word type you will be asked for during quizzes (BaseWord, Kana, Romaji).
- Flip Button: Click the "Flip" button to swap the input and translation word options.

## Adding a lesson
Click to open the Lesson Settings widget. If you are editing an existing lesson, it will open in "Edit Lesson Modal". Otherwise, it will open in "Add New Lesson Modal". Modal Window: The modal window will appear with input fields for the lesson and word details.

Word List: On the right side, you will see a list of words added to the lesson.

![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/5c98c287-fea5-47ec-b71b-a0e92a368e86)

Adding a new lesson:
**Nice feature here is point 3**. It allows to use a script to translate the word instantly to the romaji/kana.
Script is stored in the scripts/Dictionary.py and this path should be set in the settings. For the script to be properly used, the virtual environments has to be created. Described above :)
1) Main Name: Enter the main name of the lesson in the "Main Name" input field.
2) Sub Name: Enter the sub name of the lesson in the "Sub Name" input field.
3) Translation: Enter the translation of the word in the "Translation" input field.
4) Translate Button: Click "Translate" to use the dictionary to fill in Romaji and Kana fields automatically.
5) Romaji: Enter the Romaji representation of the word.
6) Kana: Enter the Kana representation of the word.
7) Example Sentence: Provide an example sentence for the word.
8) Tags: Enter tags for the word, separated by commas.
9) Add Word: Click "Add Word" to add the word to the lesson.

Managing Words in the Lesson:

- Select Word: Click on a word to select it for editing or removal.
- Update Word: After selecting a word, modify the fields and click "Update Word" to save changes.
- Remove Word: Click "Remove Word" to delete the selected word from the lesson.
- Save Lesson: Click "Save Lesson" to save the lesson with the new or updated words.

## Play quiz!

### The Vocabulary Quiz in the Tadaima! 
application is designed to help users learn and retain vocabulary through a flashcard-based quiz system. Here are the detailed rules for the Vocabulary Quiz:

![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/30be5ea1-2a0f-43fa-a1fc-0ae2647d7b36)

Getting Started: You’ll have a stack of flashcards with Japanese words. The cards can be shuffled for a random order.

Taking the Quiz: Look at the word on the flashcard and type your answer. If you get it right, your correct answer count goes up. If wrong, your incorrect answer count goes up.

Moving On: After each answer, the quiz moves to the next flashcard. It will keep going until all words are marked as "learnt".

Extra Features: The quiz tracks how many times you got each word right or wrong and if you've learnt it. You can see your progress and check the current flashcard anytime.

Buy pressing F1, you can get an extra hint :)

### Multiple Choice Quiz! Here’s how it works:
![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/051465fa-6c32-4271-9e36-3ea0e094cff8)

Setup: Choose lesson, lessons or mixed words.

Taking the Quiz: Question: You'll see a word and need to pick the correct translation from multiple choices.

Answering: Select the correct option. The quiz tracks your correct answers.

Advancing: Move to the next word after each answer.

Completion: The quiz ends when all words are answered.

Enjoy your learning journey with the Tadaima! Multiple Choice Quiz!

## Script runner

This widget allows you to manage and run quiz scripts, view outputs, and interact with the running scripts.

![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/1c45f038-dbdb-419a-856c-813c3c71cea8)

The widget initializes by loading the scripts from the specified directory. This is configured in your settings.
Ensure the scripts directory path is correctly set in your settings for the widget to locate the Python scripts.
Script Selection:

Available Scripts: On the left column, you will see a list of available Python scripts.
Select a Script: Double-click on a script name to select and run it. The script output will be displayed in the right section.
Running and Managing Scripts:

Run Script: Click the "Run Script" button to start the selected script if no script is currently running.
Stop Script: Click the "Stop Script" button to stop the currently running script.
Clear Output: Click the "Clear window" button to clear the output displayed in the widget.:

Script Output: The output of the running script will be displayed in the central area. The output is dynamically updated.
Auto-Scroll: The output window will automatically scroll to the bottom to show the latest output. You can manually scroll if needed.

Inputting Commands: At the bottom of the widget, there is an input box where you can type commands or input required by the running script.
Submit Input: Press Enter to submit the input. The input will be sent to the running script.

### Code Base Structure
```
Tadaima/ # Root project directory
│
├── Tadaima.sln # Visual Studio solution file
│
├── libraries/ # External dependencies and libraries
│ ├── imgui/ # ImGui library files
│ └── ...
│
├── src/ # Source files for the application
│ ├── main.cpp # Main application entry point
│ ├── Application/ # All backend logic
│ │ └── ...
│ ├── Gui/ # All GUIs implementation
│ │ └── ...
│ └── ... # Other source files
│
├── resources/ # Resources like icons, images, and data files
│ └── ...
│
├── tests/ # Unit tests
│ └── ... # Other tests
│
│ └── docs/ # Documentation files
│
├──Scripts
│ └── Extra # extra scripts for learning  ( converting anki to xmls ).
│ └── Quizzes # Quiz scripts that are later used for execution within the applicaion.

```
## Some technical Implementation
Script Execution

The `ScriptRunner` class uses named pipes for inter-process communication (IPC) between the main application and the script process. This allows the application to send input to the script and receive its output in real-time.

### Overview of Named Pipes

Named pipes provide a reliable, one-way or duplex channel for communication. In the context of the `ScriptRunner`, named pipes are used to:
1. **Send Input**: Send user input from the application to the script.
2. **Receive Output**: Receive the script's output and display it in the GUI.
   
Benefits of Using Pipes
Real-Time Interaction: Pipes allow for real-time communication between the application and the script, enabling immediate feedback and interaction.
Decoupling: The script process runs independently of the main application, improving modularity and separation of concerns.
Flexibility: Named pipes can handle large volumes of data and support both synchronous and asynchronous operations, providing flexibility in how data is managed.

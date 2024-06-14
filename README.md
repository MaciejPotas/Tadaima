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
- 
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

### Base Structure
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

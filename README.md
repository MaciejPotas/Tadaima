# Tadaima!

Tadaima is an application used to help learning vocabulary.
Especially japanese.

The Japanese Vocabulary and Grammar Learning Tool is a desktop application designed to assist users in learning Japanese through structured lessons, interactive flashcards, and quizzes. 
The application is built using C++ with the IMGui library for the graphical user interface. It supports Windows and integrates with a SQLite database for persistent storage of lessons and user progress.

List below is implemented, being implementing and will be implemented :-)

Features:

Lesson Management

- Custom Lessons: Users can create custom dictionary-like lessons with an unlimited number of words.
- Structured Organization: Lessons can be grouped under main names (e.g., "Grammar", "Vocabulary") with sub-names (e.g., "Basics", "Lesson 1").
- Lesson Overview: A tree view widget provides an overview of all lessons, grouped by their main names. Users can expand each main lesson to view sub-lessons and the words they contain.

Flashcards

- Interactive Flashcards: Each flashcard includes a word in Kana, its Japanese translation, pronunciation in Romaji, and an example sentence.
- Easy Addition: Users can add new words to lessons with fields for Kana, translation, Romaji, and example sentences all available at once.

Quizzes

- Multiple Quiz Types: The application includes multiple-choice, fill-in-the-blank, and matching quizzes.
- Bidirectional Learning: Quizzes can test knowledge from Japanese to English and vice versa.

Spaced Repetition System

- Leitner System: Words are moved between different levels based on user performance, with higher levels reviewed less frequently.
- SuperMemo and Anki Algorithms: Users can choose from different spaced repetition algorithms to optimize their learning.
- Progress Tracking

Dashboard: 

- A main dashboard displays all lessons and key statistics, such as daily study time, words learned, accuracy rate, and last date of learning.
- Detailed Stats: Track progress for each lesson individually, helping users identify strengths and areas for improvement.
- User Interface

User Interface

- Intuitive Design: The application features a clean and intuitive interface with interactive elements like buttons and tree nodes.
- Customization: Users can add, edit, and delete lessons and words with ease. The interface also supports opening modals for adding new lessons or editing existing ones.

Extra Features

Script Execution: The application supports running extra scripts for additional functionality, allowing for extensibility and customization.

Usage
- Creating Lessons: Users can create new lessons by clicking the "Add New Lesson" button and filling in the lesson details and words.
- Viewing Lessons: The tree view widget displays all lessons. Users can expand each main lesson node to view sub-lessons and their words.
- Editing and Deleting Lessons: Options to edit or delete lessons are available directly from the tree view.
- Taking Quizzes: Users can take various types of quizzes to test their knowledge and reinforce learning.
- Tracking Progress: The dashboard provides insights into learning progress and performance over time.

## Getting started

Technical Implementation
- Frontend: Implemented using IMGui for a responsive and interactive user interface.
- Backend: Powered by a SQLite database for persistent storage of lessons, words, and user progress.
- Data Management: Utilizes a LessonManager class to handle database interactions and a LessonTreeViewWidget for displaying lessons in a tree view.
- Complex Data Packages: Uses a ComplexDataPackage class to pass data between GUI components, ensuring a clean separation of the GUI from database operations.
  
Base structure for what can be found and where:
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
└── docs/ # Documentation files
└── ...
```
## Database
Lessons database are constructed as:
```
-- Create Lessons table
CREATE TABLE IF NOT EXISTS Lessons (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    mainName TEXT NOT NULL,
    subName TEXT NOT NULL
);

-- Create Words table
CREATE TABLE IF NOT EXISTS Words (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    lessonId INTEGER NOT NULL,
    kana TEXT NOT NULL,
    translation TEXT NOT NULL,
    romaji TEXT,
    exampleSentence TEXT,
    FOREIGN KEY(lessonId) REFERENCES Lessons(id)
);

-- Create Tags table
CREATE TABLE IF NOT EXISTS Tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    wordId INTEGER NOT NULL,
    tag TEXT NOT NULL,
    FOREIGN KEY(wordId) REFERENCES Words(id)
);
```
## Integrate with your tools

## Test and Deploy

To inspect/change of the created database, DB Browser (SQLite) can be used.
Binaries and tutorial can be found on the: https://sqlitebrowser.org/
Oopening a database, end viewing rows can give clous how database is created, or if something is wrong.
![image](https://github.com/MaciejPotas/Tadaima/assets/33178177/8a34b9cb-f1a9-40b0-a117-8ab931afbeff)

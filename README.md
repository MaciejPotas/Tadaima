# Tadaima!

Tadaima is an application used to help learning vocabulary.
Especially japanese.

## Getting started
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

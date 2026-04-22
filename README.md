# VEDA CHESS ♟️
A Cross-Platform Desktop Chess Game with Network Multiplayer and Persistent Data Management.

VEDA CHESS is a high-performance chess application built with Qt (C++). It evolved from a local-only open-source model into a full-featured multiplayer service featuring real-time LAN discovery, P2P networking, and an SQLite-based player management system.

## 🌟 Key Features
### 1. Advanced Multiplayer System
TCP P2P Connectivity: High-reliability game data transmission for move synchronization.

UDP Broadcast Discovery: Automatically search for and list active game hosts within the same local area network (LAN).

Interactive Lobby: A dynamic matchmaking UI that allows users to host or join rooms seamlessly.

### 2. Persistent Data Management (CRUD)
Integrating SQLite (chess_players.db), we provide a complete account system:

Create: Secure user registration.

Read: Real-time leaderboard and win/loss ratio tracking.

Update: Account verification and password change logic.

Delete: Account termination and complete data erasure.

### 3. Object-Oriented Architecture (OOP)
The project strictly follows OOP principles for scalability:

Polymorphism: Utilizing BasePawnModel as an abstract class to implement diverse movement rules for King, Queen, Rook, Bishop, Knight, and Pawn.

Modular Design: Decoupled Networking (NetworkManager), Database, and UI layers for easier maintenance.

### 4. Modernized UI/UX
Custom-designed VEDA Gradient Logos.

Enhanced visual feedback for piece selection and movement.

High-quality SVG/PNG assets for crisp graphics.

## 🛠 Tech Stack
Language: C++17

Framework: Qt 6 (QGraphicsView Framework)

Database: SQLite 3

Networking: Qt Network (TCP/UDP)

Collaboration: Git (Feature-branch strategy)

## 📸 Screenshots
![](/Images/after_play1.png)
![](/Images/after_play2.png)
![](/Images/after_play3.png)
![](/Images/after_play4.png)
![](/Images/after_play5.png)
![](/Images/after_play6.png)

## 🚀 Getting Started
Prerequisites
Qt Creator (Qt 6.x recommended)

CMake or qmake

Build Instructions
Clone the repository:

Bash
git clone https://github.com/Hun0305/VEDA_CHESS.git
Open ChessGame_223637.pro in Qt Creator.

Build and Run.

## 📘 Appendix. Simultaneous Multi-Instance Building
To test the network multiplayer functionality on a single machine, you need to run multiple instances of the application.

Global Setting for Running Multiple Instances:

Go to the top menu: Edit -> Preferences (or Tools -> Options on some versions).

Select the Build & Run tab -> General sub-tab.

Look for the setting "Stop applications before building" or "Stop applications before running".

Set this to "None".
This allows you to launch a second instance without the first one being closed automatically.

Developed as part of the VEDA 4th Intensive Evaluation (Qt/C++).

### Reference
https://github.com/mdziubich/chessGame_cpp

Rules sources:

https://en.wikipedia.org/wiki/King_(chess) https://en.wikipedia.org/wiki/Queen_(chess) https://en.wikipedia.org/wiki/Rook_(chess) https://pl.wikipedia.org/wiki/Goniec_(szachy) https://en.wikipedia.org/wiki/Knight_(chess) https://en.wikipedia.org/wiki/Pawn_(chess)

https://en.wikipedia.org/wiki/En_passant https://en.wikipedia.org/wiki/Castling https://en.wikipedia.org/wiki/Promotion_(chess) https://en.wikipedia.org/wiki/Opposition_(chess) https://en.wikipedia.org/wiki/Check_(chess) https://en.wikipedia.org/wiki/Checkmate

Images source:

https://www.flaticon.com
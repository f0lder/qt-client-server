# Chat Qt Client-Server

Acest proiect este o aplicație de chat client-server realizată în C++ folosind framework-ul Qt 6.  
Aplicația permite mai multor utilizatori să comunice în timp real, să vadă cine este online, să folosească comenzi speciale și să primească notificări de tip "user is typing".

---

## Pentru binare precompilate

Binare precompilate sunt in tabul de Actions pentru Windows si Linux

__
## Cum a fost creat proiectul

Proiectul a fost dezvoltat pornind de la zero, folosind Qt Creator și CMake pentru build.  
S-au folosit clasele Qt pentru interfață grafică (QWidget, QMainWindow, QVBoxLayout, etc.), pentru rețea (QTcpSocket, QTcpServer) și pentru serializarea mesajelor (QDataStream).

Funcționalitățile principale au fost implementate incremental:
- **Inițial:** conectare client-server, trimitere și afișare mesaje text.
- **Apoi:** autentificare cu username, afișare listă utilizatori online, sistem de comenzi locale (/help, /clear).
- **Ulterior:** indicator de typing, validare și filtrare HTML, sistem de tipuri de mesaje (text, sistem, typing, etc.).

---

## Ce face aplicația

- Permite conectarea mai multor clienți la un server de chat.
- Fiecare utilizator își alege un username la conectare (sau rămâne anonim).
- Mesajele sunt afișate cu timestamp și username.
- Se afișează lista utilizatorilor online.
- Suportă comenzi locale: `/help`, `/clear`.
- Suportă indicator de typing ("X is typing...").
- Permite doar anumite tag-uri HTML sigure în mesaje (`<b>`, `<i>`, `<u>`, `<br>`).
- Serverul gestionează conectarea/deconectarea și transmite mesajele tuturor clienților.

---

## Structura fișierelor

- **client/**
  - **mainwindow.h / mainwindow.cpp**  
    Codul principal al interfeței grafice pentru client.  
    Gestionează UI-ul, conexiunea la server, trimiterea/primirea mesajelor, comenzile locale și indicatorul de typing.
  - **main.cpp**  
    Punctul de intrare pentru aplicația client.
- **server/**
  - **server.h / server.cpp**  
    Codul principal pentru server.  
    Gestionează conexiunile clienților, distribuirea mesajelor, lista de utilizatori și mesajele de sistem.
- **message.h**  
  Definirea structurii unui mesaj (tip, username, text, timestamp) și metodele de serializare/deserializare.
- **README.MD**  
  Acest fișier, cu explicații despre proiect.
- **CMakeLists.txt**  
  Configurare pentru build cu CMake.
- **(opțional) icons/**  
  Iconițe pentru UI (ex: butonul de clear chat).

---

## Cum rulezi proiectul

A fost realizat direct cu CMake, in functie de setup trebuie modificat `CMAKE_PREFIX_PATH` din fisierele  `CMakeLists.txt`. Sau se poate copia doar codul in QTCreator pentru a fi compilat acolo.

---

## Alte detalii

- Poți extinde ușor aplicația cu noi comenzi, teme, emoji, mesaje private etc.
- Codul este comentat și modular pentru a facilita modificările.

---

**Autor:**  
Proiect realizat ca exemplu de aplicație client-server cu Qt.
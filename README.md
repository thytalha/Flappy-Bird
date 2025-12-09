# 🐦 Flappy Bird: The FMT Remix

**Not just another clone. This is the arcade experience evolved.**

***Sample Screenshot of the game is below 👇👇***
![Game Screenshot](sample.png)

---

## 🚀 Ready for Takeoff?

Welcome to the **FMT Studios Edition** of the legendary bird game. We didn't just rebuild it; we upgraded it. Built from scratch in **C++** and **SFML**, this project takes the addictive loop you know and adds the polish you deserve—complete with a killer soundtrack, difficulty modes for every skill level, and a leaderboard that remembers your glory.

## ✨ What Makes This Special?

* 🔥 **Choose Your Pain (Dual Difficulty):**
  * **Easy Mode:** Chill vibes, slower pipes, and forgiving gravity. Perfect for warming up.
  * **Hard Mode:** Fast pipes, heavy gravity, and randomized chaos. Only for the true flappy masters.

* 🎧 **Beats to Fly To:**
  * The music isn't just background noise—it adapts. Chill tracks for Easy mode, intense beats when you switch to Hard.

* 🏆 **The Hall of Fame:**
  * Your high scores aren't lost in the void. We built a **Persistent Leaderboard** system that saves your top 3 runs to `leaderboard.txt`. Prove you're the best.

* 🎨 **Full Arcade UI:**
  * No more abrupt starts. Enjoy a fully animated Intro, a slick Main Menu, customizable Settings (toggle that sound!), and a pause menu for when rage quits happen.

* 🎮 **Play Your Way:**
  * Mouse clicker? Keyboard smasher? We support both.

## 🕹️ Mission Controls

Seamless input switching means you can play however you want.

| Action | Keyboard | Mouse |
| :--- | :--- | :--- |
| **Flap Wings** | `Spacebar` | `Left Click` |
| **Start / Confirm** | `Spacebar` / `Enter` | `Left Click` |
| **Navigate Menus** | `Up` / `Down` Arrows | Hover Cursor |
| **Emergency Pause** | `Escape` | (Click Pause Buttons) |

## 📂 Getting Started

To get this bird in the air, you'll need the **SFML Library** (2.5.x or newer) linked to your C++ environment.

### The Asset Check

The game is smart, but it needs its resources. Make sure your folder looks exactly like this, or the bird won't fly:

```text
/Project-Root
  ├── FlappyBird.exe
  ├── leaderboard.txt    (Auto-generated upon first glory)
  └── /assets
       ├── bg.png
       ├── birdup.png / birddown.png
       ├── pipedown.png / pipeup.png
       ├── arial.ttf
       ├── music_easy.mp3 / music_hard.mp3
       ├── flap.wav / score.wav / dead.wav
       ├── (Menu Buttons: mainnewgame.png, settings.png, etc.)
       └── (Intro Frames: intro1.gif ... intro19.gif)
```
## 👨‍💻 The Crew
### Brought to you by FMT Studios. Powered by C++ & SFML.

---

# 📜 License & Usage
## This project is proudly created by FMT Studios.

We believe in open development, so you are completely free to use, modify, and remix this game. Whether you want to learn from the code, build new features on top of it, or just mess around with the physics—go for it! The code is yours to explore.

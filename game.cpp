#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib> 
#include <ctime>   
#include <iostream>
#include <fstream>

using namespace std;
using namespace sf;

const int width = 864;
const int height = 512;
int pipe_gap = 150;
int pipe_interval = 250;
const int ground_height = 112;
const int min_gap_y = 50;
const int max_pipes = 20;

float gravity = 0.30f;
float flap_strength = -5.0f;
float pipe_speed = -2.8f;

struct PipePair {
    Sprite top;
    Sprite bottom;
    bool scored = false;
};

enum GameState {
    INTRO,
    MAIN_MENU,
    SETTINGS_MENU,
    DIFFICULTY_MENU,
    LEADERBOARD_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum Difficulty {
    EASY,
    HARD
};

struct Button {
    Sprite sprite;
    Texture texture;
    bool isVisible = true;
};

GameState game_state = INTRO;
Difficulty difficulty_level = EASY;
int selected_menu = -1;
float bird_vel = 0.f;
int score = 0;
int leaderboard[3] = { 0, 0, 0 };
bool game_started = false;
bool sound_on = true;
bool music_on = true;
bool next_gap_high = true;

Texture bg_tex, bird_up_tex, bird_down_tex, pipe_down_tex, pipe_up_tex, intro_tex[19];
Font game_font;
SoundBuffer flap_buf, score_buf, dead_buf;
Sound flap_sound, score_sound, dead_sound;
Music bg_music, intro_music;

Sprite background, bird;
PipePair pipes[max_pipes];
int pipe_count = 0;

Sprite intro_sprite;
int intro_frame = 0;
float intro_time = 0.f;
const int intro_frame_count = 19;
const float intro_frame_duration = 0.40f;

Button btn_new_game, btn_settings, btn_leaderboard, btn_exit;
Button btn_difficulty, btn_sound, btn_music, btn_back_settings;
Button btn_easy, btn_hard, btn_back_difficulty;
Button btn_resume, btn_restart, btn_main_menu, btn_exit_pause;
Button btn_play_again, btn_main_over, btn_exit_over;
Button btn_back_leaderboard;
Button btn_easy_indicator, btn_hard_indicator;

Text score_text, leaderboard_text, title_text;

float min_f(float a, float b);
int get_max_gap_y();
void int_to_string(int num, char buffer[]);

void apply_difficulty();
void update_music_for_difficulty();

void load_leaderboard();
void save_leaderboard();
void update_leaderboard(int new_score);

bool load_button(Button& btn, const char filename[]);
bool load_all_assets();
void setup_background();
void setup_bird();
void setup_text();
void setup_button_positions();
void setup_all();
void reset_game();

bool is_button_hovered(const Button& btn, const Vector2f& mouse_pos);
void highlight_button(Button& btn, bool is_highlighted);

void handle_main_menu_input(const Event& ev, RenderWindow& window);
void handle_settings_input(const Event& ev, RenderWindow& window);
void handle_difficulty_input(const Event& ev, RenderWindow& window);
void handle_leaderboard_input(const Event& ev, RenderWindow& window);
void handle_pause_input(const Event& ev, RenderWindow& window);
void handle_game_over_input(const Event& ev, RenderWindow& window);
void handle_playing_input(const Event& ev);
void handle_intro_input(const Event& ev);
void handle_events(RenderWindow& window);

void      update_bird();
void      spawn_pipes();
void      move_pipes();
void      remove_old_pipes();
void      update_scoring();
void      update_pipes();
FloatRect get_bird_box(float shrink_x = 10.f, float shrink_y = 10.f);
FloatRect get_pipe_box(const Sprite& pipe, float shrink_x = 5.f);
void      check_collision();

void update_game(float dt);
void draw_background(RenderWindow& window);
void draw_main_menu(RenderWindow& window);
void draw_settings(RenderWindow& window);
void draw_difficulty(RenderWindow& window);
void draw_leaderboard(RenderWindow& window);
void draw_pause(RenderWindow& window);
void draw_game_over(RenderWindow& window);
void draw_pipes(RenderWindow& window);
void draw_score(RenderWindow& window);
void draw_game(RenderWindow& window);
void draw(RenderWindow& window);

bool init_game();
void run_game(RenderWindow& window);

int main() {
    RenderWindow window(VideoMode(width, height), "Flappy Bird - FMT Studios");
    window.setFramerateLimit(60);

    if (!init_game()) {
        return 1;
    }
    run_game(window);
    return 0;
}

float min_f(float a, float b) {
    return (a < b) ? a : b;
}

int get_max_gap_y() {
    return height - ground_height - pipe_gap - 25;
}

void int_to_string(int num, char buffer[]) {
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    int i = 0;
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    if (is_negative) {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

void apply_difficulty() {
    switch (difficulty_level) {
    case EASY:
        pipe_gap = 150;
        gravity = 0.30f;
        flap_strength = -7.0f;
        pipe_speed = -2.8f;
        pipe_interval = 250;
        break;
    case HARD:
        pipe_gap = 110;
        gravity = 0.55f;
        flap_strength = -8.5f;
        pipe_speed = -4.8f;
        pipe_interval = 180;
        break;
    }
}

void update_music_for_difficulty() {
    if (difficulty_level == EASY) {
        if (bg_music.openFromFile("assets/music_easy.mp3")) {
            bg_music.setLoop(true);
            if (music_on && game_started) bg_music.play();
        }
    }
    else {
        if (bg_music.openFromFile("assets/music_hard.mp3")) {
            bg_music.setLoop(true);
            if (music_on && game_started) bg_music.play();
        }
    }
}

void load_leaderboard() {
    ifstream in("leaderboard.txt");
    if (in.is_open()) {
        for (int i = 0; i < 3; i++) {
            if (!(in >> leaderboard[i])) {
                leaderboard[i] = 0;
            }
        }
        in.close();
    }
}

void save_leaderboard() {
    ofstream out("leaderboard.txt");
    if (out.is_open()) {
        for (int i = 0; i < 3; i++) {
            out << leaderboard[i] << "\n";
        }
        out.close();
    }
}

void update_leaderboard(int new_score) {
    int temp[4];
    for (int i = 0; i < 3; i++) {
        temp[i] = leaderboard[i];
    }
    temp[3] = new_score;

    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (temp[i] < temp[j]) {
                int swap = temp[i];
                temp[i] = temp[j];
                temp[j] = swap;
            }
        }
    }
    for (int i = 0; i < 3; i++) {
        leaderboard[i] = temp[i];
    }
    save_leaderboard();
}

bool load_button(Button& btn, const char filename[]) {
    char path[256] = "assets/";
    int i = 7;
    int j = 0;
    while (filename[j] != '\0') {
        path[i++] = filename[j++];
    }
    path[i] = '\0';

    if (!btn.texture.loadFromFile(path)) {
        cout << "Failed to load " << path << endl;
        return false;
    }
    btn.sprite.setTexture(btn.texture);
    return true;
}

bool load_all_assets() {
    char bg_path[] = "assets/bg.png";
    char bird_up_path[] = "assets/birdup.png";
    char bird_down_path[] = "assets/birddown.png";
    char pipe_down_path[] = "assets/pipedown.png";
    char pipe_up_path[] = "assets/pipeup.png";

    if (!bg_tex.loadFromFile(bg_path) ||
        !bird_up_tex.loadFromFile(bird_up_path) ||
        !bird_down_tex.loadFromFile(bird_down_path) ||
        !pipe_down_tex.loadFromFile(pipe_down_path) ||
        !pipe_up_tex.loadFromFile(pipe_up_path)) {
        return false;
    }

    if (!game_font.loadFromFile("assets/arial.ttf")) {
        cout << "Failed to load font" << endl;
        return false;
    }

    for (int i = 0; i < intro_frame_count; ++i) {
        int frame_number = i + 1;
        char num_buf[8];
        int_to_string(frame_number, num_buf);

        char path[64] = "assets/intro";
        int p = 11;
        int k = 0;
        while (num_buf[k] != '\0') {
            path[p++] = num_buf[k++];
        }
        path[p++] = '.';
        path[p++] = 'g';
        path[p++] = 'i';
        path[p++] = 'f';
        path[p] = '\0';

        if (!intro_tex[i].loadFromFile(path)) {
            cout << "Failed to load " << path << endl;
        }
    }

    if (intro_tex[0].getSize().x > 0) {
        intro_sprite.setTexture(intro_tex[0]);
        intro_sprite.setPosition(0.f, 0.f);
        float sx = (float)width / intro_tex[0].getSize().x;
        float sy = (float)height / intro_tex[0].getSize().y;
        intro_sprite.setScale(sx, sy);
    }

    char flap_path[] = "assets/flap.wav";
    char score_path[] = "assets/score.wav";
    char dead_path[] = "assets/dead.wav";

    if (!flap_buf.loadFromFile(flap_path) ||
        !score_buf.loadFromFile(score_path) ||
        !dead_buf.loadFromFile(dead_path)) {
        return false;
    }

    flap_sound.setBuffer(flap_buf);
    score_sound.setBuffer(score_buf);
    dead_sound.setBuffer(dead_buf);

    if (!load_button(btn_new_game, "mainnewgame.png") ||
        !load_button(btn_settings, "settings.png") ||
        !load_button(btn_leaderboard, "leaderboard.png") ||
        !load_button(btn_exit, "mainexit.png")) {
        return false;
    }
    if (!load_button(btn_difficulty, "difficulty.png") ||
        !load_button(btn_sound, sound_on ? "soundon.png" : "soundoff.png") ||
        !load_button(btn_music, music_on ? "musicon.png" : "musicoff.png") ||
        !load_button(btn_back_settings, "backbutton.png")) {
        return false;
    }
    if (!load_button(btn_easy, "easy.png") ||
        !load_button(btn_hard, "hard.png") ||
        !load_button(btn_back_difficulty, "backbutton.png")) {
        return false;
    }
    if (!load_button(btn_resume, "pauseresume.png") ||
        !load_button(btn_restart, "pauserestart.png") ||
        !load_button(btn_main_menu, "pausemain.png") ||
        !load_button(btn_exit_pause, "pauseexit.png")) {
        return false;
    }
    if (!load_button(btn_play_again, "overagain.png") ||
        !load_button(btn_main_over, "overmain.png") ||
        !load_button(btn_exit_over, "overexit.png")) {
        return false;
    }
    if (!load_button(btn_back_leaderboard, "backbutton.png")) {
        return false;
    }
    if (!load_button(btn_easy_indicator, "easy.png") ||
        !load_button(btn_hard_indicator, "hard.png")) {
        return false;
    }

    if (!intro_music.openFromFile("assets/animationsound.mp3")) {
        cout << "Failed to load animationsound.mp3" << endl;
    }
    else {
        intro_music.setLoop(false);
    }

    return true;
}

void setup_background() {
    background.setTexture(bg_tex);
    background.setPosition(0.f, 0.f);
    float scale_x = static_cast<float>(width) / bg_tex.getSize().x;
    float scale_y = static_cast<float>(height) / bg_tex.getSize().y;
    background.setScale(scale_x, scale_y);
}

void setup_bird() {
    bird.setTexture(bird_up_tex);
    bird.setOrigin(bird.getLocalBounds().width / 2.f, bird.getLocalBounds().height / 2.f);
    bird.setPosition(120, height / 2);
}

void setup_text() {
    score_text = Text("", game_font, 64);
    score_text.setFillColor(Color::White);
    score_text.setOutlineColor(Color::Black);
    score_text.setOutlineThickness(4);
    score_text.setPosition(width / 2 - 40, 20);

    leaderboard_text = Text("", game_font, 48);
    leaderboard_text.setFillColor(Color::White);
    leaderboard_text.setOutlineColor(Color::Black);
    leaderboard_text.setOutlineThickness(3);

    title_text = Text("Flappy Bird by FMT Studios", game_font, 48);
    title_text.setFillColor(Color(0, 51, 102));
    title_text.setStyle(Text::Bold);
    FloatRect title_bounds = title_text.getLocalBounds();
    title_text.setPosition((width - title_bounds.width) / 2, 80);
}

void setup_button_positions() {
    float start_y = 180;
    float spacing = 80;
    btn_new_game.sprite.setPosition(width / 2 - btn_new_game.sprite.getGlobalBounds().width / 2, start_y);
    btn_settings.sprite.setPosition(width / 2 - btn_settings.sprite.getGlobalBounds().width / 2, start_y + spacing);
    btn_leaderboard.sprite.setPosition(width / 2 - btn_leaderboard.sprite.getGlobalBounds().width / 2, start_y + spacing * 2);
    btn_exit.sprite.setPosition(width / 2 - btn_exit.sprite.getGlobalBounds().width / 2, start_y + spacing * 3);

    start_y = 120;
    spacing = 70;
    btn_difficulty.sprite.setPosition(width / 2 - btn_difficulty.sprite.getGlobalBounds().width / 2, start_y);
    btn_sound.sprite.setPosition(width / 2 - btn_sound.sprite.getGlobalBounds().width / 2, start_y + spacing);
    btn_music.sprite.setPosition(width / 2 - btn_music.sprite.getGlobalBounds().width / 2, start_y + spacing * 2);
    btn_back_settings.sprite.setPosition(width / 2 - btn_back_settings.sprite.getGlobalBounds().width / 2, start_y + spacing * 3);

    float indicator_scale = 0.6f;
    btn_easy_indicator.sprite.setScale(indicator_scale, indicator_scale);
    btn_hard_indicator.sprite.setScale(indicator_scale, indicator_scale);

    float indicator_x = btn_difficulty.sprite.getPosition().x + btn_difficulty.sprite.getGlobalBounds().width + 30;
    btn_easy_indicator.sprite.setPosition(indicator_x, (btn_difficulty.sprite.getPosition().y + 8));
    btn_hard_indicator.sprite.setPosition(indicator_x, btn_difficulty.sprite.getPosition().y + 8);

    start_y = 180;
    spacing = 80;
    btn_easy.sprite.setPosition(width / 2 - btn_easy.sprite.getGlobalBounds().width / 2, start_y);
    btn_hard.sprite.setPosition(width / 2 - btn_hard.sprite.getGlobalBounds().width / 2, start_y + spacing);
    btn_back_difficulty.sprite.setPosition(width / 2 - btn_back_difficulty.sprite.getGlobalBounds().width / 2, start_y + spacing * 2);

    start_y = 140;
    spacing = 70;
    btn_resume.sprite.setPosition(width / 2 - btn_resume.sprite.getGlobalBounds().width / 2, start_y);
    btn_restart.sprite.setPosition(width / 2 - btn_restart.sprite.getGlobalBounds().width / 2, start_y + spacing);
    btn_main_menu.sprite.setPosition(width / 2 - btn_main_menu.sprite.getGlobalBounds().width / 2, start_y + spacing * 2);
    btn_exit_pause.sprite.setPosition(width / 2 - btn_exit_pause.sprite.getGlobalBounds().width / 2, start_y + spacing * 3);

    start_y = 200;
    spacing = 80;
    btn_play_again.sprite.setPosition(width / 2 - btn_play_again.sprite.getGlobalBounds().width / 2, start_y);
    btn_main_over.sprite.setPosition(width / 2 - btn_main_over.sprite.getGlobalBounds().width / 2, start_y + spacing);
    btn_exit_over.sprite.setPosition(width / 2 - btn_exit_over.sprite.getGlobalBounds().width / 2, start_y + spacing * 2);
    btn_back_leaderboard.sprite.setPosition(width / 2 - btn_back_leaderboard.sprite.getGlobalBounds().width / 2, height - 100);
}

void setup_all() {
    setup_background();
    setup_bird();
    setup_text();
    setup_button_positions();
}

void reset_game() {
    game_state = PLAYING;
    game_started = false;
    score = 0;
    pipe_count = 0;
    bird.setPosition(120, height / 2);
    bird.setRotation(0);
    bird_vel = 0;
    selected_menu = -1;
    next_gap_high = true;
    apply_difficulty();
    bg_music.stop();
}

bool is_button_hovered(const Button& btn, const Vector2f& mouse_pos) {
    return btn.isVisible && btn.sprite.getGlobalBounds().contains(mouse_pos);
}

void highlight_button(Button& btn, bool is_highlighted) {
    if (is_highlighted) {
        btn.sprite.setColor(Color::Yellow);
    }
    else {
        btn.sprite.setColor(Color::White);
    }
}


void handle_main_menu_input(const Event& ev, RenderWindow& window) {
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Down) {
            selected_menu = (selected_menu + 1) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Up) {
            selected_menu = (selected_menu - 1 + 4) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Enter && selected_menu >= 0) {
            if (selected_menu == 0) reset_game();
            else if (selected_menu == 1) { game_state = SETTINGS_MENU; selected_menu = -1; }
            else if (selected_menu == 2) { game_state = LEADERBOARD_MENU; selected_menu = -1; }
            else if (selected_menu == 3) window.close();
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_new_game, mouse_pos)) reset_game();
        else if (is_button_hovered(btn_settings, mouse_pos)) { game_state = SETTINGS_MENU; selected_menu = -1; }
        else if (is_button_hovered(btn_leaderboard, mouse_pos)) { game_state = LEADERBOARD_MENU; selected_menu = -1; }
        else if (is_button_hovered(btn_exit, mouse_pos)) window.close();
    }
}

void handle_settings_input(const Event& ev, RenderWindow& window) {
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Down) {
            selected_menu = (selected_menu + 1) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Up) {
            selected_menu = (selected_menu - 1 + 4) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Enter && selected_menu >= 0) {
            if (selected_menu == 0) { game_state = DIFFICULTY_MENU; selected_menu = -1; }
            else if (selected_menu == 1) {
                sound_on = !sound_on;
                load_button(btn_sound, sound_on ? "soundon.png" : "soundoff.png");
                setup_button_positions();
            }
            else if (selected_menu == 2) {
                music_on = !music_on;
                load_button(btn_music, music_on ? "musicon.png" : "musicoff.png");
                setup_button_positions();
                if (music_on) {
                    if (game_started && bg_music.getStatus() != Music::Playing)
                        bg_music.play();
                }
                else {
                    bg_music.pause();
                }
            }
            else if (selected_menu == 3) { game_state = MAIN_MENU; selected_menu = -1; }
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_difficulty, mouse_pos)) { game_state = DIFFICULTY_MENU; selected_menu = -1; }
        else if (is_button_hovered(btn_sound, mouse_pos)) {
            sound_on = !sound_on;
            load_button(btn_sound, sound_on ? "soundon.png" : "soundoff.png");
            setup_button_positions();
        }
        else if (is_button_hovered(btn_music, mouse_pos)) {
            music_on = !music_on;
            load_button(btn_music, music_on ? "musicon.png" : "musicoff.png");
            setup_button_positions();
            if (music_on) {
                if (game_started && bg_music.getStatus() != Music::Playing)
                    bg_music.play();
            }
            else {
                bg_music.pause();
            }
        }
        else if (is_button_hovered(btn_back_settings, mouse_pos)) { game_state = MAIN_MENU; selected_menu = -1; }
    }
}

void handle_difficulty_input(const Event& ev, RenderWindow& window) {
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Down) {
            selected_menu = (selected_menu + 1) % 3;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Up) {
            selected_menu = (selected_menu - 1 + 3) % 3;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Enter && selected_menu >= 0) {
            if (selected_menu == 0) {
                difficulty_level = EASY;
                apply_difficulty();
                update_music_for_difficulty();
            }
            else if (selected_menu == 1) {
                difficulty_level = HARD;
                apply_difficulty();
                update_music_for_difficulty();
            }
            else if (selected_menu == 2) { game_state = SETTINGS_MENU; selected_menu = -1; }
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_easy, mouse_pos)) {
            difficulty_level = EASY;
            apply_difficulty();
            update_music_for_difficulty();
        }
        else if (is_button_hovered(btn_hard, mouse_pos)) {
            difficulty_level = HARD;
            apply_difficulty();
            update_music_for_difficulty();
        }
        else if (is_button_hovered(btn_back_difficulty, mouse_pos)) { game_state = SETTINGS_MENU; selected_menu = -1; }
    }
}

void handle_leaderboard_input(const Event& ev, RenderWindow& window) {
    if (ev.type == Event::KeyPressed && ev.key.code == Keyboard::Enter) {
        game_state = MAIN_MENU;
        selected_menu = -1;
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_back_leaderboard, mouse_pos)) {
            game_state = MAIN_MENU;
            selected_menu = -1;
        }
    }
}

void handle_pause_input(const Event& ev, RenderWindow& window) {
    bg_music.pause();
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Down) {
            selected_menu = (selected_menu + 1) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Up) {
            selected_menu = (selected_menu - 1 + 4) % 4;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Enter && selected_menu >= 0) {
            if (selected_menu == 0) { game_state = PLAYING; bg_music.play(); selected_menu = -1; }
            else if (selected_menu == 1) reset_game();
            else if (selected_menu == 2) { game_state = MAIN_MENU; selected_menu = -1; }
            else if (selected_menu == 3) window.close();
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_resume, mouse_pos)) { game_state = PLAYING; bg_music.play(); selected_menu = -1; }
        else if (is_button_hovered(btn_restart, mouse_pos)) reset_game();
        else if (is_button_hovered(btn_main_menu, mouse_pos)) { game_state = MAIN_MENU; selected_menu = -1; }
        else if (is_button_hovered(btn_exit_pause, mouse_pos)) window.close();
    }
}

void handle_game_over_input(const Event& ev, RenderWindow& window) {
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Down) {
            selected_menu = (selected_menu + 1) % 3;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Up) {
            selected_menu = (selected_menu - 1 + 3) % 3;
            if (sound_on) flap_sound.play();
        }
        else if (ev.key.code == Keyboard::Enter && selected_menu >= 0) {
            if (selected_menu == 0) reset_game();
            else if (selected_menu == 1) { game_state = MAIN_MENU; selected_menu = -1; }
            else if (selected_menu == 2) window.close();
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        Vector2f mouse_pos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (is_button_hovered(btn_play_again, mouse_pos)) reset_game();
        else if (is_button_hovered(btn_main_over, mouse_pos)) { game_state = MAIN_MENU; selected_menu = -1; }
        else if (is_button_hovered(btn_exit_over, mouse_pos)) window.close();
    }
}

void handle_playing_input(const Event& ev) {
    if (ev.type == Event::KeyPressed) {
        if (ev.key.code == Keyboard::Escape) {
            game_state = PAUSED;
            selected_menu = -1;
            return;
        }
        if (ev.key.code == Keyboard::Space) {
            if (!game_started) {
                game_started = true;
                update_music_for_difficulty();
            }
            bird_vel = flap_strength;
            if (sound_on) flap_sound.play();
            bird.setTexture(bird_up_tex);
        }
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        if (!game_started) {
            game_started = true;
            update_music_for_difficulty();
        }
        bird_vel = flap_strength;
        if (sound_on) flap_sound.play();
        bird.setTexture(bird_up_tex);
    }
}

void handle_intro_input(const Event& ev) {
    if (ev.type == Event::KeyPressed) {
        intro_music.stop();
        game_state = MAIN_MENU;
    }
    if (ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
        intro_music.stop();
        game_state = MAIN_MENU;
    }
}

void handle_events(RenderWindow& window) {
    Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == Event::Closed) {
            window.close();
        }

        switch (game_state) {
        case INTRO:
            handle_intro_input(ev);
            break;
        case MAIN_MENU:
            handle_main_menu_input(ev, window);
            break;
        case SETTINGS_MENU:
            handle_settings_input(ev, window);
            break;
        case DIFFICULTY_MENU:
            handle_difficulty_input(ev, window);
            break;
        case LEADERBOARD_MENU:
            handle_leaderboard_input(ev, window);
            break;
        case PLAYING:
            handle_playing_input(ev);
            break;
        case PAUSED:
            handle_pause_input(ev, window);
            break;
        case GAME_OVER:
            handle_game_over_input(ev, window);
            break;
        }
    }
}

void update_bird() {
    bird_vel += gravity;
    bird.move(0, bird_vel);
    bird.setRotation(min_f(bird_vel * 4.f, 90.f));
    if (bird_vel > 0) {
        bird.setTexture(bird_down_tex);
    }
}

void spawn_pipes() {
    if (pipe_count == 0 || pipes[pipe_count - 1].top.getPosition().x <= width - pipe_interval) {
        if (pipe_count < max_pipes) {
            int max_gap = get_max_gap_y();
            int gap_y;

            if (difficulty_level == HARD) {
                if (next_gap_high) {
                    gap_y = min_gap_y + (max_gap - min_gap_y) / 4;
                }
                else {
                    gap_y = min_gap_y + (3 * (max_gap - min_gap_y)) / 4;
                }
                next_gap_high = !next_gap_high;
            }
            else {
                gap_y = min_gap_y + rand() % (max_gap - min_gap_y + 1);
            }

            pipes[pipe_count].top.setTexture(pipe_down_tex);
            pipes[pipe_count].top.setPosition(width, 0);
            pipes[pipe_count].top.setScale(1.5f, gap_y / float(pipe_down_tex.getSize().y));

            float bottom_y = gap_y + pipe_gap;
            float bottom_h = height - bottom_y;
            pipes[pipe_count].bottom.setTexture(pipe_up_tex);
            pipes[pipe_count].bottom.setPosition(width, bottom_y);
            pipes[pipe_count].bottom.setScale(1.5f, bottom_h / float(pipe_up_tex.getSize().y));
            pipes[pipe_count].scored = false;
            ++pipe_count;
        }
    }
}

void move_pipes() {
    for (int i = 0; i < pipe_count; ++i) {
        pipes[i].top.move(pipe_speed, 0);
        pipes[i].bottom.move(pipe_speed, 0);
    }
}

void remove_old_pipes() {
    if (pipe_count > 0 && pipes[0].top.getPosition().x + pipes[0].top.getGlobalBounds().width < 0) {
        for (int i = 0; i < pipe_count - 1; ++i) {
            pipes[i] = pipes[i + 1];
        }
        --pipe_count;
    }
}

void update_scoring() {
    for (int i = 0; i < pipe_count; ++i) {
        if (!pipes[i].scored && pipes[i].top.getPosition().x + pipes[i].top.getGlobalBounds().width < bird.getPosition().x) {
            pipes[i].scored = true;
            ++score;
            if (sound_on) score_sound.play();
        }
    }
}

void update_pipes() {
    spawn_pipes();
    move_pipes();
    remove_old_pipes();
    update_scoring();
}

FloatRect get_bird_box(float shrink_x, float shrink_y) {
    FloatRect bounds = bird.getGlobalBounds();
    return FloatRect(
        bounds.left + shrink_x,
        bounds.top + shrink_y,
        bounds.width - (shrink_x * 2),
        bounds.height - (shrink_y * 2)
    );
}

FloatRect get_pipe_box(const Sprite& pipe, float shrink_x) {
    FloatRect bounds = pipe.getGlobalBounds();
    return FloatRect(
        bounds.left + shrink_x,
        bounds.top,
        bounds.width - (shrink_x * 2),
        bounds.height
    );
}

void check_collision() {
    FloatRect bird_box = get_bird_box(10.f, 10.f);
    bool died = (bird_box.top < 0) || (bird_box.top + bird_box.height > height);

    if (!died) {
        for (int i = 0; i < pipe_count; ++i) {
            FloatRect top_box = get_pipe_box(pipes[i].top, 5.f);
            FloatRect bottom_box = get_pipe_box(pipes[i].bottom, 5.f);
            if (bird_box.intersects(top_box) || bird_box.intersects(bottom_box)) {
                died = true;
                break;
            }
        }
    }
    if (died) {
        if (sound_on) dead_sound.play();
        update_leaderboard(score);
        game_state = GAME_OVER;
        game_started = false;
        bg_music.stop();
        selected_menu = -1;
    }
}

void update_game(float dt) {
    if (game_state == INTRO) {
        if (intro_music.getStatus() != Music::Playing) {
            intro_music.play();
        }
        intro_time += dt;
        if (intro_time >= intro_frame_duration) {
            intro_time = 0.f;
            intro_frame++;
            if (intro_frame >= intro_frame_count) {
                intro_music.stop();
                game_state = MAIN_MENU;
            }
            else {
                if (intro_tex[intro_frame].getSize().x > 0) {
                    intro_sprite.setTexture(intro_tex[intro_frame]);
                    float sx = (float)width / intro_tex[intro_frame].getSize().x;
                    float sy = (float)height / intro_tex[intro_frame].getSize().y;
                    intro_sprite.setScale(sx, sy);
                }
            }
        }
    }
    else if (game_state == PLAYING && game_started) {
        update_bird();
        update_pipes();
        check_collision();
    }
}

void draw_background(RenderWindow& window) {
    window.draw(background);
}

void draw_main_menu(RenderWindow& window) {
    window.draw(title_text);

    highlight_button(btn_new_game, selected_menu == 0);
    highlight_button(btn_settings, selected_menu == 1);
    highlight_button(btn_leaderboard, selected_menu == 2);
    highlight_button(btn_exit, selected_menu == 3);

    window.draw(btn_new_game.sprite);
    window.draw(btn_settings.sprite);
    window.draw(btn_leaderboard.sprite);
    window.draw(btn_exit.sprite);
}

void draw_settings(RenderWindow& window) {
    highlight_button(btn_difficulty, selected_menu == 0);
    highlight_button(btn_sound, selected_menu == 1);
    highlight_button(btn_music, selected_menu == 2);
    highlight_button(btn_back_settings, selected_menu == 3);

    window.draw(btn_difficulty.sprite);
    window.draw(btn_sound.sprite);
    window.draw(btn_music.sprite);
    window.draw(btn_back_settings.sprite);

    if (difficulty_level == EASY) {
        window.draw(btn_easy_indicator.sprite);
    }
    else {
        window.draw(btn_hard_indicator.sprite);
    }
}

void draw_difficulty(RenderWindow& window) {
    highlight_button(btn_easy, selected_menu == 0 || (selected_menu == -1 && difficulty_level == EASY));
    highlight_button(btn_hard, selected_menu == 1 || (selected_menu == -1 && difficulty_level == HARD));
    highlight_button(btn_back_difficulty, selected_menu == 2);

    window.draw(btn_easy.sprite);
    window.draw(btn_hard.sprite);
    window.draw(btn_back_difficulty.sprite);
}

void draw_leaderboard(RenderWindow& window) {
    Text title("LEADERBOARD", game_font, 60);
    title.setFillColor(Color::Yellow);
    title.setOutlineColor(Color::Black);
    title.setOutlineThickness(4);
    FloatRect bounds = title.getLocalBounds();
    title.setPosition((width - bounds.width) / 2, 80);
    window.draw(title);

    char buffer[100];
    for (int i = 0; i < 3; i++) {
        buffer[0] = '1' + i;
        buffer[1] = '.';
        buffer[2] = ' ';
        int_to_string(leaderboard[i], buffer + 3);

        Text score_display(buffer, game_font, 48);
        score_display.setFillColor(Color::White);
        score_display.setOutlineColor(Color::Black);
        score_display.setOutlineThickness(3);
        FloatRect sb = score_display.getLocalBounds();
        score_display.setPosition((width - sb.width) / 2, 200 + i * 70);
        window.draw(score_display);
    }
    highlight_button(btn_back_leaderboard, false);
    window.draw(btn_back_leaderboard.sprite);
}

void draw_pause(RenderWindow& window) {
    RectangleShape overlay(Vector2f(width, height));
    overlay.setFillColor(Color(0, 0, 0, 150));
    window.draw(overlay);

    highlight_button(btn_resume, selected_menu == 0);
    highlight_button(btn_restart, selected_menu == 1);
    highlight_button(btn_main_menu, selected_menu == 2);
    highlight_button(btn_exit_pause, selected_menu == 3);

    window.draw(btn_resume.sprite);
    window.draw(btn_restart.sprite);
    window.draw(btn_main_menu.sprite);
    window.draw(btn_exit_pause.sprite);
}

void draw_game_over(RenderWindow& window) {
    Text game_over_title("GAME OVER", game_font, 72);
    game_over_title.setFillColor(Color::Red);
    game_over_title.setOutlineColor(Color::Black);
    game_over_title.setOutlineThickness(5);
    FloatRect bounds = game_over_title.getLocalBounds();
    game_over_title.setPosition((width - bounds.width) / 2, 40);
    window.draw(game_over_title);

    char score_buffer[50];
    score_buffer[0] = 'S';
    score_buffer[1] = 'c';
    score_buffer[2] = 'o';
    score_buffer[3] = 'r';
    score_buffer[4] = 'e';
    score_buffer[5] = ':';
    score_buffer[6] = ' ';
    int_to_string(score, score_buffer + 7);

    Text final_score_text(score_buffer, game_font, 40);
    final_score_text.setFillColor(Color::White);
    final_score_text.setOutlineColor(Color::Black);
    final_score_text.setOutlineThickness(3);
    FloatRect score_bounds = final_score_text.getLocalBounds();
    final_score_text.setPosition(
        (width - score_bounds.width) / 2,
        40 + bounds.height + 20
    );
    window.draw(final_score_text);

    highlight_button(btn_play_again, selected_menu == 0);
    highlight_button(btn_main_over, selected_menu == 1);
    highlight_button(btn_exit_over, selected_menu == 2);

    window.draw(btn_play_again.sprite);
    window.draw(btn_main_over.sprite);
    window.draw(btn_exit_over.sprite);
}

void draw_pipes(RenderWindow& window) {
    for (int i = 0; i < pipe_count; ++i) {
        window.draw(pipes[i].top);
        window.draw(pipes[i].bottom);
    }
}

void draw_score(RenderWindow& window) {
    if (game_state == PLAYING) {
        char score_str[20];
        int_to_string(score, score_str);
        score_text.setString(score_str);
        window.draw(score_text);
    }
}

void draw_game(RenderWindow& window) {
    draw_pipes(window);
    window.draw(bird);
    draw_score(window);

    if (!game_started && game_state == PLAYING) {
        Text start_text("PRESS SPACE OR CLICK TO START", game_font, 32);
        start_text.setFillColor(Color::White);
        start_text.setOutlineColor(Color::Black);
        start_text.setOutlineThickness(3);
        FloatRect bounds = start_text.getLocalBounds();
        start_text.setPosition((width - bounds.width) / 2, height / 2 - 50);
        window.draw(start_text);
    }
}

void draw(RenderWindow& window) {
    window.clear();

    if (game_state == INTRO) {
        window.draw(intro_sprite);
        window.display();
        return;
    }

    draw_background(window);

    switch (game_state) {
    case MAIN_MENU:
        draw_main_menu(window);
        break;
    case SETTINGS_MENU:
        draw_settings(window);
        break;
    case DIFFICULTY_MENU:
        draw_difficulty(window);
        break;
    case LEADERBOARD_MENU:
        draw_leaderboard(window);
        break;
    case PLAYING:
        draw_game(window);
        break;
    case PAUSED:
        draw_game(window);
        draw_pause(window);
        break;
    case GAME_OVER:
        draw_game(window);
        draw_game_over(window);
        break;
    default:
        break;
    }
    window.display();
}

bool init_game() {
    srand((unsigned)time(NULL));
    if (!load_all_assets()) {
        cerr << "Game initialization failed due to asset loading error." << endl;
        return false;
    }
    setup_all();
    load_leaderboard();
    apply_difficulty();
    return true;
}

void run_game(RenderWindow& window) {
    Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        handle_events(window);
        update_game(dt);
        draw(window);
    }
}
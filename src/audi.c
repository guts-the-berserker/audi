/*

Copyright (c) 2025 GutsTheBerserker

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

// Author: GutsTheBerserker
// audi: a minimalistic audio player
// Libraries used: SDL2, SDL2_Mixer, ncurses

// Current Version: 1.0

/*

NOTE: This was my first C project
might contain some bugs or
memory leaks, sorry for that!

*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <json-c/json.h>
#include <assert.h>

typedef enum PlayerReturnCode
{
    SUCCESS,
    // Errors
    ERR_JSON_INVALID_FORMAT,
    ERR_UNABLE_TO_READ,
    ERR_MEM_ALLOC,
    ERR_COULD_NOT_PLAY,
    ERR_COULD_NOT_OPEN_DIR,
    ERR_UNABLE_TO_OPEN_ORDER_FILE,

    // Player
    PLAYER_NEXT,
    PLAYER_BACK,
    PLAYER_QUIT
} RCode;

char *err_ref_file = NULL;
bool error_reference = false;

void set_err_ref_file(const char *name)
{
    if (!error_reference)
    {
        err_ref_file = strdup(name);
        error_reference = true;
    }
}

void free_err_file_ref()
{
    if (error_reference)
    {
        free(err_ref_file);
        err_ref_file = NULL;
        error_reference = false;
    }
}

// Prints help message
void print_help(void)
{
    printf("audi - a minimalistic terminal based audio player (SDL2_Mixer v2.8.0)\n\
current version: v1.0\n\n\
below is a list of possible commands:\n\
help\t\t- prints the help message\n\
play P\t\t- play an audio file located at path P\n\
playlist P\t- plays audio file(s) from a folder in user specified order (JSON) located at path P\n\n\
to quickly play a single audio file, type: audi play <path of the audio file you want to play>\n\
for example: audi play song.mp3\n\
");
}

// Prints time in the format: mm:ss
void print_time(int seconds)
{
    int min = ((float)seconds) / 60;

    printw("%02d:", min);

    if (seconds >= 60)
    {
        printw("%02d", seconds % 60);
    }
    else
    {
        printw("%02d", seconds);
    }
}

// Draws progress bar into the screen
void prog(int a, int max)
{
    int val = (((float)a / max) * 50);
    print_time(a);

    printw(" (");
    for (int i = 0; i <= 50; i++)
    {
        if (i <= val)
        {
            printw(">");
        }
        else
        {
            printw(".");
        }
    }

    printw(") ");
    print_time(max);
    printw("\n\n");
}

RCode play_file(const char *filename, bool from_dir, const char *dirname, int curpos, int total_count, const char *next_file)
{
    Mix_Music *music_file = Mix_LoadMUS(filename);

    if (music_file == NULL)
    {
        set_err_ref_file(filename);
        return ERR_COULD_NOT_PLAY;
    }

    bool running = true;
    bool playing = true;
    int duration = 0;
    RCode ret_code = SUCCESS;

    if (Mix_PlayMusic(music_file, 0) == -1)
    {
        Mix_FreeMusic(music_file);
        set_err_ref_file(filename);
        return ERR_COULD_NOT_PLAY;
    }

    int total_duration = Mix_MusicDuration(music_file);

    assert(total_duration >= 0);

    while (running)
    {
        int key = getch();

        erase();
        if (from_dir)
        {
            printw("playlist\t- %s\n", dirname);
            printw("position\t- %d / %d\n", curpos, total_count);

            if (next_file != NULL)
            {
                printw("next track\t- %s\n", next_file);
            }
        }

        printw("loaded track\t- %s\n", filename);
        printw("file status\t- %s\n\n", playing ? "playing" : "paused");
        // printw("Duration: %02d : %02d\n\n", duration, total_duration);
        prog(duration, total_duration);

        printw("[P] pause\t\t[R] resume\t\t[Q] quit\n");
        printw("[W] forward\t\t[S] backward\n");

        printw("\n");

        if (from_dir && curpos != total_count)
        {
            printw("[N] next track\t\t");
        }

        if (from_dir && curpos != 1)
        {
            printw("[B] previous track");
        }

        printw("\n");

        if (!Mix_PlayingMusic() && playing)
        {
            playing = false;
            running = false;
            break;
        }

        if (!playing && key == 'r')
        {
            Mix_ResumeMusic();
            playing = true;
        }
        else if (key == 'q' || key == 3)
        {
            if (playing)
                Mix_PauseMusic();

            clear();
            refresh();

            running = false;
            ret_code = PLAYER_QUIT;
            break;
        }

        else if (from_dir && key == 'n' && curpos != total_count)
        {
            if (playing)
                Mix_PauseMusic();

            running = false;
            ret_code = PLAYER_NEXT;
            break;
        }

        else if (from_dir && key == 'b' && curpos != 1)
        {
            if (playing)
                Mix_PauseMusic();

            running = false;
            ret_code = PLAYER_BACK;
            break;
        }

        else if (key == 'w' && duration < total_duration)
        {
            if (playing)
            {
                Mix_PauseMusic();
                playing = false;
            }

            duration += 1;
            Mix_SetMusicPosition(duration);
        }

        else if (key == 's' && duration > 0)
        {
            if (playing)
            {
                Mix_PauseMusic();
                playing = false;
            }

            duration -= 1;
            Mix_SetMusicPosition(duration);
        }

        if (playing)
        {
            if (key == 'p')
            {
                Mix_PauseMusic();
                playing = false;
            }
            duration = Mix_GetMusicPosition(music_file);
        }

        refresh();

        // Just to cap frames
        usleep(16000);
    }

    // clear();
    // refresh();

    Mix_FreeMusic(music_file);
    return ret_code;
}

typedef struct StringList
{
    char **list;
    int size;
} str_list;

str_list *init_str_list()
{
    str_list *list = malloc(sizeof(str_list));

    if (list == NULL)
    {
        return NULL;
    }

    list->list = NULL;
    list->size = 0;

    return list;
}

void append_str_list(str_list *list, char *element)
{
    assert(list != NULL);

    list->size++;
    list->list = realloc(list->list, list->size * (sizeof(char *)));

    assert(list->list != NULL);

    list->list[list->size - 1] = strdup(element);
}

// Debug function to see if everything
// is working fine
void print_str_list(str_list *list)
{
    for (int i = 0; i < list->size; i++)
    {
        printf("(D) Element at pos %d: \"%s\"\n", i, list->list[i]);
    }
}

void free_str_list(str_list *list)
{
    if (list != NULL)
    {
        if (list->list != NULL)
        {
            for (int i = 0; i < list->size; i++)
            {
                free(list->list[i]);
            }

            free(list->list);
            free(list);
        }
    }
}

char *get_filename_ext(const char *filename)
{
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

// Not used
// RCode play_dir(const char *dirname)
// {
//     DIR *folder = opendir(dirname);
//     if (folder == NULL)
//     {
//         return ERR_COULD_NOT_OPEN_DIR;
//     }

//     // const char *playable_extensions[3] = {"ogg", "mp3", "wav"};
//     struct dirent *folder_handler;

//     char full_path[300];
//     int count = 0;
//     RCode code = SUCCESS;
//     str_list *file_list = init_str_list();

//     while ((folder_handler = readdir(folder)) != NULL)
//     {
//         char *ext = get_filename_ext(folder_handler->d_name);

//         if ((SDL_strcmp(ext, "mp3") == 0) || (SDL_strcmp(ext, "wav") == 0) || (SDL_strcmp(ext, "ogg") == 0))
//         {
//             sprintf(full_path, "%s/%s", dirname, folder_handler->d_name);
//             append_str_list(file_list, full_path);
//             count++;
//         }
//     }

//     rewinddir(folder);

//     for (int index = 0; index < file_list->size; index++)
//     {
//         RCode c = play_file(file_list->list[index], true, dirname, index + 1, count);

//         if (c == PLAYER_QUIT)
//         {
//             break;
//         }
//         else if (c == PLAYER_NEXT)
//         {
//             continue;
//         }
//         else if (c == PLAYER_BACK)
//         {
//             index -= 2;
//         }
//         else
//         {
//             code = c;
//             break;
//         }
//     }

//     free_str_list(file_list);
//     closedir(folder);

//     return code;
// }

void setup(void)
{
    initscr();
    noecho();
    nodelay(stdscr, true);
    raw();
    Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_VolumeMusic(MIX_MAX_VOLUME);
}

void cleanup(void)
{
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    endwin();
}

typedef enum PlayerCommand
{
    PLAY_FILE,
    // PLAY_DIR,
    PLAY_PLAYLIST
} p_cmd;

RCode rp_error;

str_list *read_playlist_order(const char *filename)
{
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        set_err_ref_file(filename);
        rp_error = ERR_UNABLE_TO_OPEN_ORDER_FILE;
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *data = (char *)malloc(size + 1);

    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    struct json_object *parsed_json = json_tokener_parse(data);
    free(data);

    if (!parsed_json)
    {
        set_err_ref_file(filename);
        rp_error = ERR_JSON_INVALID_FORMAT;
        return NULL;
    }

    if (!json_object_is_type(parsed_json, json_type_array))
    {
        set_err_ref_file(filename);
        rp_error = ERR_JSON_INVALID_FORMAT;
        json_object_put(parsed_json);
        return NULL;
    }

    str_list *list = init_str_list();

    size_t array_len = json_object_array_length(parsed_json);

    for (size_t i = 0; i < array_len; i++)
    {
        struct json_object *elem = json_object_array_get_idx(parsed_json, i);

        if (json_object_is_type(elem, json_type_string))
        {
            append_str_list(list, (char *)json_object_get_string(elem));
        }
        else
        {
            free_str_list(list);
            json_object_put(parsed_json);
            set_err_ref_file(filename);
            rp_error = ERR_JSON_INVALID_FORMAT;
            return NULL;
        }
    }

    json_object_put(parsed_json);

    return list;
}

RCode play_playlist(const char *dirname)
{
    RCode code = SUCCESS;
    char full_path[300];
    char path[300];

    sprintf(path, "%s/%s", dirname, "order.json");
    str_list *file_list = read_playlist_order(path);

    if (file_list == NULL)
    {
        return rp_error;
    }

    for (int index = 0; index < file_list->size; index++)
    {
        bool next_file = (index + 1) < file_list->size;

        sprintf(full_path, "%s/%s", dirname, file_list->list[index]);

        RCode c = play_file(full_path, true, dirname, index + 1, file_list->size, next_file ? file_list->list[index + 1] : NULL);

        if (c == PLAYER_QUIT)
        {
            break;
        }
        else if (c == PLAYER_BACK)
        {
            index -= 2;
        }
        else if (c == SUCCESS || c == PLAYER_NEXT)
        {
            continue;
        }
        else
        {
            code = c;
            break;
        }
    }

    free_str_list(file_list);
    return code;
}

int main(int argc, const char **argv)
{
    p_cmd c;
    int exit_status = 0;

    // Quit if SDL didnt initialize
    if (SDL_Init(SDL_INIT_AUDIO) == -1)
    {
        printf("audi: error initializing SDL2\nerror log: %s\n", SDL_GetError());
        return 100;
    }

    if (argc < 2)
    {
        printf("audi: no command provided\ntype 'audi help' for more information\n");
        SDL_Quit();
        return 20;
    }
    else
    {

        // Print the help message
        // and quit SDL
        if (strcmp(argv[1], "help") == 0)
        {
            print_help();
            SDL_Quit();
            return 0;
        }

        // The original 'playlist' thing
        // idea

        // Play from a specific directory
        // else if (strcmp(argv[1], "playdir") == 0)
        // {
        //     if (argc >= 3)
        //     {
        //         c = PLAY_DIR;
        //     }
        //     else
        //     {
        //         printf("audi: No directory path provided.\n");
        //         SDL_Quit();
        //         return 1;
        //     }
        // }

        else if (strcmp(argv[1], "playlist") == 0)
        {
            if (argc >= 3)
            {
                c = PLAY_PLAYLIST;
            }
            else
            {
                printf("audi: no playlist path provided\n");
                SDL_Quit();
                return 20;
            }
        }

        else if (strcmp(argv[1], "play") == 0)
        {
            if (argc >= 3){
                if (access(argv[2], F_OK) != 0)
                {
                    printf("audi: could not open '%s'\n", argv[2]);
                    SDL_Quit();
                    return 1;
                }

                c = PLAY_FILE;
            }
            else
            {
                printf("audi: no audio file provided\n");
                SDL_Quit();
                return 1;
            }
        }
        else
        {
            printf("audi: unknown command '%s'\ntype 'audi help' for more information\n", argv[1]);
            return 30;
        }
    }

    // Setup
    setup();

    switch (c)
    {
    case PLAY_FILE:
        exit_status = play_file(argv[2], false, NULL, 0, 0, NULL);
        break;

        // Not used
        // case PLAY_DIR:
        //     exit_status = play_dir(argv[2]);
        //     break;

    case PLAY_PLAYLIST:
        exit_status = play_playlist(argv[2]);
        break;
    default:
        printf("audi: unknown command provided (command not registered!)\n");
        exit_status = 1;
        break;
    }

    cleanup();

    if (exit_status != SUCCESS)
    {
        switch (exit_status)
        {
        case ERR_COULD_NOT_PLAY:
            if (c == PLAY_PLAYLIST)
            {

                printf("audi: could not play '%s' from '%s'\n", err_ref_file, argv[2]);
            }
            else
            {
                printf("audi: could not play '%s'\n", err_ref_file);
            }
            break;

        case ERR_JSON_INVALID_FORMAT:
            printf("audi: invalid JSON format in '%s' of playlist '%s'\n", err_ref_file, argv[2]);
            break;

        case ERR_MEM_ALLOC:
            printf("audi: internal memory allocation error\n");
            break;

        case ERR_UNABLE_TO_OPEN_ORDER_FILE:
            printf("audi: could not open '%s' playlist's order JSON file\n", argv[2]);
            break;

        default:
            break;
        }

        free_err_file_ref();
    }

    return exit_status;
}

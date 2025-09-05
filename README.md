# Audi
A minimalistic terminal based audio player made using C. (Current Version 1.0)

(4/9/2025): I have completed this project and implemented all the features I wanted.

# Features
* Play audio files
* Move forward or backward at a certain duration
* Play all audio files from a directory in user specified order (Playlist)

# Usage
## Play an audio file
```
audi <path of the file you want to play>
```
## Play a playlist
```
audi playlist <path of the folder>
```
**NOTE**: Before playing, make sure the directory has an ***'order.json'*** file setup in it. If you don't know how to setup a playlist, [go here](#setting-up-a-playlist)

## Print Help Message
```
audi help
```

# Setting up a playlist
To quickly setup a playlist, follow the given steps:

### STEP 1 
To quickly setup a playlist, you need to first make a folder and store all the audio files in it. Here i have a folder my_playlist having all my files in it.

```
my_playlist
    |
    |_ song1.mp3
    |
    |_ song2.mp3
    |
    |_ song3.wav
```

---
### Step 2
Create an order.json file in your folder, like this:
```
my_playlist
    |
    |_ song1.mp3
    |
    |_ song2.mp3
    |
    |_ song3.wav
    |
    |_ order.json (+)
```
**NOTE**: The name should be the same as well as the case.

---
### Step 3
In your order.json file, add the name of the files you want to play in the order you like. For example let's say I want to play song3.wav first then song1.mp3; So my order.json file will look like this:

```json
[
    "song3.wav",
    "song1.mp3"
]
```

Now, when you play this playlist, song3.wav will be played first. Note that I have not included song2.mp3 file in this json, which means that song2.mp3 will not get played by the player and will be ignored.

---
### Step 4
Finally type audi playlist followed by the path of the folder you want to play from.

```
audi playlist my_playlist
```


## Done!
This is it, you have created your own playlist. Just some points to remember:

* All files given should exist and must be of supported formats (mp3, wav, ogg additional formats require more libs)

* Audi will only play files that are mentioned in order.json.

Enjoy the music!

# Thank You!
Thank you for using audi! I really appreciate it! I hope you like it.
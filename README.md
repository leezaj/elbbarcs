# Elbbarcs

### [Play it in your web browser!](https://leezaj.github.io/elbbarcs/)

## Description

Elbbarcs is a single-player crossword-style game where you must make valid, connected words on a board given a set of
letter tiles. Your opponent uses a very simple but effective strategy: play the highest possible scoring move each turn.
Your goal is to beat the opponent's brute-force strategy with your own. While you are given a 'Hint' button that *also*
allows you to find the highest scoring move, the challenge is to be able to beat the opponent by yourself!

<details>
<summary><h3> FAQ </h3></summary>

### Which word list does the game use?
Elbbarcs uses the [NASPA](https://www.scrabbleplayers.org) Word List 2023 Edition (NWL2023).

### How am I supposed to beat this thing?
While the computer's strategy is very powerful, it has the following shortcomings:
1. **It has no concept of defensive play**. It *will* give away high scoring squares and leave itself open to strong
   attacks by you in the process of making its moves.
2. **It does not swap letters**. This means that even if it has a terrible set of letters to play with, where swapping
   would be the best option, it will still try to make a word with what its got.
3. **It has no foresight**. It will not save valuable letters for the future and it does not rearrange its letters to
   make future 'bingo' moves.

All of these disadvantages are things that a committed player can exploit.

### Does the opponent have a rack of tiles, just like I do?
Yes. It won't 'magically' play tiles out of nowhere. If it puts an 'S' after your word, it's because it actually drew
one :-)

### I have a question that's not listed!
Feel free to send me an email at jazeel.abdul3 at gmail dot com for any questions. You are also welcome to open an issue
if needed.
</details>

<details>
<summary> <h3> Build Instructions </h3> </summary>

Elbbarcs has the following requirements and dependencies:
- A compatible C++23 compiler
- CMake 3.5 or newer
- SDL2, SDL2_ttf, SDL2_image and SDL2_gfx development headers
- Boost headers (needed for `boost::hash_combine`)

Elbbarcs also uses [Glaze](https://github.com/stephenberry/glaze) for serialization, but this will automatically be
fetched with CMake.

1. Clone the repository with `git clone https://github.com/leezaj/elbbarcs.git`
2. In the newly made `elbbarcs` folder, create a `build` folder and change directories to it
3. Run `cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4`. This will also pull Glaze due to CMake's *FetchContent*
   feature.
4. The binary will be installed in the `bin` folder in the root directory. Run `../bin/elbbarcs` from inside `build` to
   launch the game.

Please note that building has only been tested on Linux so far, and has not been tested for other operating systems.
</details>

<details>
<summary> <h3> Technical Details </h3> </summary>

The game's dictionary of words is generated using a *Directed Acyclic Word Graph (DAWG)* construction algorithm[^1].
This data structure is similar to a trie but much more memory efficient. Click
[here](https://en.wikipedia.org/wiki/Deterministic_acyclic_finite_state_automaton) to read more about it.

While the *DAWG* *could* be constructed each time the game starts, the DAWG has instead been pre-constructed once, and
then serialized into a binary file format. The game then deserializes it when launching, which results in a faster
construction time (around 9x faster), thus leading to faster load times.

The solver, which finds the highest scoring move, uses a fast backtracking algorithm[^2] that traverses the *DAWG* with
a given set of letters, finds all possible moves, calculates the score for each one, and saves the max scoring one.

The game has been compiled into WebAssembly for the web using [Emscripten](https://emscripten.org/).
</details>

## To-Do

- [ ] Add fullscreen support
- [ ] Embed the game's assets directly into the source code to minimize file loads
- [ ] Add sound
- [ ] Show definitions of words played
- [ ] If the player tries to make an invalid word, specify which word(s) failed
- [ ] Allow changing the difficulty of the opponent
- [ ] Add a feature for the opponent to swap tiles if it's the best move to do so

## Credits
Design, art, and programming by Jazeel Abdul-Jabbar (leezaj). 

NASPA Word List 2023 Edition © NASPA 2023. The copy included in this app is licensed for personal use. You may not use
it for any commercial purposes.

The tile template was made by [Sinner-PWA on
DeviantArt](https://www.deviantart.com/sinner-pwa/art/Scrabble-tile-Vector-and-PSD-396003600).

[^1]: [Jan Daciuk, Stoyan Mihov, Bruce W. Watson and Richard E. Watson (2000). *Incremental Construction of Minimal Acyclic Finite-State Automata*](https://aclanthology.org/J00-1002.pdf)
[^2]: [Andrew W. Appel and Guy J. Jacobson (1988). *The World's Fastest Scrabble Program*](https://www.cs.cmu.edu/afs/cs/academic/class/15451-s06/www/lectures/scrabble.pdf)

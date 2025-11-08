# C++ Samdbox1


## about

- C++復習とSDL3の練習場所です。

## initialize, build and run

- CMakeを利用しています。
- SDL3をsubmoduleで配置しています。
    - メモ: git submodule add https://github.com/libsdl-org/SDL.git vendored/SDL
- configure
    - cmake -S . -B build
- build iteration
    - cmake --build build && ./build/main

---

End of Contents
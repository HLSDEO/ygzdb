cmake_minimum_required(VERSION 3.21)

project(
    CARTAS
    LANGUAGES    C ASM
    VERSION      1.0.0
    DESCRIPTION  "Cartas do Destiny - PS1 card duel"
)

file(GLOB _sources src/*.c)
psn00bsdk_add_executable(GAME STATIC ${_sources})

install(FILES ${PROJECT_BINARY_DIR}/GAME.exe TYPE BIN)

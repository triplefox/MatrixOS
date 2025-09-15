# SameGame Application
file(GLOB_RECURSE SAMEGAME_SOURCES "*.cpp" "*.c")
file(GLOB_RECURSE SAMEGAME_HEADERS "*.h")

add_library(SameGameAPP ${SAMEGAME_SOURCES} ${SAMEGAME_HEADERS})

target_include_directories(SameGameAPP PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SameGameAPP PUBLIC
    MatrixOSInterface
    MatrixOSAPPInterface
    BrightnessControlUI
)

add_executable(nw_snd_test
    ./src/include/utils/file_util.h
    ./src/main.cpp
)

target_include_directories(nw_snd_test PRIVATE ./src/include)
target_link_libraries(nw_snd_test PUBLIC nw_snd)
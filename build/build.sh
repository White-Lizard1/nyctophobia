# !/bin/bash

# gcc -c external/GLAD/src/glad.c -Iexternal/GLAD/include -o out/glad.o -g -Wall
# echo "GLAD built"

# gcc -c external/stb/stb_impl.c -Iexternal -o out/stb.o -g -Wall
# echo "STB built"

# gcc -c external/nfd/nfd_win.cpp -Iexternal/nfd -Iexternal/nfd/include -o out/nfd_win.o -g -Wall
# echo "nfd built"

gcc -c src/main.c -o out/main.o -g -Wall -pedantic -Wextra -Werror -Iexternal -Isrc
echo "main built"

g++ out/main.o out/nfd_win.o out/stb.o out/glad.o -o out/main.exe -g -Wall -Lexternal/SDL2/bin -lSDL2 -lm -lole32 -luuid
echo "finished linking"

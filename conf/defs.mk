PPGO_CXX := g++
PPGO_CXX_FLAGS := -std=gnu++17 -ggdb3 -O2 \
	-Werror -Wall -Wextra -Wshadow \
	-Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable \
	-fPIC -fchar8_t -fno-strict-aliasing -fno-delete-null-pointer-checks -fno-strict-overflow \
	-pthread -U_FORTIFY_SOURCE

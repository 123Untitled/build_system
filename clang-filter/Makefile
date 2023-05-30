.PHONY: all
.SILENT:

exec     := clang-filter
src      := $(wildcard src/*.cpp)
inc      := $(addprefix -I,$(shell find inc -type f -name '*.hpp'))
cxxflags := -Wall -Wextra -Werror -pedantic
opt      := -O3
std      := -std=c++20

all:
	clang++ $(std) $(opt) $(cxxflags) $(inc) $(src) -o $(exec)

fclean:
	rm -rf $(exec)


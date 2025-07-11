CPP := g++

CPP_FILES := src/Zucchini.cpp src/afctool.cpp src/FileHolder.cpp src/BinaryReader.cpp src/wavfile.cpp src/asttool.cpp

ifeq ($(OS),Windows_NT)
	Zucchini := Zucchini.exe
else
	Zucchini := Zucchini
endif

all:
	$(CPP) -O3 -s -static -I . -Wno-multichar -Wall -o $(Zucchini) $(CPP_FILES)
	
clean:
	rm -f $(Zucchini)
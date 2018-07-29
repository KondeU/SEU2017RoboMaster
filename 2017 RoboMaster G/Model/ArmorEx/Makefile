LDFLAGS = -Wl,-rpath,./ -L./ -lpthread -lrt -lv4l2 $(OPENCV_LINK)

OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`

./Release/Run.out : ./Release/Armor.o ./Release/Main.o
	g++ -o ./Release/Run.out ./Release/Armor.o ./Release/Main.o $(LDFLAGS)

./Release/Armor.o : Armor.cpp Armor.h
	g++ -o ./Release/Armor.o -c Armor.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Main.o : Main.cpp Armor.h
	g++ -o ./Release/Main.o -c Main.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

clean:
	rm -f ./Release/*
	rm -f *~

LDFLAGS = -Wl,-rpath,./ -L./ -L./SDK_Apriltags/build/lib/arm7/ -lpthread -lrt -lapriltags $(OPENCV_LINK)

OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`

./Release/Run.out : ./Release/Apriltags.o ./Release/Main.o
	g++ -o ./Release/Run.out ./Release/Apriltags.o ./Release/Main.o $(LDFLAGS)

./Release/Apriltags.o : Apriltags.cpp Apriltags.h
	g++ -o ./Release/Apriltags.o -c Apriltags.cpp -I./SDK_Apriltags/build/include/ -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Main.o : Main.cpp Apriltags.h
	g++ -o ./Release/Main.o -c Main.cpp -I./SDK_Apriltags/build/include/ -g -Wall --std=c++11 $(OPENCV_FLAG)

clean:
	rm -f ./Release/*
	rm -f *~
	

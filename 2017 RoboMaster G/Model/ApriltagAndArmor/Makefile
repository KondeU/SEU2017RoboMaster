LDFLAGS = -Wl,-rpath,./ -L./ -L./SDK_Apriltag/build/arm7/ -lpthread -lrt -lapriltag -lv4l2 $(OPENCV_LINK)

OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`

MUTITNR = -pthread

./Release/Run.out : ./Release/Apriltag.o ./Release/Armor.o ./Release/Main.o
	g++ -o ./Release/Run.out ./Release/Apriltag.o ./Release/Armor.o ./Release/Main.o $(LDFLAGS) $(MUTITNR)

./Release/Apriltag.o : Apriltag.cpp Apriltag.h
	g++ -o ./Release/Apriltag.o -c Apriltag.cpp -I./SDK_Apriltag/ -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

./Release/Armor.o : Armor.cpp Armor.h
	g++ -o ./Release/Armor.o -c Armor.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Main.o : Main.cpp Apriltag.h Armor.h
	g++ -o ./Release/Main.o -c Main.cpp -I./SDK_Apriltag/ -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

clean:
	rm -f ./Release/*
	rm -f *~
	

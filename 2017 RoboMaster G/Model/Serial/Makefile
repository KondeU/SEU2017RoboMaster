LDFLAGS = -Wl,-rpath,./ -L./ -lpthread -lrt $(OPENCV_LINK)
OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`

./Release/Run.out : ./Release/CConfig.o ./Release/CVideolog.o ./Release/Logger.o ./Release/Serial.o ./Release/Main.o
	g++ -o ./Release/Run.out ./Release/CConfig.o ./Release/CVideolog.o ./Release/Logger.o ./Release/Serial.o ./Release/Main.o $(LDFLAGS)

./Release/CConfig.o : CConfig.cpp CConfig.h
	g++ -o ./Release/CConfig.o -c CConfig.cpp -g -Wall --std=c++11

./Release/CVideolog.o : CVideolog.cpp Logger.cpp CVideolog.h
	g++ -o ./Release/CVideolog.o -c CVideolog.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Logger.o : Logger.cpp Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Logger.o -c Logger.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Serial.o : Serial.cpp Serial.h Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Serial.o -c Serial.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

./Release/Main.o : Main.cpp Serial.h Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Main.o -c Main.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)

clean:
	rm -f ./Release/*
	rm -f *~

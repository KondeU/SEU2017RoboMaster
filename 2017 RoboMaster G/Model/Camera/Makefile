LDFLAGS = -Wl,-rpath,./ -L./ -lpthread -lrt $(OPENCV_LINK)
OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`
MUTITNR = -pthread

./Release/Run.out : ./Release/CConfig.o ./Release/CVideolog.o ./Release/Logger.o ./Release/Camera.o ./Release/Main.o
	g++ -o ./Release/Run.out ./Release/CConfig.o ./Release/CVideolog.o ./Release/Logger.o ./Release/Camera.o ./Release/Main.o $(LDFLAGS)

./Release/CConfig.o : CConfig.cpp CConfig.h
	g++ -o ./Release/CConfig.o -c CConfig.cpp -g -Wall --std=c++11 $(MUTITNR)

./Release/CVideolog.o : CVideolog.cpp Logger.cpp CVideolog.h
	g++ -o ./Release/CVideolog.o -c CVideolog.cpp -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

./Release/Logger.o : Logger.cpp Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Logger.o -c Logger.cpp -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

./Release/Camera.o : Camera.cpp Camera.h Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Camera.o -c Camera.cpp -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

./Release/Main.o : Main.cpp Camera.h Logger.h CConfig.h CVideolog.h
	g++ -o ./Release/Main.o -c Main.cpp -g -Wall --std=c++11 $(OPENCV_FLAG) $(MUTITNR)

clean:
	rm -f ./Release/*
	rm -f *~

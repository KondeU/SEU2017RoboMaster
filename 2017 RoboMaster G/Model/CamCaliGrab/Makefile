LDFLAGS = -Wl,-rpath,./ -L./ -lpthread -lrt $(OPENCV_LINK)
OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs`

camgrab.out : camgrab.o
	g++ -o camgrab.out camgrab.o $(LDFLAGS)

camgrab.o : camgrab.cpp
	g++ -o camgrab.o -c camgrab.cpp -g -Wall --std=c++11 $(OPENCV_FLAG)


clean:
	rm -f camgrab.o camgrab.out
	rm -f *~

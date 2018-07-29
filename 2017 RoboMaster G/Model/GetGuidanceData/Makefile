TARGET = guidance

OBJECTS = DJI_utility.o Guidance.o Main.o

CFLAGS = -g -Wall --std=c++11 -I./GuidanceSDK/include -I./GuidanceSDK/examples/usb_example/DJI_guidance_example `pkg-config opencv --cflags`

LDFLAGS = -Wl,-rpath,./ -L./ -lDJI_guidance -lpthread -lrt -lusb-1.0 `pkg-config opencv --libs` `pkg-config cuda-6.5 --libs` `pkg-config cudart-6.5 --libs`

CXX = g++

$(TARGET) : $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

Guidance.o : Guidance.cpp
	$(CXX) $(CFLAGS) -c Guidance.cpp

DJI_utility.o : ./GuidanceSDK/examples/usb_example/DJI_guidance_example/DJI_utility.cpp
	$(CXX) $(CFLAGS) -c ./GuidanceSDK/examples/usb_example/DJI_guidance_example/DJI_utility.cpp

Main.o : Main.cpp
	$(CXX) $(CFLAGS) -c Main.cpp
	
clean:
	rm *.o $(TARGET)



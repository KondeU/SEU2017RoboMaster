# Makefile
# Version 1.0
# Writed by Deyou Kong, 2017-04-20
# Check by Deyou Kong, 2017-04-21


TARGET = RM2017.out

CXX = g++

DIR_RELEASE = ./Release

LDFLAGS = -Wl,-rpath,./ -L./ -lDJI_guidance -ldcam -lpthread -lrt -lusb-1.0 $(OPENCV_LINK)


# OpenCV

OPENCV_FLAG = `pkg-config opencv --cflags`
OPENCV_LINK = `pkg-config opencv --libs` `pkg-config cuda-6.5 --libs` `pkg-config cudart-6.5 --libs`


# Guidance

DIR_GUIDANCE = ./GuidanceSDK
DIR_GUID_INC = $(DIR_GUIDANCE)/include $(DIR_GUIDANCE)/examples/usb_example/DJI_guidance_example
DIR_GUID_SRC = $(DIR_GUIDANCE)/examples/usb_example/DJI_guidance_example

GUID_OBJECTS = $(DIR_RELEASE)/DJI_utility.o $(DIR_RELEASE)/Guidance.o
GUID_CXXFLAGS = -g -Wall --std=c++11 $(addprefix -I,$(DIR_GUID_INC)) $(OPENCV_FLAG)


# Onboard

DIR_ONBOARD = ./OnboardSDK
DIR_FLGT_INC = $(DIR_ONBOARD)/osdk-core/inc $(DIR_ONBOARD)/platform/manifold/inc $(DIR_ONBOARD)/sample/CommandlineLinux_old/inc $(DIR_ONBOARD)/sample/CommandlineLinux_old/DJIscript/inc
DIR_FLGT_SRC_LIBS = $(DIR_ONBOARD)/osdk-core/src
DIR_FLGT_SRC_MANIFOLD = $(DIR_ONBOARD)/platform/manifold/src
DIR_FLGT_SRC_DJITHREAD = $(DIR_ONBOARD)/sample/CommandlineLinux_old/src
DIR_FLGT_SRC_DJISCRIPT = $(DIR_ONBOARD)/sample/CommandlineLinux_old/DJIscript/src

FLGT_SOURCES_LIBS = DJI_API.cpp DJI_App.cpp DJI_Camera.cpp DJI_Codec.cpp DJI_Flight.cpp DJI_Follow.cpp DJI_HardDriver.cpp DJI_HotPoint.cpp DJI_Link.cpp DJI_Memory.cpp DJI_Mission.cpp DJI_VirtualRC.cpp DJI_WayPoint.cpp
FLGT_DIRSRCS_LIBS = $(addprefix $(DIR_FLGT_SRC_LIBS)/,$(FLGT_SOURCES_LIBS))
FLGT_OBJECTS_LIBS = $(addprefix $(DIR_RELEASE)/,$(FLGT_SOURCES_LIBS:.cpp=.o))

FLGT_SOURCES_MANIFOLD = DJIHardDriverManifold.cpp
FLGT_DIRSRCS_MANIFOLD = $(addprefix $(DIR_FLGT_SRC_MANIFOLD)/,$(FLGT_SOURCES_MANIFOLD))
FLGT_OBJECTS_MANIFOLD = $(addprefix $(DIR_RELEASE)/,$(FLGT_SOURCES_MANIFOLD:.cpp=.o))

FLGT_SOURCES_DJITHREAD = APIThread.cpp conboardsdktask.cpp
FLGT_DIRSRCS_DJITHREAD = $(addprefix $(DIR_FLGT_SRC_DJITHREAD)/,$(FLGT_SOURCES_DJITHREAD))
FLGT_OBJECTS_DJITHREAD = $(addprefix $(DIR_RELEASE)/,$(FLGT_SOURCES_DJITHREAD:.cpp=.o))

FLGT_SOURCES_DJISCRIPT = cmdCamera.cpp cmdCoreAPI.cpp cmdFlight.cpp cmdFollow.cpp cmdHotPoint.cpp cmdIO.cpp cmdSettings.cpp cmdVirtualRC.cpp cmdWayPoint.cpp DJI_Interpreter.cpp DJI_Script.cpp
FLGT_DIRSRCS_DJISCRIPT = $(addprefix $(DIR_FLGT_SRC_DJISCRIPT)/,$(FLGT_SOURCES_DJISCRIPT))
FLGT_OBJECTS_DJISCRIPT = $(addprefix $(DIR_RELEASE)/,$(FLGT_SOURCES_DJISCRIPT:.cpp=.o))

FLGT_OBJECTS = $(FLGT_OBJECTS_LIBS) $(FLGT_OBJECTS_MANIFOLD) $(FLGT_OBJECTS_DJITHREAD) $(FLGT_OBJECTS_DJISCRIPT)
FLGT_CXXFLAGS = -g -Wall --std=c++11 $(addprefix -I,$(DIR_FLGT_INC)) -pthread


# Third

DIR_THIRD = ./ThirdSDK
DIR_THRD_INC = $(DIR_THIRD)
DIR_THRD_SRC = $(DIR_THIRD)

THRD_OBJECTS = $(DIR_RELEASE)/histogram.o
THRD_CXXFLAGS = -g -Wall --std=c++11 -I$(DIR_THRD_INC) $(OPENCV_FLAG)


# Onboard custom

DIR_CUSTOM = .
DIR_CUST_INC = $(DIR_CUSTOM) $(DIR_FLGT_INC) $(DIR_THRD_INC)
DIR_CUST_SRC = $(DIR_CUSTOM)

CUST_OBJECTS = $(DIR_RELEASE)/Videolog.o $(DIR_RELEASE)/Flight.o $(DIR_RELEASE)/FlightIndoor.o $(DIR_RELEASE)/X3.o $(DIR_RELEASE)/Main.o $(DIR_RELEASE)/Task.o $(DIR_RELEASE)/RM_Task.o
CUST_CXXFLAGS = -g -Wall --std=c++11 $(addprefix -I,$(DIR_CUST_INC)) $(OPENCV_FLAG)


# Relationships

$(TARGET) : $(GUID_OBJECTS) $(FLGT_OBJECTS) $(THRD_OBJECTS) $(CUST_OBJECTS)
	cp $(DIR_GUIDANCE)/so/XU3/libDJI_guidance.so .
	cp $(DIR_GUIDANCE)/so/XU3/libDJI_guidance.so $(DIR_RELEASE)
	$(CXX) -o $(DIR_RELEASE)/$(TARGET) $(GUID_OBJECTS) $(FLGT_OBJECTS) $(THRD_OBJECTS) $(CUST_OBJECTS) $(LDFLAGS)
	
$(DIR_RELEASE)/DJI_utility.o : $(DIR_GUID_SRC)/DJI_utility.cpp
	$(CXX) $(GUID_CXXFLAGS) -o $(DIR_RELEASE)/DJI_utility.o -c $(DIR_GUID_SRC)/DJI_utility.cpp

$(DIR_RELEASE)/Guidance.o : Guidance.cpp
	$(CXX) $(GUID_CXXFLAGS) -o $(DIR_RELEASE)/Guidance.o -c Guidance.cpp

$(DIR_RELEASE)/%.o : $(DIR_FLGT_SRC_LIBS)/%.cpp
	$(CXX) $(FLGT_CXXFLAGS) -o $@ -c $<

$(DIR_RELEASE)/%.o : $(DIR_FLGT_SRC_MANIFOLD)/%.cpp
	$(CXX) $(FLGT_CXXFLAGS) -o $@ -c $<

$(DIR_RELEASE)/%.o : $(DIR_FLGT_SRC_DJITHREAD)/%.cpp
	$(CXX) $(FLGT_CXXFLAGS) -o $@ -c $<

$(DIR_RELEASE)/%.o : $(DIR_FLGT_SRC_DJISCRIPT)/%.cpp
	$(CXX) $(FLGT_CXXFLAGS) -o $@ -c $<

$(DIR_RELEASE)/histogram.o : $(DIR_THRD_SRC)/histogram.cpp
	$(CXX) $(THRD_CXXFLAGS) -o $(DIR_RELEASE)/histogram.o -c $(DIR_THRD_SRC)/histogram.cpp

$(DIR_RELEASE)/Videolog.o : $(DIR_CUST_SRC)/Videolog.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/Videolog.o -c $(DIR_CUST_SRC)/Videolog.cpp

$(DIR_RELEASE)/Flight.o : $(DIR_CUST_SRC)/Flight.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/Flight.o -c $(DIR_CUST_SRC)/Flight.cpp

$(DIR_RELEASE)/FlightIndoor.o : $(DIR_CUST_SRC)/FlightIndoor.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/FlightIndoor.o -c $(DIR_CUST_SRC)/FlightIndoor.cpp

$(DIR_RELEASE)/X3.o : $(DIR_CUST_SRC)/X3.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/X3.o -c $(DIR_CUST_SRC)/X3.cpp

$(DIR_RELEASE)/Main.o : $(DIR_CUST_SRC)/Main.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/Main.o -c $(DIR_CUST_SRC)/Main.cpp

$(DIR_RELEASE)/Task.o : $(DIR_CUST_SRC)/Task.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/Task.o -c $(DIR_CUST_SRC)/Task.cpp

$(DIR_RELEASE)/RM_Task.o : $(DIR_CUST_SRC)/RM_Task.cpp
	$(CXX) $(CUST_CXXFLAGS) -o $(DIR_RELEASE)/RM_Task.o -c $(DIR_CUST_SRC)/RM_Task.cpp
	

# Command
	
clean:
	rm -f $(DIR_RELEASE)/*
	rm -f libDJI_guidance.so
	rm -f log.txt
	rm -f *~

remove:
	rm -f $(DIR_RELEASE)/$(TARGET) $(DIR_RELEASE)/RM_Task.o
	rm -f libDJI_guidance.so
	rm -f log.txt
	rm -f *~

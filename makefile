src=$(wildcard *.cpp)
objs=$(patsubst %.cpp,%.o,$(src))
LIBS=-lstdc++ -lpthread -lssl -lcrypto -locci -lclntsh
target=Main
$(target):$(objs)
	g++ -std=c++11 -o $@ $^ $(LIBS)

%.o:%.cpp
	g++ -g -std=c++11 -c $<
.PHONY:clean
clean:
	rm -f $(target) $(objs)

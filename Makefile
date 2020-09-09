SRC_DIR=.
SRCS=$(wildcard $(SRC_DIR)/*.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))
TARGET=unreal-hacking

CPP=g++

.PHONY: all tidy clean

all: $(TARGET)

%.o: %.cpp $(DEPENDS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CPP) $^ $(CPPFLAGS) -o $@

tidy:
	rm -f $(OBJS)

clean: tidy
	rm -f $(TARGET)

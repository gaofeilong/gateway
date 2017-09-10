object = json_value.o json_writer.o json_reader.o

all: jsoncpp.a

jsoncpp.a:$(object)
	ar -cr $@ $(object)

json_value.o:json_value.cpp
	g++ -c $< -o $@

json_writer.o:json_writer.cpp
	g++ -c $< -o $@

json_read.o:json_reader.cpp
	g++ -c $< -o $@

clean:
	rm -f jsoncpp.a
	rm -f *.o

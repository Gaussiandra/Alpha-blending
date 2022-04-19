SFML_flags = -lsfml-graphics -lsfml-window -lsfml-system
CXX_NoSSE_flags = -O0
CXX_SSE_flags = -D SSE -O0 -mavx2 

#---------------------------------------

exec_nosse: blending-nosse.out
	./blending-nosse.out

blending-nosse.out: window-nosse.o blending-nosse.o
	g++ window-nosse.o blending-nosse.o $(CXX_NoSSE_flags) $(SFML_flags) -o blending-nosse.out

window-nosse.o: window.cpp
	g++ -c window.cpp $(CXX_NoSSE_flags) -o window-nosse.o

blending-nosse.o: blending-nosse.cpp blending-nosse.hpp
	g++ -c blending-nosse.cpp $(CXX_NoSSE_flags) -o blending-nosse.o

#---------------------------------------

exec_sse: blending-sse.out
	./blending-sse.out

window-sse.o: window.cpp
	g++ -c window.cpp $(CXX_SSE_flags) -o window-sse.o

blending-sse.out: window-sse.o blending-sse.o
	g++ window-sse.o blending-sse.o $(CXX_SSE_flags) $(SFML_flags) -o blending-sse.out

blending-sse.o: blending-sse.cpp blending-sse.hpp
	g++ -c blending-sse.cpp $(CXX_SSE_flags) -o blending-sse.o

#---------------------------------------

clear:
	rm *.o *.out
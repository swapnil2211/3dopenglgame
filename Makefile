all: game2

#sample3D: Sample_GL3_3D.cpp glad.c
#	g++ -o sample3D Sample_GL3.cpp glad.c -lGL -lglfw

game2: game2.cpp glad.c
	g++ -o game2 game2.cpp glad.c -lGL -lglfw -ldl

clean:
	rm game2

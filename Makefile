Files = ./build/Multithreading/ThreadPool.o ./build/Model/Loss.o ./build/Model/Tree/TreeBooster.o ./build/Model/Tree/XGTreeBooster.o ./build/Utils/model_utils.o ./build/DataStructs/Matrix/lin_alg.o ./build/Data/data.o ./build/Model/Activation.o ./build/DataStructs/Memory/Arena.o ./build/DataStructs/Memory/HazardPointer.o ./build/Model/Tree/TreeBoosterNode.o
Include = -I./include
Flags = $(Include) --std=c++23 -fPIC -Ofast -fno-math-errno -fno-trapping-math -ffp-contract=fast -latomic -pthread -Wno-terminate

TestsImp = ./tests/test.cpp
TestsObj = ./build/tests/test.o

all: ./build/libCNum.so $(TestsObj)

./build/tests/test.o: ./tests/test.cpp
	g++ $(Include) --std=c++23 -fsanitize=address -fsanitize=leak -g -O1 -lCNum -Ofast -fno-math-errno -fno-trapping-math -ffp-contract=fast -latomic -pthread -Wno-terminate ./tests/test.cpp -o ./build/tests/test.o

./build/libCNum.so: $(Files)
	g++ $(Include) --std=c++23 $(Files) -shared -o ./build/libCNum.so

./build/Multithreading/ThreadPool.o: ./src/Multithreading/ThreadPool.cpp
	g++ $(Flags) -c ./src/Multithreading/ThreadPool.cpp -o ./build/Multithreading/ThreadPool.o

./build/Model/Loss.o: ./src/Model/Loss.cpp
	g++ $(Flags) -c ./src/Model/Loss.cpp -o ./build/Model/Loss.o

./build/Model/Activation.o: ./src/Model/Activation.cpp
	g++ $(Flags) -c ./src/Model/Activation.cpp -o ./build/Model/Activation.o

./build/Model/Tree/TreeBooster.o: ./src/Model/Tree/TreeBooster.cpp
	g++ $(Flags) -c ./src/Model/Tree/TreeBooster.cpp -o ./build/Model/Tree/TreeBooster.o

./build/Model/Tree/TreeBoosterNode.o: ./src/Model/Tree/TreeBoosterNode.cpp
	g++ $(Flags) -c ./src/Model/Tree/TreeBoosterNode.cpp -o ./build/Model/Tree/TreeBoosterNode.o

./build/Model/Tree/XGTreeBooster.o: ./src/Model/Tree/XGTreeBooster.cpp
	g++ $(Flags) -c ./src/Model/Tree/XGTreeBooster.cpp -o ./build/Model/Tree/XGTreeBooster.o

./build/Model/Tree/LGBMTreeBooster.o: ./src/Model/Tree/LGBMTreeBooster.cpp
	g++ $(Flags) -c ./src/Model/Tree/LGBMTreeBooster.cpp -o ./build/Model/Tree/LGBMTreeBooster.o

./build/Utils/model_utils.o: ./src/Utils/model_utils.cpp
	g++ $(Flags) -c ./src/Utils/model_utils.cpp -o ./build/Utils/model_utils.o

./build/DataStructs/Matrix/lin_alg.o: ./src/DataStructs/Matrix/lin_alg.cpp
	g++ $(Flags) -c ./src/DataStructs/Matrix/lin_alg.cpp -o ./build/DataStructs/Matrix/lin_alg.o

./build/Data/data.o: ./src/Data/data.cpp
	g++ $(Flags) -c ./src/Data/data.cpp -o ./build/Data/data.o

./build/DataStructs/Memory/Arena.o: ./src/DataStructs/Memory/Arena.c
	gcc $(Include) -c ./src/DataStructs/Memory/Arena.c -o ./build/DataStructs/Memory/Arena.o

./build/DataStructs/Memory/HazardPointer.o: ./src/DataStructs/Memory/HazardPointer.cpp
	g++ $(Flags) -c ./src/DataStructs/Memory/HazardPointer.cpp -o ./build/DataStructs/Memory/HazardPointer.o

test: ./build/tests/test.o
	./build/tests/test.o

clean:
	rm -rf $(Files)
	rm -rf $(TestsObj)
	rm -rf ./tests/model.cmod
	rm -rf ./build/libCNum.so

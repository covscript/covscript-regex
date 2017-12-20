@mkdir build
@cd build
@mkdir imports
@g++ -std=c++11 -I ..\include -shared -static -fPIC -s -O3 ..\regex.cpp -o .\imports\regex.cse
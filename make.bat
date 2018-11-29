@mkdir build
@cd build
@mkdir imports
@g++ -std=c++11 -I%CS_DEV_PATH%\include -I..\include -shared -static -fPIC -s -O3 ..\regex.cpp -o .\imports\regex.cse -L%CS_DEV_PATH%\lib -lcovscript
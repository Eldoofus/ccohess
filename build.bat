ptime g++ -O2 -std=gnu++23 -march=native engine.cpp -o engine.exe -L"senjo/build/" -lsenjo -flto && engine
:: -ftemplate-backtrace-limit=1000 -ftemplate-depth=20
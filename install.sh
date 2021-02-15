rm -rf ~/.config/scat # Remove cache file if exists
g++ -o scat main.cpp -std=c++17 -l sqlite3
chmod +x ./scat
for i in $(ls ./langs/);
    do
        ./scat --save ./langs/$i
        echo "Parsing file \"${i}\""
    done
./scat --create_ptr .cpp .c
./scat --create_ptr .cpp .h
./scat --create_ptr .cpp .cpp
# rm -rf ./langs
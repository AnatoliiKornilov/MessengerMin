cd ~/projects/MessengerMin
rm -rf build

mkdir build
cd build

cmake ..
cmake --build .

./test_user_repository
./test_chat_repository

cd ..

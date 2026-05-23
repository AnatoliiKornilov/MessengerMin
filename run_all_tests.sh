export DB_CONN="postgresql://messenger:devpass@localhost:5432/messenger_db"

cd ~/projects/MessengerMin
rm -rf build

mkdir build
cd build

cmake ..
cmake --build .

./test_user_repository
./test_chat_repository
./test_message_repository

cd ..

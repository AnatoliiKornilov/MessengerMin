export DBCONN="postgresql://messenger:devpass@localhost:5432/messenger_db"
export JWT_SECRET="super-secret-jwt-key-for-dev-only"
export PORT="8080"
export DB_POOL_SIZE="10"

rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --target messenger_server -j$(nproc)

./messenger_server

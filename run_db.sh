set -e

docker compose down -v 2>/dev/null
docker compose up -d

until docker exec messenger-db pg_isready -U messenger -d messenger_db; do
    sleep 1
done

docker exec messenger-db psql -U messenger -d messenger_db -c "\dt"

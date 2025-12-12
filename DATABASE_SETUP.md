# PostgreSQL Database Setup

## 🐳 Using Docker (Recommended)

The easiest way to run PostgreSQL is using Docker Compose. No installation needed!

### Prerequisites
- [Docker](https://www.docker.com/get-started) installed on your system

### Quick Start

1. **Start PostgreSQL:**
   ```bash
   docker-compose up -d
   ```

2. **Check if it's running:**
   ```bash
   docker-compose ps
   ```

3. **View logs:**
   ```bash
   docker-compose logs postgres
   ```

4. **Stop PostgreSQL:**
   ```bash
   docker-compose down
   ```

5. **Stop and remove all data:**
   ```bash
   docker-compose down -v
   ```

### Connection String (Docker)

The Docker setup uses these default credentials:
```
host=localhost port=5432 dbname=healthtracker user=healthuser password=healthpass
```

### Accessing the Database

To connect via psql:
```bash
docker exec -it healthtracker-db psql -U healthuser -d healthtracker
```

---

## 📦 Manual Installation (Alternative)

If you prefer not to use Docker:

### macOS
```bash
brew install postgresql
brew services start postgresql
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install postgresql postgresql-contrib libpq-dev
sudo systemctl start postgresql
```

### Windows
Download and install from: https://www.postgresql.org/download/windows/

### Create Database and User

1. **Connect to PostgreSQL:**
   ```bash
   psql postgres
   ```

2. **Create database:**
   ```sql
   CREATE DATABASE healthtracker;
   ```

3. **Create user:**
   ```sql
   CREATE USER healthuser WITH PASSWORD 'yourpassword';
   GRANT ALL PRIVILEGES ON DATABASE healthtracker TO healthuser;
   ```

4. **Exit psql:**
   ```sql
   \q
   ```

### Connection String Format

Use this format in your code:
```
host=localhost port=5432 dbname=healthtracker user=healthuser password=yourpassword
```

## Usage in server.cpp

```cpp
#include "database.h"

int main() {
    // Initialize database connection
    // For Docker: use the connection string below
    // For manual install: adjust user/password as needed
    std::string connStr = "host=localhost port=5432 dbname=healthtracker user=healthuser password=healthpass";
    Database db(connStr);
    
    if (!db.initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    // Use database methods...
    // db.createUser("user@example.com", "hashed_password");
    // db.authenticateUser("user@example.com", "hashed_password");
}
```

## Notes

- **Docker**: Data persists in a Docker volume (`postgres_data`)
- The database tables will be created automatically when you call `initialize()`
- Make sure PostgreSQL is running before starting your server
- Store connection strings securely (consider environment variables for production)
- To change Docker credentials, edit `docker-compose.yml` and restart


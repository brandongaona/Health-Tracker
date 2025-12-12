# Use Alpine Linux with C++ tools
FROM alpine:latest

# Install g++, make, and postgres libraries (libpq)
RUN apk add --no-cache g++ make libpq-dev

# Set working directory
WORKDIR /app

# Copy all files
COPY . .

# COMPILE MANUALLY (No Makefile needed)
# We compile server.cpp and healthtracker.cpp into one executable named "server"
RUN g++ -std=c++17 server.cpp healthtracker.cpp -o server -lpq

# Expose the port
EXPOSE 8080

# Run the server
CMD ["./server"]
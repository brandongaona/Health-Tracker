# Building the C++ server

## Command

```bash
g++ -o server server.cpp healthtracker.cpp -std=c++17 -pthread -D_WIN32_WINNT=0x0A00 -lws2_32
```

On Windows the executable will be `server.exe`.

## Requirements

- **Headers (in project root):** `httplib.h` (cpp-httplib), `json.hpp` (nlohmann/json), `planner.h`
- **Compiler:** g++ (e.g. MSYS2 MinGW64 or MinGW-w64)
- **Windows:** `-D_WIN32_WINNT=0x0A00` targets Windows 10+ (needed for cpp-httplib).  
  `-lws2_32` links the Winsock library.

## If `build.bat` fails from cmd

Paths with spaces or parentheses can make g++ from cmd fail silently. Build from **MSYS2 MinGW 64-bit** instead:

1. Open **MSYS2 MinGW 64-bit** from the Start menu.
2. Go to your project folder (use your real path; backslashes are fine in `cd` with quotes):

   ```bash
   cd "/c/Users/Brandon Gaona/OneDrive/Stevens Semester 3/CS 385 (Akcam)/shared/healthtracker"
   ```

3. Run:

   ```bash
   g++ -o server.exe server.cpp healthtracker.cpp -std=c++17 -pthread -D_WIN32_WINNT=0x0A00 -lws2_32
   ```

## Missing headers

If you see missing `httplib.h` or `json.hpp`:

- **httplib.h:** [cpp-httplib](https://github.com/yhirose/cpp-httplib) — use the single-header `httplib.h` in the project root.
- **json.hpp:** [nlohmann/json](https://github.com/nlohmann/json) — use the single-header `json.hpp` from `include/nlohmann/json.hpp` in the project root.

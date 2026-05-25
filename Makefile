# =============================================================================
# Makefile - Hospital Queue System
# Compiler: MinGW (g++) on Windows
#
# CARA PAKAI:
#   1. Buka terminal (cmd/PowerShell) di folder ini
#   2. Jalankan: mingw32-make
#   3. Jalankan server: server.exe
#   4. Di terminal lain, jalankan: client.exe
# =============================================================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LIBS     = -lws2_32

all: server.exe client.exe
	@echo.
	@echo [OK] Kompilasi selesai!
	@echo Jalankan: server.exe
	@echo Lalu di terminal lain: client.exe

server.exe: HospitalServer.cpp Patient.h ManualLinkedList.h
	$(CXX) $(CXXFLAGS) -o server.exe HospitalServer.cpp $(LIBS)
	@echo [OK] server.exe berhasil dikompilasi

client.exe: HospitalClient.cpp
	$(CXX) $(CXXFLAGS) -o client.exe HospitalClient.cpp $(LIBS)
	@echo [OK] client.exe berhasil dikompilasi

clean:
	del /f server.exe client.exe 2>NUL || true

.PHONY: all clean

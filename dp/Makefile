# Makefile for Dpaper Application

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Wno-deprecated-declarations
DEBUG_FLAGS = -g -DDEBUG

# Package configurations
PKG_CONFIG = pkg-config
GTK_FLAGS = $(shell $(PKG_CONFIG) --cflags gtk+-3.0)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk+-3.0)
APPINDICATOR_FLAGS = $(shell $(PKG_CONFIG) --cflags ayatana-appindicator3-0.1)
APPINDICATOR_LIBS = $(shell $(PKG_CONFIG) --libs ayatana-appindicator3-0.1)

# Include directories
INCLUDES = -Iinclude

# Source files
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/config.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Target executable
TARGET = dpaper

# Default target
all: $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(GTK_LIBS) $(APPINDICATOR_LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(GTK_FLAGS) $(APPINDICATOR_FLAGS) $(INCLUDES) -c $< -o $@

# Create directories
dirs:
	mkdir -p $(SRCDIR)
	mkdir -p include
	mkdir -p data/icons

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt update
	sudo apt install -y libgtk-3-dev libayatana-appindicator3-dev

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Install the application
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo ln -sf /usr/local/bin/$(TARGET) /usr/local/bin/dp
	sudo mkdir -p /usr/share/icons/hicolor/48x48/apps/
	sudo cp data/icons/dpaper.png /usr/share/icons/hicolor/48x48/apps/dpaper.png
	sudo cp data/dpaper.desktop /usr/share/applications/
	sudo sh -c 'mkdir -p ~$(SUDO_USER)/Desktop && cp data/dpaper.desktop ~$(SUDO_USER)/Desktop/'

# Uninstall the application
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -f /usr/local/bin/dp
	sudo rm -f /usr/share/icons/hicolor/48x48/apps/dpaper.png
	sudo rm -f /usr/share/applications/dpaper.desktop
	sudo sh -c 'rm -f ~$(SUDO_USER)/Desktop/dpaper.desktop'

.PHONY: all debug clean rebuild install uninstall install-deps dirs
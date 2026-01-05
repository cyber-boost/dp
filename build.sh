#!/bin/bash

# Dpaper Build Script
# Entry point for building the application

echo "Dpaper Build System"
echo "==============================="

# Change to dp directory and run make
cd dp
exec make "$@"
#!/bin/bash

# R-Type Documentation Generator Script
# This script generates the Doxygen documentation for the R-Type project

set -e

# Get script directory and go to project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Banner
echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║     R-Type Documentation Generator                     ║"
echo "║     Powered by Doxygen                                 ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Check if Doxygen is installed
print_info "Checking for Doxygen installation..."
if ! command -v doxygen &> /dev/null; then
    print_error "Doxygen is not installed!"
    print_info "Installing Doxygen and Graphviz..."
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sudo apt-get update
        sudo apt-get install -y doxygen graphviz
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        brew install doxygen graphviz
    else
        print_error "Unsupported operating system. Please install Doxygen manually."
        exit 1
    fi
else
    DOXYGEN_VERSION=$(doxygen --version)
    print_success "Doxygen ${DOXYGEN_VERSION} is installed"
fi

# Check if Graphviz is installed
print_info "Checking for Graphviz installation..."
if ! command -v dot &> /dev/null; then
    print_warning "Graphviz is not installed. Diagrams will not be generated."
else
    print_success "Graphviz is installed"
fi

# Check if Doxyfile exists
if [ ! -f "Doxyfile" ]; then
    print_error "Doxyfile not found in doc folder!"
    exit 1
fi

# Clean previous documentation
print_info "Cleaning previous documentation..."
rm -rf doxygen/html doxygen/latex doxygen/xml
print_success "Cleaned previous documentation"

# Generate documentation
print_info "Generating Doxygen documentation..."

if doxygen Doxyfile; then
    print_success "Documentation generated successfully!"
    
    # Create .nojekyll file for GitHub Pages
    touch doc/doxygen/html/.nojekyll
    
    echo ""
    print_success "Documentation is ready!"
    print_info "Location: doc/doxygen/html/index.html"
    
else
    print_error "Documentation generation failed!"
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║     Documentation generation completed!                ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

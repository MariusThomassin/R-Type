# Doxygen Documentation

This directory contains the generated Doxygen documentation for the R-Type ECS Engine project.

## Viewing the Documentation

The documentation is automatically deployed to GitHub Pages when changes are pushed to the main branch.

You can access it at: `https://mariusthomassin.github.io/R-Type/`

## Generating Documentation Locally

To generate the documentation locally:

```bash
# Install Doxygen and Graphviz
sudo apt-get install doxygen graphviz

# Generate documentation from project root
doxygen Doxyfile

# Or use the convenience script
./doc/generate_docs.sh
```

The generated documentation will be available in the `doc/doxygen/html` directory.

## Documentation Contents

The complete documentation includes:

- **Classes & Structures**: All ECS components, systems, and engine classes
- **Namespaces**: Organization of code modules
- **Files**: Source file documentation
- **Class Hierarchy**: Inheritance diagrams
- **Collaboration Diagrams**: Component interactions
- **Call Graphs**: Function dependencies
- **Include Graphs**: File dependencies
- **Alphabetical Index**: Quick reference to all symbols

## Configuration

The Doxygen configuration is stored in the `Doxyfile` at the root of the project.

Key features enabled:
- Complete extraction of all code elements
- Private and static member documentation
- Internal documentation
- UML-style class diagrams
- Call and caller graphs
- Interactive SVG diagrams
- Source code browser
- Full search functionality

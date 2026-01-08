# How to Deploy Doxygen Documentation to GitHub Pages

## Prerequisites

- Repository must be hosted on GitHub
- You need admin access to the repository settings

## Setup Instructions

### Step 1: Enable GitHub Pages

1. Go to your repository on GitHub: `https://github.com/MariusThomassin/R-Type`
2. Click on **Settings** (gear icon at the top)
3. In the left sidebar, click on **Pages**
4. Under "Build and deployment", configure:
   - **Source**: Select "GitHub Actions"
5. Save the changes

### Step 2: Push Changes

```bash
git add .
git commit -m "Add Doxygen documentation and deployment workflow"
git push origin main
```

### Step 3: Access Documentation

Once deployed: `https://mariusthomassin.github.io/R-Type/`

## Local Testing

```bash
# Run the generation script
./doc/generate_docs.sh

# Or generate manually from project root
doxygen Doxyfile

# Open in browser
xdg-open doc/doxygen/html/index.html
```

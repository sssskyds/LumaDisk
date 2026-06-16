# LumaDisk

A from-scratch Qt/C++ storage cleaner focused on speed and UX.

## Highlights
- Fast background scanning
- Folder weight, largest files, file type summary, scan notes
- **Advanced Organizer** tab that can organize files inside all subfolders of the selected directory
- Organizer preview before moving files
- Rules: file type category, extension, modified year/month, size tier
- Conflict handling: auto-rename, skip, overwrite
- Output folder: `_Luma Organized`
- Optional preservation of original subfolder structure
- Windows GUI build with no cmd window

## Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```
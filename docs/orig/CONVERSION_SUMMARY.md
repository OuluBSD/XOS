# PDF Conversion Summary

PDF files in the docs directory have been converted to text and HTML formats for better AI readability.

## Original Files
- XOS Programmer's guide v3.2.pdf
- XOS User's guide v3.2.pdf

## Converted Files

### Text Format (for easy reading and search)
- XOS Programmer's guide v3.2.txt
- XOS User's guide v3.2.txt

### HTML Format (preserving structure and links)
- XOS Programmer's guide v3.2-html.html
- XOS Programmer's guide v3.2-outline.html
- XOS User's guide v3.2-html.html
- XOS User's guide v3.2-outline.html

### Metadata
- converted_docs_metadata.json - Contains information about the conversion process and file details

## Conversion Tools Used
- pdftotext: Creates plain text files with preserved layout
- pdftohtml: Creates HTML files with preserved structure and links

## Notes
The pdftohtml command also generated image files for each page which are not needed for AI reading but are preserved in case they are needed for reference. The HTML files contain the complete document structure and are recommended for the best formatted reading experience.

For AI processing, the .txt files provide the cleanest text format without any formatting complications.
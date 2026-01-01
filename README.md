# Poketrex
A Pokemon-style demo for the Vectrex, programmed in C with the use of CMOC.

# Build
The .bin file can be generated, once CMOC is installed (tested on CMOC version 0.1.97) with the command;
" cmoc --vectrex -I. -o POKETREX.bin poketrex_main.c "
Note as well that many of the headers seen in the 'vectrex_files' folder are provided directly from the 'cmoc/includes' directly provided on installation.

# Running
This program has only been tested on the VIDE and ParaJVE vectrex emulators. Once compiled into a .bin file, select the 'POKETREX.bin' file (in VIDE, this can be done by selecting the folder icon in the top left corner of the 'vecxi' window) and run.

# Helper Programs
This repository also includes a helper program used to generate vector images from provided images. It can be run in Processing, where the supplied image to replicate must be placed in the the '/data' folder inside 'helper_programs'. The controls can be viewed inside the program or using the instructions when ran. The first version of this file allows for custom vectors to be placed or a dot method to be used (very inefficent). The second version of the program will generate the vectors automatically from the input image using side-scrolling vectors of varying intensity. This second approach seems to work much better but will still have performance issues (especially when compiling) for small resolutions or large images so be warned.
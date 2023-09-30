# otoolrecursive

Command line tool which lists refernced dylibs in a given dylib recursively
Usage: otoolrecursive [-vprelo]  <dynlib file>

Options are:

- v verbose output
- p only lists dylibs which are part of Homebrew or Macports package manager
- r only lists dylibs which are referenced by @rpath
- l only lists dylibs which are referenced by @loader_path
- e only lists dylibs which are referenced by @executbale_path
- o only lists all other dylibs

If neither of the options p,r,l,e,o are given, all dylbs will be listed

Currently dylibs are  listed under packages if they reside in the subtree of /opt/homebrew or /opt/local
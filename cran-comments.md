## vol2birdR 0.2.0
This version is the initial release of this package,
resubmitting after initial review by Benjamin Altmann

Fixes include:
* quotes added for software and package names
* authors added to the reference in DESCRIPTION
* all main authors of dependency software are now listed as contributors
* a COPYRIGHTS file is added listing all license information for depenencies, 
  and listing the names of any contributors found in these dependencies.
* we found a piece of code found in the rsl2odim dependency used for
  doubly linked lists that was under copyright. The current package includes
  our own implementation for doubly linked lists, and the copyrighted code
  was removed
* We added documentation on return values for all exported functions
* We added a test radar file, to be used for testing the main vol2bird function
* We added examples for all exported functions.

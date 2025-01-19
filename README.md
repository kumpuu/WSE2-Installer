### WSE2 Installer
- Doesn't create registry entries or an uninstaller. It's more of an extractor.
- Can add WSE2 to users steam library
- Can copy profiles.dat

To update included WSE, simply change contents of ./files/WSE/  
and compile with "Inno\iscc.exe setup.iss /DMyAppVersion=version_number_here"

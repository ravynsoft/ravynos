import os
import sys

if hasattr(os, 'add_dll_directory'):
    os.add_dll_directory(os.path.join(os.getcwd(), '..', '..', '.libs'))

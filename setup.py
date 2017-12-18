import os
import glob
from distutils.core import setup, Extension
import numpy

try:
    # for python 3, let 2to3 do most of the work
    from distutils.command.build_py import build_py_2to3 as build_py
except ImportError:
    # for python 2 don't apply any transformations
    from distutils.command.build_py import build_py


scripts=[
    'meds-extract-range',
    'meds-extract-catalog',
    'meds-view',
    'meds-compare',
]
scripts=[os.path.join('./scripts', s) for s in scripts]

sources=["meds/_uberseg.c"]
include_dirs=[numpy.get_include()]
ext=Extension("meds._uberseg", sources, include_dirs=include_dirs)

setup(name="meds", 
      version="0.9.3rc2",
      description="Python and C libraries for reading MEDS files",
      license = "GNU GPLv3",
      author="Erin Scott Sheldon",
      author_email="erin.sheldon@gmail.com",
      packages=['meds'],
      ext_modules=[ext],
      include_dirs=include_dirs,
      scripts=scripts,
      cmdclass={'build_py': build_py})


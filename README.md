meds
====

Python and C libraries to work with Multi Epoch Data Structures

Description of the file format here
    https://cdcvs.fnal.gov/redmine/projects/deswlwg/wiki/Multi_Epoch_Data_Structure

In addition to the libraries listed below, a program called make-meds-input is
also installed under $prefix/bin.  This program converts a fits file to 
an input file for the make-cutouts program (hosted elsewhere).

python library
-----------------------------

This is a pure python library.  For reading the FITS files, the fitsio python
package is used.  fitsio https://github.com/esheldon/fitsio

MEDS python Docs are here
    https://github.com/esheldon/meds/blob/master/meds/meds.py

installing the python library
#############################

    in the usual place
        python setup.py install

    at a different prefix
        python setup.py install --prefix=/some/path

    make sure it is on your PYTHONPATH

examples
########
```python

import meds

# create a MEDS object for the given MEDS file
m=meds.MEDS(filename)

# read a cutout for object 35, cutout index 5
index=35
cutout_index=5
image=m.get_cutout(index, cutout_index)

# the coadd is always cutout_index=0
cutout_index=0
coadd=m.get_cutout(index, cutout_index)

# get other image types
seg=m.get_cutout(index, cutout_index, type=’seg’)
wt=m.get_cutout(index, cutout_index, type=’weight’)
mask=m.get_cutout(index, cutout_index, type=’bmask’)

# get a list of all cutouts for this object
imlist=m.get_cutout_list(index)
seglist=m.get_cutout_list(index,type=’seg’)

# The contents of the object data table is loaded when the MEDS object is
# created, and are accessible by name.

# number of cutouts
ncutout=m[’ncutout’][index]
for i in xrange(ncutout):
    imlist=m.get_cutout_list(i)
    # process the images
```

C library
------------------------

This is a pure C library for working with MEDS.  Docs here
    https://github.com/esheldon/meds/blob/master/src/meds.h
The only requirement is an installation of cfitsio and gcc.

installing the C library
#############################

    in the usual place
        make install

    in a different prefix
        make install prefix=/some/path

linking to the library
#######################

Include "meds.h" and use the following to link against the library.  Make sure
to get the order correct

    CC  ... -lmeds -lcfitsio -lm ...


testing
---------
You can test your build of the C libary using

    ./src/test $medsfile

Where $medsfile is the name of a MEDS fits file.

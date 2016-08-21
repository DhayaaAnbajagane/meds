// vim: set tabstop=8 shiftwidth=2 :
// above to match Matt's settings
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <Python.h>
#include <numpy/arrayobject.h>

static PyObject * uberseg_direct(PyObject* self, PyObject* args) {
  int dmin=0,d=0,x=0,y=0,k=0,imin=0; 

  PyObject* seg = NULL;
  PyObject* weight = NULL;
  int Nx;
  int Ny;
  int object_number;
  PyObject* obj_inds_x = NULL;
  PyObject* obj_inds_y = NULL;
  int Ninds;
  int *ptrx,*ptry,*ptrs;
  float *ptrw;
  int dx;
  
  if (!PyArg_ParseTuple(args, (char*)"OOiiiOOi", &seg, &weight, &Nx, &Ny, &object_number, &obj_inds_x, &obj_inds_y, &Ninds)) {
    return NULL;
  }
  
  for(x=0;x<Nx;++x) {
    for(y=0;y<Ny;++y) {

      //shortcuts
      ptrs = (int*)PyArray_GETPTR2(seg,x,y);
      if(*ptrs == object_number)
	continue;
      
      if(*ptrs > 0 && *ptrs != object_number) {
	ptrw = (float*)PyArray_GETPTR2(weight,x,y);
	*ptrw = 0.0;
	continue;
      }

      //must do direct search
      imin = -1;
      for(k=0;k<Ninds;++k) {
	ptrx = (int*)PyArray_GETPTR1(obj_inds_x,k);
	ptry = (int*)PyArray_GETPTR1(obj_inds_y,k);
	dx = x-(*ptrx);
	d = dx*dx;
	dx = y-(*ptry);
	d += dx*dx;
	if(d < dmin || imin == -1) {
	  dmin = d;
	  imin = k;
	}
      }

      if(imin == -1) {
	continue;
      }
      
      ptrx = (int*)PyArray_GETPTR1(obj_inds_x,imin);
      ptry = (int*)PyArray_GETPTR1(obj_inds_y,imin);
      ptrs = (int*)PyArray_GETPTR2(seg,*ptrx,*ptry);

      if(*ptrs != object_number) {
	ptrw = (float*)PyArray_GETPTR2(weight,x,y);
	*ptrw = 0.0;
      }
    }
  }
  
  Py_INCREF(Py_None);
  return Py_None;
}

#include <inttypes.h>
#define FAST3TREE_FLOATTYPE float
struct mytype{int64_t idx; int64_t seg; FAST3TREE_FLOATTYPE pos[2];};
#define FAST3TREE_TYPE struct mytype
#define FAST3TREE_DIM 2
#include "fast3tree.c"

static PyObject * uberseg_tree(PyObject* self, PyObject* args) {
  int x=0,y=0,k=0,segmin=0;
  float dmin=0,r=0,dx=0,d=0;
  float pos[2],fac=0;

  PyObject* seg = NULL;
  PyObject* weight = NULL;
  int Nx;
  int Ny;
  int object_number;
  PyObject* obj_inds_x = NULL;
  PyObject* obj_inds_y = NULL;
  int Ninds;
  int *ptrx,*ptry,*ptrs;
  float *ptrw;
  
  struct mytype *obj = NULL;
  struct fast3tree *tree = NULL;
  struct fast3tree_results *res = NULL;
  
  if (!PyArg_ParseTuple(args, (char*)"OOiiiOOi", &seg, &weight, &Nx, &Ny, &object_number, &obj_inds_x, &obj_inds_y, &Ninds)) {
    return NULL;
  }

  obj = (struct mytype *)malloc(sizeof(struct mytype)*Ninds);
  if(obj == NULL) exit(1);
  
  for(k=0;k<Ninds;++k) {
    ptrx = (int*)PyArray_GETPTR1(obj_inds_x,k);
    ptry = (int*)PyArray_GETPTR1(obj_inds_y,k);
    ptrs = (int*)PyArray_GETPTR2(seg,*ptrx,*ptry);
    obj[k].idx = k;
    obj[k].seg = *ptrs;
    obj[k].pos[0] = (float)(*ptrx);
    obj[k].pos[1] = (float)(*ptry);
  }
  
  tree = fast3tree_init(Ninds,obj);
  if(tree == NULL) exit(1);

  res = fast3tree_results_init();    
  if(res == NULL) exit(1);

  float maxr = 1.1*sqrt(Nx*Nx+Ny*Ny);
    
  for(x=0;x<Nx;++x) {
    for(y=0;y<Ny;++y) {
      
      //shortcuts
      ptrs = (int*)PyArray_GETPTR2(seg,x,y);
      if(*ptrs == object_number)
	continue;
      
      if(*ptrs > 0 && *ptrs != object_number) {
	ptrw = (float*)PyArray_GETPTR2(weight,x,y);
	*ptrw = 0.0;
	continue;
      }
      
      //must do search
      pos[0] = (float)x;
      pos[1] = (float)y;
      r = fast3tree_find_next_closest_distance(tree,res,pos);
      fac = 1.0/1.1;
      do {
	fac *= 1.1;
	fast3tree_results_clear(res);
	fast3tree_find_sphere(tree,res,pos,r*fac);
      } while(res->num_points == 0 && r*fac <= maxr);
      
      segmin = -1;
      for(k=0;k<res->num_points;++k) {
	dx = res->points[k]->pos[0]-x;
	d = dx*dx;
	dx = res->points[k]->pos[1]-y;
	d += dx*dx;
	if(segmin == -1 || d < dmin) {
	  segmin = res->points[k]->seg;
	  dmin = d;
	}
      }
      
      if(segmin == -1) {
	continue;
      }
      
      if(segmin != object_number) {
	ptrw = (float*)PyArray_GETPTR2(weight,x,y);
	*ptrw = 0.0;
      }
    }
  }

  free(obj);
  fast3tree_free(&tree);
  fast3tree_results_free(res);
 
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef methods[] = {
  {"uberseg_direct", (PyCFunction)uberseg_direct, METH_VARARGS, "fast uberseg\n"},
  {"uberseg_tree", (PyCFunction)uberseg_tree, METH_VARARGS, "fast uberseg w/ tree\n"},
  {NULL}  /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_uberseg",      /* m_name */
        "Define c version of uberseg",  /* m_doc */
        -1,                  /* m_size */
        methods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif


#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit__uberseg(void) 
#else
init_uberseg(void) 
#endif
{
  PyObject* m = NULL;

  
#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
    if (m==NULL) {
        return NULL;
    }

#else

    m = Py_InitModule3("_uberseg", methods, "uberseg methods");
    if(m == NULL) {
      return;
    }

#endif


  import_array();

#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}



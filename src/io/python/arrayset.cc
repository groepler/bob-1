/**
 * @file io/python/arrayset.cc
 * @date Wed Jun 22 17:50:08 2011 +0200
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief Python bindings for bob::io::Arrayset
 *
 * Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/python.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/format.hpp>
#include <blitz/array.h>

#include "bob/io/Arrayset.h"
#include "bob/core/array_assert.h"
#include "bob/core/python/exception.h"

#include "bob/core/python/ndarray.h"

using namespace boost::python;
namespace io = bob::io;
namespace core = bob::core;
namespace ca = bob::core::array;
namespace tp = bob::python;

static object arrayset_dtype (const io::Arrayset& s) {
  const ca::typeinfo& info = s.type();
  if (info.is_valid()) return tp::dtype(info.dtype).self();
  return object();
}

static void extend_with_iterable(io::Arrayset& s, object iter) {
  object dtype = arrayset_dtype(s);
  stl_input_iterator<object> end;
  for (stl_input_iterator<object> it(iter); it != end; ++it) {
    if (TPY_ISNONE(dtype)) {
      s.add(io::Array(boost::make_shared<tp::py_array>(*it, object())));
      dtype = arrayset_dtype(s);
    }
    else {
      s.add(io::Array(boost::make_shared<tp::py_array>(*it, dtype)));
    }
  }
}

static void extend_with_ndarray(io::Arrayset& s, numeric::array a, size_t D) {
  PyArrayObject* arr = (PyArrayObject*)a.ptr();

  size_t ndim = PyArray_NDIM(arr);

  if (D >= ndim) {
    PYTHON_ERROR(RuntimeError, "iteration axis has be < len(array.shape)");
  }

  if (ndim < 2) {
    PYTHON_ERROR(RuntimeError, "the minimum number of dimensions for the input array has to be 2");
  }

  if (ndim > (BOB_MAX_DIM+1)) {
    PYTHON_ERROR(RuntimeError, "the maximum number of dimensions for the array to be iterated on is BOB_MAX_DIM+1 - you have exceeded that value");
  }

  //reads the default type, if any
  object dtype = arrayset_dtype(s);

  //the indexing
  list index;
  for (size_t i=0; i<ndim; ++i) index.append(slice());

  for (npy_intp k=0; k<PyArray_DIMS(arr)[D]; ++k) {
    //gets a slice of the array, makes a contiguous copy of it and append
    index[D] = k;
    s.add(io::Array(boost::make_shared<tp::py_array>(a[index], dtype)));
    if (TPY_ISNONE(dtype)) dtype = arrayset_dtype(s); ///< may need a refresh
  }
}

static boost::shared_ptr<io::Arrayset> make_from_array_iterable1(object iter) {
  boost::shared_ptr<io::Arrayset> retval = boost::make_shared<io::Arrayset>();
  object dtype = object();
  stl_input_iterator<object> end;
  for (stl_input_iterator<object> it(iter); it != end; ++it) {
    if (TPY_ISNONE(dtype)) {
      retval->add(io::Array(boost::make_shared<tp::py_array>(*it, object())));
      dtype = arrayset_dtype(*retval);
    }
    else {
      retval->add(io::Array(boost::make_shared<tp::py_array>(*it, dtype)));
    }
  }
  return retval;
}

static boost::shared_ptr<io::Arrayset> make_from_array_iterable2(object iter,
    object dtype) {
  boost::shared_ptr<io::Arrayset> retval = boost::make_shared<io::Arrayset>();
  stl_input_iterator<object> end;
  for (stl_input_iterator<object> it(iter); it != end; ++it) {
    if (TPY_ISNONE(dtype)) {
      retval->add(io::Array(boost::make_shared<tp::py_array>(*it, object())));
      dtype = arrayset_dtype(*retval);
    }
    else {
      retval->add(io::Array(boost::make_shared<tp::py_array>(*it, dtype)));
    }
  }
  return retval;
}

template <typename T>
static void fill_with_2d_array(io::Arrayset& s, tp::const_ndarray input) {
  const blitz::Array<T,2> bz = input.bz<T,2>();
  for (int i=0; i<bz.extent(0); ++i) {
    const blitz::Array<T,1> bztmp = bz(i, blitz::Range::all());
    s.add(bztmp); ///< uses the copy variant implicitly
  }
}

struct arrayset_from_ndarray {

  arrayset_from_ndarray() {
    converter::registry::push_back(&convertible, &construct, 
        type_id<io::Arrayset>());
  }

  static void* convertible(PyObject* obj_ptr) {
    handle<> hdl(borrowed(obj_ptr));
    object obj(hdl);
    if (tp::convertible_to(obj, false, true)) { //writeable=false, behaved=true
      handle<> hdl(borrowed(obj_ptr));
      object tmp(hdl);
      tp::const_ndarray pyarray(tmp);
      ca::typeinfo info = pyarray.type();

      if (info.nd != 2) return 0;

      switch (info.dtype) {
        case ca::t_int8:
        case ca::t_int16:
        case ca::t_int32:
        case ca::t_int64:
        case ca::t_uint8:
        case ca::t_uint16:
        case ca::t_uint32:
        case ca::t_uint64:
        case ca::t_float32:
        case ca::t_float64:
        case ca::t_complex128:
        case ca::t_complex256:
          return obj_ptr;
        default:
          return 0;
      }

      return obj_ptr;
    }
    return 0;
  }

  static void construct(PyObject* obj_ptr,
      converter::rvalue_from_python_stage1_data* data) {

    //setup destination Arrayset
    void* storage = ((converter::rvalue_from_python_storage<io::Arrayset>*)data)->storage.bytes;
    new (storage) io::Arrayset();
    data->convertible = storage;
    io::Arrayset& result = *((io::Arrayset*)storage);

    //convert input obj_ptr
    handle<> hdl(borrowed(obj_ptr));
    object tmp(hdl);
    tp::const_ndarray pyarray(tmp);

    //create const_ndarray
    switch (pyarray.type().dtype) {
      case ca::t_int8: 
        fill_with_2d_array<int8_t>(result, pyarray);
        break;
      case ca::t_int16: 
        fill_with_2d_array<int16_t>(result, pyarray);
        break;
      case ca::t_int32: 
        fill_with_2d_array<int32_t>(result, pyarray);
        break;
      case ca::t_int64: 
        fill_with_2d_array<int64_t>(result, pyarray);
        break;
      case ca::t_uint8: 
        fill_with_2d_array<uint8_t>(result, pyarray);
        break;
      case ca::t_uint16: 
        fill_with_2d_array<uint16_t>(result, pyarray);
        break;
      case ca::t_uint32: 
        fill_with_2d_array<uint32_t>(result, pyarray);
        break;
      case ca::t_uint64: 
        fill_with_2d_array<uint64_t>(result, pyarray);
        break;
      case ca::t_float32: 
        fill_with_2d_array<float>(result, pyarray);
        break;
      case ca::t_float64: 
        fill_with_2d_array<double>(result, pyarray);
        break;
      case ca::t_complex128: 
        fill_with_2d_array<std::complex<float> >(result, pyarray);
        break;
      case ca::t_complex256: 
        fill_with_2d_array<std::complex<double> >(result, pyarray);
        break;
      default:
        break;
    }
  }

};

static io::Array get_array(io::Arrayset& s, size_t index) {
  return s[index];
}

static void set_array(io::Arrayset& s, size_t index, const io::Array& a) {
  s.set(index, a);
}

static void append_array(io::Arrayset& s, const io::Array& a) {
  s.add(a);
}

static object get_ndarray(io::Arrayset& s, size_t index) {
  return tp::py_array(s[index].get()).pyobject();
}

static void set_ndarray(io::Arrayset& s, size_t index, object o) {
  s.set(index, io::Array(tp::py_array(o, arrayset_dtype(s))));
}

static void set_string(io::Arrayset& s, size_t index, const std::string& filename) {
  s.set(index, io::Array(filename));
}

static void append_ndarray(io::Arrayset& s, object o, object dtype=object()) {
  if (TPY_ISNONE(dtype)) dtype = arrayset_dtype(s); ///try something better...
  s.add(io::Array(boost::make_shared<tp::py_array>(o, dtype)));
}

BOOST_PYTHON_FUNCTION_OVERLOADS(append_ndarray_overloads, append_ndarray, 2, 3)

static void append_string(io::Arrayset& s, const std::string& filename) {
  s.add(io::Array(filename));
}

void bind_io_arrayset() {
  class_<io::Arrayset, boost::shared_ptr<io::Arrayset> >("Arrayset", "Arraysets represent lists of Arrays that share the same element type and dimension properties.", no_init)
    
    //attention: order here is crucial - last defined is tried first by the
    //boost::python overloading resolution system; the last thing we want to
    //try are the numpy handlers.
    .def("__init__", make_constructor(make_from_array_iterable1, default_call_policies(), (arg("iterable"))), "Creates a new Arrayset from a python iterable containing array-like objects. In this mode the first object will be coerced into a numpy ndarray if that is not already the case and all following objects in the iterable will be coerced to the same type as the first object.")
    .def("__init__", make_constructor(make_from_array_iterable2, default_call_policies(), (arg("iterable"),arg("dtype"))), "Creates a new Arrayset from a python iterable containing array-like objects, with an optional coertion dtype that will be applied to all arrays. If the coertion type 'dtype' is None, than this falls back to the case described in the constructor with iterables w/o dtype specification.")
    .def(init<boost::shared_ptr<io::File>, optional<size_t,size_t> >((arg("file"), arg("begin"), arg("end")), "Start with all or some arrays in a given file. You can select the start and/or the end. Numbers past the end of the given file are ignored. For example, if a file contains 5 arrays, this constructor will work ok if you leave 'end' on its default (maximum possible unsigned integer)."))
    .def(init<const std::string&, optional<char> >((arg("filename"), arg("mode")='r'), "Initializes a new arrayset from an external file."))
    .def(init<>("Creates a new empty arraset with an inlined representation."))
    
    .add_property("type", make_function(&io::Arrayset::type, return_value_policy<copy_const_reference>()), "Typing information for this arrayset")
    .def("save", &io::Arrayset::save, (arg("self"), arg("filename")), "Saves the arrayset into a single file. The file is truncated before this operation.")
    .def("load", &io::Arrayset::load, "Loads all Arrays into memory, if that is not the case")
    .def("get", &get_array, (arg("self"), arg("index")), "Gets an array from this set given its id")
    
    //some list-like entries
    .def("__len__", &io::Arrayset::size, "The number of arrays stored in this set.")
    .def("__getitem__", &get_ndarray, (arg("self"), arg("index")), "Gets a numpy array given its relative position")
    
    //attention: order here is crucial - last defined is tried first by the
    //boost::python overloading resolution system; the last thing we want to
    //try are the numpy handlers.
    .def("__setitem__", &set_ndarray, (arg("self"), arg("index"), arg("array")), "Sets an array-like in the set. May raise an exception if there is no such index in the set. A coertion will take place if types differ1")
    .def("__setitem__", &set_array, (arg("self"), arg("index"), arg("array")), "Sets an array in the set. May raise an exception if there is no such index in the set.")
    .def("__setitem__", &set_string, (arg("self"), arg("index"), arg("filename")), "Sets an array encoded in a file at the set. May raise an exception if there is no such index in the set.")
    
    .def("__delitem__", &io::Arrayset::remove, (arg("self"), arg("index")), "Removes the array given its id. May raise an exception if there is no such array inside.")
    .def("append", &append_ndarray, append_ndarray_overloads((arg("self"), arg("array"), arg("dtype")), "Appends an array-like object to the set. This version allows you to set a coertion type to be applied to the input array."))
    .def("append", &append_array, (arg("self"), arg("array")), "Adds an array to the set")
    .def("append", &append_string, (arg("self"), arg("filename")), "Appends an array encoded in a file to the set")

    //attention: order here is crucial - last defined is tried first by the
    //boost::python overloading resolution system; the last thing we want to
    //try are the iterable handlers
    .def("extend", &extend_with_ndarray, (arg("self"), arg("array"), arg("axis")), "Extends this array set with an ndarray one more dimension than what my type has (or will have), iterating over the specified axis. If this arrayset is not new, extension may be subject to type coertion.")
    .def("extend", &extend_with_iterable, (arg("self"), arg("iterable")), "Extends this array set with array-like objects coming from the given iterable. The current arrayset dtype is inforced on all required coertions.")
    ;

  arrayset_from_ndarray();
}
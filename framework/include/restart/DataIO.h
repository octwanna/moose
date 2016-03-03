/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DATAIO_H
#define DATAIO_H

// MOOSE includes
#include "MooseTypes.h"
#include "HashMap.h"
#include "MooseError.h"

// libMesh includes
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/parallel.h"
#ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
#  include <type_traits>
#endif

// C++ includes
#include <string>
#include <vector>
#include <iostream>
#include <map>

// Forward declarations
class ColumnMajorMatrix;
namespace libMesh
{
template <typename T> class NumericVector;
template <typename T> class DenseMatrix;
template <typename T> class DenseVector;
class Elem;
}

/**
 * Scalar helper routine
 */
template<typename P>
inline void storeHelper(std::ostream & stream, P & data, void * context);

/**
 * Vector helper routine
 */
template<typename P>
inline void storeHelper(std::ostream & stream, std::vector<P> & data, void * context);

/**
 * Shared pointer helper routine
 */
template<typename P>
inline void storeHelper(std::ostream & stream, MooseSharedPointer<P> & data, void * context);

/**
 * Set helper routine
 */
template<typename P>
inline void storeHelper(std::ostream & stream, std::set<P> & data, void * context);

/**
 * Map helper routine
 */
template<typename P, typename Q>
inline void storeHelper(std::ostream & stream, std::map<P,Q> & data, void * context);

/**
 * HashMap helper routine
 */
template<typename P, typename Q>
inline void storeHelper(std::ostream & stream, HashMap<P,Q> & data, void * context);

/**
 * Scalar helper routine
 */
template<typename P>
inline void loadHelper(std::istream & stream, P & data, void * context);

/**
 * Vector helper routine
 */
template<typename P>
inline void loadHelper(std::istream & stream, std::vector<P> & data, void * context);

/**
 * SharedPointer helper routine
 */
template<typename P>
inline void loadHelper(std::istream & stream, MooseSharedPointer<P> & data, void * context);

/**
 * Set helper routine
 */
template<typename P>
inline void loadHelper(std::istream & stream, std::set<P> & data, void * context);

/**
 * Map helper routine
 */
template<typename P, typename Q>
inline void loadHelper(std::istream & stream, std::map<P,Q> & data, void * context);

/**
 * Hashmap helper routine
 */
template<typename P, typename Q>
inline void loadHelper(std::istream & stream, HashMap<P,Q> & data, void * context);

template<typename T>
inline void
dataStore(std::ostream & stream, T & v, void * /*context*/);

// global store functions

template<typename T>
inline void
dataStore(std::ostream & stream, T & v, void * /*context*/)
{
  #ifdef LIBMESH_HAVE_CXX11_TYPE_TRAITS
    static_assert( std::is_polymorphic<T>::value == false,
      "Cannot serialize a class that has virtual members!\nWrite a custom dataStore() template specialization!\n\n");
    static_assert( std::is_trivially_copyable<T>::value || std::is_same<T, Point>::value,
      "Cannot serialize a class that is not trivially copyable!\nWrite a custom dataStore() template specialization!\n\n");
  #endif

  // Moose::out<<"Generic dataStore"<<std::endl;
  stream.write((char *) &v, sizeof(v));
}

template<typename T>
inline void
dataStore(std::ostream & /*stream*/, T * & /*v*/, void * /*context*/)
{
  mooseError("Cannot store raw pointers as restartable data!\nWrite a custom dataStore() template specialization!\n\n");
}

template<typename T, typename U>
inline void
dataStore(std::ostream & stream, std::pair<T, U> & p, void * context)
{
  dataStore(stream, p.first, context);
  dataStore(stream, p.second, context);
}

template<typename T>
inline void
dataStore(std::ostream & stream, std::vector<T> & v, void * context)
{
  // Moose::out<<"Vector dataStore"<<std::endl;

  // First store the size of the vector
  unsigned int size = v.size();
  stream.write((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
    storeHelper(stream, v[i], context);
}

template<typename T>
inline void
dataStore(std::ostream & stream, MooseSharedPointer<T> & v, void * context)
{
  // Moose::out<<"SharedPointer dataStore"<<std::endl;

  T * tmp = v.get();

  storeHelper(stream, tmp, context);
}

template<typename T>
inline void
dataStore(std::ostream & stream, std::set<T> & s, void * context)
{
  // Moose::out<<"Set dataStore"<<std::endl;

  // First store the size of the set
  unsigned int size = s.size();
  stream.write((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  typename std::set<T>::iterator it = s.begin();
  typename std::set<T>::iterator end = s.end();

  for (; it != end; ++it)
  {
    T & x = const_cast<T&>(*it);
    storeHelper(stream, x, context);
  }

}

template<typename T, typename U>
inline void
dataStore(std::ostream & stream, std::map<T,U> & m, void * context)
{
  // Moose::out<<"Map dataStore"<<std::endl;

  // First store the size of the map
  unsigned int size = m.size();
  stream.write((char *) &size, sizeof(size));

  //// Moose::out<<"Size: "<<size<<std::endl;

  typename std::map<T,U>::iterator it = m.begin();
  typename std::map<T,U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    // Moose::out<<"First"<<std::endl;
    // Moose::out<<"Key: "<<it->first<<std::endl;

    T & key = const_cast<T&>(it->first);

    storeHelper(stream, key, context);

    // Moose::out<<"Second"<<std::endl;
    storeHelper(stream, it->second, context);
  }
}

template<typename T, typename U>
inline void
dataStore(std::ostream & stream, HashMap<T,U> & m, void * context)
{
  // Moose::out<<"HashMap dataStore"<<std::endl;

  // First store the size of the map
  unsigned int size = m.size();
  stream.write((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  typename HashMap<T,U>::iterator it = m.begin();
  typename HashMap<T,U>::iterator end = m.end();

  for (; it != end; ++it)
  {
    // Moose::out<<"First"<<std::endl;
    // Moose::out<<"Key: "<<it->first<<std::endl;

    T & key = const_cast<T&>(it->first);

    storeHelper(stream, key, context);

    // Moose::out<<"Second"<<std::endl;
    storeHelper(stream, it->second, context);
  }
}

// Specializations (defined in .C)
template<> void dataStore(std::ostream & stream, Real & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, std::string & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, NumericVector<Real> & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, DenseVector<Real> & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, DenseMatrix<Real> & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, ColumnMajorMatrix & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, RealTensorValue & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, RealVectorValue & v, void * /*context*/);
template<> void dataStore(std::ostream & stream, const Elem * & e, void * context);
template<> void dataStore(std::ostream & stream, const Node * & n, void * context);
template<> void dataStore(std::ostream & stream, Elem * & e, void * context);
template<> void dataStore(std::ostream & stream, Node * & n, void * context);
template<> void dataStore(std::ostream & stream, std::stringstream & s, void * context);
template<> void dataStore(std::ostream & stream, std::stringstream * & s, void * context);

// global load functions

template<typename T>
inline void
dataLoad(std::istream & stream, T & v, void * /*context*/)
{
  // Moose::out<<"Generic dataLoad"<<std::endl;

  stream.read((char *) &v, sizeof(v));
}

template<typename T>
void dataLoad(std::istream & /*stream*/, T * & /*v*/, void * /*context*/)
{
  mooseError("Cannot load raw pointers as restartable data!\nWrite a custom dataLoad() template specialization!\n\n");
}

template<typename T, typename U>
inline void
dataLoad(std::istream & stream, std::pair<T, U> & p, void * context)
{
  dataLoad(stream, p.first, context);
  dataLoad(stream, p.second, context);
}

template<typename T>
inline void
dataLoad(std::istream & stream, std::vector<T> & v, void * context)
{
  //// Moose::out<<"Vector dataLoad"<<std::endl;

  // First read the size of the vector
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  v.resize(size);

  for (unsigned int i = 0; i < size; i++)
    loadHelper(stream, v[i], context);
}

template<typename T>
inline void
dataLoad(std::istream & stream, MooseSharedPointer<T> & v, void * context)
{
  T * tmp = v.get();

  loadHelper(stream, tmp, context);
}

template<typename T>
inline void
dataLoad(std::istream & stream, std::set<T> & s, void * context)
{
  // Moose::out<<"Set dataLoad"<<std::endl;

  // First read the size of the set
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  //// Moose::out<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
  {
    T data;
    loadHelper(stream, data, context);
    s.insert(data);
  }
}

template<typename T, typename U>
inline void
dataLoad(std::istream & stream, std::map<T,U> & m, void * context)
{
  // Moose::out<<"Map dataLoad"<<std::endl;

  m.clear();

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U & value = m[key];
    loadHelper(stream, value, context);
  }
}


template<typename T, typename U>
inline void
dataLoad(std::istream & stream, HashMap<T,U> & m, void * context)
{
  // Moose::out<<"HashMap dataLoad"<<std::endl;

  // m.clear();

  // First read the size of the map
  unsigned int size = 0;
  stream.read((char *) &size, sizeof(size));

  // Moose::out<<"Size: "<<size<<std::endl;

  for (unsigned int i = 0; i < size; i++)
  {
    T key;
    loadHelper(stream, key, context);

    U & value = m[key];
    loadHelper(stream, value, context);
  }
}

// Specializations (defined in .C)
template<> void dataLoad(std::istream & stream, Real & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, std::string & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, NumericVector<Real> & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, DenseVector<Real> & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, DenseMatrix<Real> & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, ColumnMajorMatrix & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, RealTensorValue & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, RealVectorValue & v, void * /*context*/);
template<> void dataLoad(std::istream & stream, const Elem * & e, void * context);
template<> void dataLoad(std::istream & stream, const Node * & e, void * context);
template<> void dataLoad(std::istream & stream, Elem * & e, void * context);
template<> void dataLoad(std::istream & stream, Node * & e, void * context);
template<> void dataLoad(std::istream & stream, std::stringstream & s, void * context);
template<> void dataLoad(std::istream & stream, std::stringstream * & s, void * context);

// Scalar Helper Function
template<typename P>
inline void
storeHelper(std::ostream & stream, P & data, void * context)
{
  // Moose::out<<"Generic storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// Vector Helper Function
template<typename P>
inline void
storeHelper(std::ostream & stream, std::vector<P> & data, void * context)
{
  // Moose::out<<"Vector storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// MooseSharedPointer Helper Function
template<typename P>
inline void
storeHelper(std::ostream & stream, MooseSharedPointer<P> & data, void * context)
{
  // Moose::out<<"MooseSharedPointer storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// Set Helper Function
template<typename P>
inline void
storeHelper(std::ostream & stream, std::set<P> & data, void * context)
{
  // Moose::out<<"Set storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// Map Helper Function
template<typename P, typename Q>
inline void
storeHelper(std::ostream & stream, std::map<P,Q> & data, void * context)
{
  //// Moose::out<<"Map storeHelper"<<std::endl;
  dataStore(stream, data, context);
}


// HashMap Helper Function
template<typename P, typename Q>
inline void
storeHelper(std::ostream & stream, HashMap<P,Q> & data, void * context)
{
  // Moose::out<<"HashMap storeHelper"<<std::endl;
  dataStore(stream, data, context);
}

// Scalar Helper Function
template<typename P>
inline void
loadHelper(std::istream & stream, P & data, void * context)
{
  // Moose::out<<"Generic loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// Vector Helper Function
template<typename P>
inline void
loadHelper(std::istream & stream, std::vector<P> & data, void * context)
{
  // Moose::out<<"Vector loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// MooseSharedPointer Helper Function
template<typename P>
inline void
loadHelper(std::istream & stream, MooseSharedPointer<P> & data, void * context)
{
  // Moose::out<<"Vector loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// Set Helper Function
template<typename P>
inline void
loadHelper(std::istream & stream, std::set<P> & data, void * context)
{
  // Moose::out<<"Set loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// Map Helper Function
template<typename P, typename Q>
inline void
loadHelper(std::istream & stream, std::map<P,Q> & data, void * context)
{
  // Moose::out<<"Map loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}

// HashMap Helper Function
template<typename P, typename Q>
inline void
loadHelper(std::istream & stream, HashMap<P,Q> & data, void * context)
{
  // Moose::out<<"HashMap loadHelper"<<std::endl;
  dataLoad(stream, data, context);
}


/**
 * The following methods are specializations for using the libMesh::Parallel::packed_range_* routines
 * for std::strings. These are here because the dataLoad/dataStore routines create raw string
 * buffers that can be communicated in a standard way using packed ranges.
 */
namespace libMesh {
namespace Parallel {
template <typename T>
class Packing<std::basic_string<T> > {
public:

  static const unsigned int size_bytes = 4;

  typedef T buffer_type;

  static unsigned int get_string_len(typename std::vector<T>::const_iterator in)
  {
    unsigned int string_len = reinterpret_cast<const unsigned char &>(in[size_bytes-1]);
    for (signed int i=size_bytes-2; i >= 0; --i)
    {
      string_len *= 256;
      string_len += reinterpret_cast<const unsigned char &>(in[i]);
    }
    return string_len;
  }

  static unsigned int packed_size(typename std::vector<T>::const_iterator in)
  {
    return get_string_len(in) + size_bytes;
  }

  static unsigned int packable_size(const std::basic_string<T> & s, const void *)
  {
    return s.size() + size_bytes;
  }

  template <typename Iter>
  static void pack (const std::basic_string<T> & b, Iter data_out, const void *)
  {
    unsigned int string_len = b.size();
    for (unsigned int i=0; i != size_bytes; ++i)
    {
      *data_out++ = (string_len % 256);
      string_len /= 256;
    }

    std::copy(b.begin(), b.end(), data_out);
  }

  static std::basic_string<T> unpack(typename std::vector<T>::const_iterator in, void *)
  {
    unsigned int string_len = get_string_len(in);

    std::ostringstream oss;
    for (unsigned int i = 0; i < string_len; ++i)
      oss << reinterpret_cast<const unsigned char &>(in[i+size_bytes]);

    in += size_bytes + string_len;

    return std::string(oss.str());
  }

};

} // namespace Parallel

} // namespace libMesh

#endif /* DATAIO_H */

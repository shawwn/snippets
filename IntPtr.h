#ifndef INT_PTR_H

#define INT_PTR_H

/*!
**
** Copyright (c) 2007 by John W. Ratcliff mailto:jratcliff@infiniplex.net
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype Phone: 636-486-4040 (let it ring a long time while it goes through switches)
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliff@infiniplex.net
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#include <hash_map>


#ifndef INT64

#ifdef WIN32
typedef __int64				INT64;
#elif LINUX
typedef long long			INT64;
#else
	#error Unknown platform!
#endif

#endif


#ifndef UINT64

#ifdef WIN32
typedef unsigned __int64				UINT64;
#elif LINUX
typedef unsigned long long			UINT64;
#else
	#error Unknown platform!
#endif

#endif

// a helper class to convert an integer to a pointer
// this implementation uses the STL hash_map but *might* be switched to something faster later.
//
template <class IntType,class Type > class IntPtr
{
public:
  typedef stdext::hash_map< IntType ,Type * > IntTypeHash;

  Type * get(IntType value) const
  {
    Type *ret = 0;

    IntTypeHash::const_iterator found;
    found = mTable.find(value);
    if ( found != mTable.end() )
    {
      ret = (*found).second;
    }
    return ret;
  }

  void set(Type *t,IntType value)
  {
    mTable[value] = t;
  }

  bool erase(IntType value)
  {
    bool ret = false;

    IntTypeHash::iterator found;
    found = mTable.find(value);
    if ( found != mTable.end() )
    {
      ret = true;
      mTable.erase(found);
    }
    return ret;
  }

  void clear(void)
  {
    mTable.clear();
  }

  size_t size(void) const { return mTable.size(); };

  Type * begin(void)
  {
    Type *ret = 0;
    if ( !mTable.empty() )
    {
      mIterator = mTable.begin();
      ret = (*mIterator).second;
    }
    return ret;
  }

  Type * next(void)
  {
    Type *ret = 0;
    if ( mIterator != mTable.end() )
    {
      mIterator++;
      if ( mIterator != mTable.end() )
      {
        ret = (*mIterator).second;
      }
    }
    return ret;
  }

  IntType find(Type *p)
  {
    IntType ret = 0;
    IntTypeHash::iterator i;
    for (i=mTable.begin(); i!=mTable.end(); i++)
    {
      if ( (*i).second == p )
      {
        ret = (*i).first;
        break;
      }
    }
    return ret;
  }

  typename IntTypeHash::iterator  mIterator;
  IntTypeHash            mTable;
};

template <class IntType,class Type > class IntInt
{
public:
  typedef stdext::hash_map< IntType ,Type > IntTypeHash;

  Type  get(IntType value) const
  {
    Type ret = 0;

    IntTypeHash::const_iterator found;
    found = mTable.find(value);
    if ( found != mTable.end() )
    {
      ret = (*found).second;
    }
    return ret;
  }

  Type  get(IntType value,bool &vfound) const
  {
    Type ret = 0;

    vfound = false;
    IntTypeHash::const_iterator found;
    found = mTable.find(value);
    if ( found != mTable.end() )
    {
      ret = (*found).second;
      vfound = true;
    }
    return ret;
  }

  void set(Type t,IntType value)
  {
    mTable[value] = t;
  }

  bool erase(IntType value)
  {
    bool ret = false;

    IntTypeHash::iterator found;
    found = mTable.find(value);
    if ( found != mTable.end() )
    {
      ret = true;
      mTable.erase(found);
    }
    return ret;
  }

  void clear(void)
  {
    mTable.clear();
  }

  size_t size(void) const { return mTable.size(); };

  Type begin(void)
  {
    Type ret = 0;
    if ( !mTable.empty() )
    {
      mIterator = mTable.begin();
      ret = (*mIterator).second;
    }
    return ret;
  }

  Type next(bool &end)
  {
    end = true;
    Type ret = 0;
    if ( mIterator != mTable.end() )
    {
      mIterator++;
      if ( mIterator != mTable.end() )
      {
        ret = (*mIterator).second;
        end = false;
      }
    }
    return ret;
  }

  typename IntTypeHash::iterator  mIterator;
  IntTypeHash            mTable;
};


#endif

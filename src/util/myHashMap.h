/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class HashKey
// {
// public:
//    HashKey() {}
// 
//    size_t operator() () const { return 0; }
// 
//    bool operator == (const HashKey& k) const { return true; }
// 
// private:
// };
//
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

      public:
         iterator();
         iterator(unsigned b, unsigned l, vector<HashNode>* h,unsigned n): _buc(b), _loc(l), _hash(h), _num(n){}
         ~iterator() {} // Should NOT delete _node

         // TODO: implement these overloaded operators
         const HashNode& operator * () const { return *(this); }
         HashNode& operator * ()  { return _hash[_buc][_loc]; } // write

         iterator& operator ++ () { 
            iterator tmp = getNode(true);
            _buc = tmp._buc;
            _loc = tmp._loc;
            return *this;
          }

          iterator operator ++ (int) { 
            iterator tmp(*(this));
            iterator tmpi = getNode(true);
            _buc = tmpi._buc;
            _loc = tmpi._loc;
            return tmp; 
          }

          iterator& operator -- () { 
            iterator tmp = getNode(false);
            _buc = tmp._buc;
            _loc = tmp._loc;
            return *this;
          }

          iterator operator -- (int)   { 
            iterator tmp(*(this)); 
            iterator tmpi = getNode(false);
            _buc = tmpi._buc;
            _loc = tmpi._loc;
            return tmp;
          }

         
         iterator& operator = (const iterator& i) 
         { 
            _loc = i._loc;
            _buc = i._buc;
            _hash = i._hash;
            _num = i._num;
            return *(this); 
         } 

         bool operator != (const iterator& i) const { return ( _buc != i._buc || _loc != i._loc); }
         bool operator == (const iterator& i) const { return ( _buc == i._buc && _loc == i._loc); }

         iterator getNode(bool b) //true->next false->prev
         {
            if(b){
               if(_loc != _hash[_buc].size()-1 ) { iterator tmp(_buc, _loc+1, _hash, _num); return tmp ; }
               else {
                  for (unsigned i = _buc+1; i < _num; ++i)
                  {
                     if(_hash[i].size() != 0) { iterator tmp(i, 0, _hash, _num); return tmp ; }
                  } 
               }
               iterator tmp(_buc, _loc+1, _hash, _num); return tmp ;
            }

            else{
               if(_loc != 0) { iterator tmp(_buc, _loc-1, _hash, _num); return tmp ; }
               else{
                  for (unsigned i = _buc-1; i > 0; --i)
                  {
                     if(_hash[i].size() != 0) { iterator tmp(i, _hash[i].size()-1, _hash, _num); return tmp ; }
                  }
               }
            }
         }
      private:
         unsigned _buc ;
         unsigned _loc ;
         vector<HashNode>*  _hash;
         unsigned _num;
   };


   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const
   { 
      if(empty())
      {
         iterator firhash(0, 0, _buckets, _numBuckets);
         return firhash;
      }
      for (unsigned i = 0; i < _numBuckets; ++i)
      {
         if(_buckets[i].size() != 0) 
         { 
            iterator firhash(i, 0, _buckets, _numBuckets);
            return firhash; 
         }
      }
   }
   
   // Pass the end
   iterator end() const
   {
      if(empty())
      {
         iterator lasthash(0, 0, _buckets, _numBuckets);
         return lasthash;
      }
      for (unsigned i = _numBuckets-1 ; i >= 0; --i)
      {
         if(_buckets[i].size() != 0) 
         {  
            iterator lasthash(i, _buckets[i].size(), _buckets, _numBuckets);
            return lasthash; 
         }
      } 
   }

   // return true if no valid data
   bool empty() const 
   { 
      for (unsigned i = 0; i < _numBuckets; ++i)
      {
         if(_buckets[i].size() != 0) { return false;  }
      }
      return true;
   }

   // number of valid data
   size_t size() const 
   { 
      size_t s = 0; 
      for (unsigned i = 0; i < _numBuckets; ++i)
      {
         if(_buckets[i].size() != 0) { s += (_buckets[i].size());  }
      }
      return s; 
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const 
   { 
      int num = bucketNum(k);
      for (unsigned i = 0; i < _buckets[num].size(); ++i)
      {
         if(_buckets[num][i].first == k ) { return true;  }
      } 
      return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const 
   { 
      int num = bucketNum(k);
      for (unsigned i = 0; i < _buckets[num].size(); ++i)
      {
         if(_buckets[num][i].first == k ) 
         {
            d = _buckets[num][i].second;
            return true;  
         }
      } 
      return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) 
   { 
      int num = bucketNum(k);
      for (unsigned i = 0; i < _buckets[num].size(); ++i)
      {
         if(_buckets[num][i].first == k ) { _buckets[num][i].second = d; return true; }
      } 
      return false; 
   }

   // return true if inserted d successfully (i.e. k is not in the hash) 
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) 
   { 
      if(check(k)) return false;
      else {
         HashNode tmp(k,d);
         _buckets[bucketNum(k)].push_back(tmp);
      } 
      return true;
   }

   void quickinsert(const HashKey& k, const HashData& d) 
   { 
      HashNode tmp(k,d);
      _buckets[bucketNum(k)].push_back(tmp);
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) 
   { 
      int num = bucketNum(k);
      typename std::vector<HashNode>::iterator i = _buckets[num].begin();
      for (; i != _buckets[num].end(); ++i)
      {
         if((*i).first == k) { _buckets[num].erase(i); return true; }
      }
      return false;
   }

   void operator = (const HashMap<HashKey, HashData>& i) 
   {
      if(i.empty()) { init(i._numBuckets); return;}
      init(i._numBuckets);
      HashMap<HashKey, HashData>::iterator it = i.begin();
      for (; it != i.end(); ++it)
      {
         insert((*it).first, (*it).second);
      } 
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H

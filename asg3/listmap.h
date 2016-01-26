// Author: Coy Humphrey (cmhumphr)

#ifndef __LISTMAP_H__
#define __LISTMAP_H__

#include "xless.h"
#include "xpair.h"

template <typename Key, typename Value, class Less=xless<Key>>
class listmap {
   public:
      typedef Key key_type;
      typedef Value mapped_type;
      typedef xpair<key_type,mapped_type> value_type;
   private:
      Less less;
      struct node {
         value_type pair;
         node* prev;
         node* next;
         node (const value_type&);
      };
      node* head;
      node* tail;
   public:
      class iterator;
      listmap();
      listmap (const listmap&);
      listmap& operator= (const listmap&);
      ~listmap();
      void insert (const value_type&);
      iterator find (const key_type&);// const;
      iterator begin();
      iterator end();
      bool empty() const;
};


template <typename Key, typename Value, class Less=xless<Key>>
class listmap<Key,Value,Less>::iterator {
      friend class listmap<Key,Value>;
   private:
      iterator (listmap* map, node* where);
      listmap<Key,Value,Less>* map;
      node* where;
   public:
      iterator(): map(NULL), where(NULL) {}
      value_type& operator*();
      value_type* operator->();
      iterator& operator++(); //++itor
      iterator& operator--(); //--itor
      void erase();
      bool operator== (const iterator&) const;
      inline bool operator!= (const iterator& that) const {
         return not (*this == that);
      }
};

#include "listmap.tcc"
#endif


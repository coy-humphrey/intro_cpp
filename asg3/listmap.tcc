// Author: Coy Humphrey (cmhumphr)

#include "listmap.h"
#include "trace.h"

template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (const value_type& pair):
            pair(pair), prev(nullptr), next(nullptr) 
{
}

template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::listmap (): head(nullptr), tail (nullptr) 
{
   
}

template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap () {
   TRACE ('l', (void*) this);
   while (!empty())
   {
      auto itor = begin();
      itor.erase();
   }
}

template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::insert (const xpair<Key,Value>& pair) {
   TRACE ('l', &pair << "->" << pair);
   node *itor = head;
   while (itor != nullptr)
   {
      // While the new item is greater than the current item
      if (less (itor->pair.first, pair.first))
      {
         itor = itor->next;
         continue;
      }
      else if (!less (pair.first, itor->pair.first))
      {
         // Keys are equal
         itor->pair.second = pair.second;
         return;
      }
      // Not equal and in correct order
      else
      {
         node *new_node = new node (pair);
         new_node->next = itor;
         new_node->prev = itor->prev;
         if (new_node->prev != nullptr)
         {
            new_node->prev->next = new_node;
         }
         else
         {
            head = new_node;
         }
         if (new_node->next != nullptr)
         {
            new_node->next->prev = new_node;
         }
         else
         {
            tail = new_node;
         }
         return;
      }
   }
   if (empty())
   {
      head = tail = new node (pair);
   }
   else
   {
      tail->next = new node (pair);
      tail->next->prev = tail;
      tail = tail->next;
   }
}

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that)// const {
{
   TRACE ('l', that);
   node *itor = head;
   for (; itor != nullptr; itor = itor->next)
   {
      if (!less (that, (itor->pair).first) &&
         !less ((itor->pair).first, that))
      {
         return iterator (this, itor);
      }
   }
   return iterator();
}

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::begin () 
{
   return iterator (this, head);
}

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::end () 
{
   return iterator (this, nullptr);
}

template <typename Key, typename Value, class Less>
bool listmap<Key,Value,Less>::empty () const 
{
   return head == nullptr;
}


template <typename Key, typename Value, class Less>
xpair<Key,Value>& listmap<Key,Value,Less>::iterator::operator* () 
{
   TRACE ('l', where->pair);
   return where->pair;
}

template <typename Key, typename Value, class Less>
xpair<Key,Value> *listmap<Key,Value,Less>::iterator::operator-> () 
{
   TRACE ('l', where->pair);
   return &(where->pair);
}

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++ () 
{
   TRACE ('l', "First: " << map << ", " << where);
   TRACE ('l', "Second: " << map->head << ", " << map->tail);
   if (where == nullptr) return *this;
   where = where->next;
   return *this;
}

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator-- () 
{
   if (where == nullptr) return *this;
   where = where->prev;
   return *this;
}

template <typename Key, typename Value, class Less>
bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const 
{
   return this->where == that.where;
}

template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::iterator::iterator (listmap *map,
            node *where): map (map), where (where)
{
   
}

template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::iterator::erase () 
{
   TRACE ('l', "map = " << map << ", where = " << where << endl);
   if (where == nullptr) return;
   if (where->prev != nullptr)
   {
      where->prev->next = where->next;
   }
   else
   {
      map->head = where->next;
   }
   if (where->next != nullptr)
   {
      where->next->prev = where->prev;
   }
   else
   {
      map->tail = where->prev;
   }
   node *tmp = where->next;
   delete where;
   where = tmp;
}


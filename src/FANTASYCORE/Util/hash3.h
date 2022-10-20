#ifndef __JOO_HASH_V3_3__
#define __JOO_HASH_V3_3__

/*
 *          A Chained Hash Table Implementation
 *  incorporating iterators, sentinels, and fast rehashing
 *
 *    Created in 1998/11/1
 *            by Young-Hyun Joo
 *
 *    Last Modified in 2001/9/18
 *                  by Young-Hyun Joo
 */

#include <string.h>

/*
 *  Declaration of pre-defined hash functions
 *  
 *  Note : To define your own hash function, conform to the following interface form.
 *
 *     struct custom_hash_fn 
 *     {
 *         typedef 'your key type' Key;
 *
 *         ; return a number between [0..capacity-1]
 *         static unsigned int get_hash_index( Key key, unsigned int capacity );
 *
 *         ; return whether key1 and key2 is equal or not.
 *         static bool is_equal( Key key1, Key key2 );
 *
 *         ; return whether key1 is smaller than key2 or not.
 *         static bool is_less( Key key1, Key key2 );
 *     };
 */

template< class IntNum >		// integer key, casual use
struct mod_hash_fn;

struct Jenkins32_hash_fn;		// 32-bit integer key, more serious use

struct string_mod_hash_fn;		// string key

struct pstring_mod_hash_fn;		// string key, but no internal contents copying is occurred.

template< class T >
struct pointer_mod_hash_fn;		// pointer of T key

/*
 *    hash3 - hash version 3
 */

template < class T, class HashFn = mod_hash_fn< int > >
class hash3
{
	struct Slot;
	struct Node;
public:

	typedef typename HashFn::Key HashKey;

	hash3( unsigned int n = 271 ): capacity(n) 
	{
		// make sentinels

		nilSlot = &nilSlotData;
		nilNode = reinterpret_cast<Node*>( &nilNodeData );
		nilSlot->prev = nilSlot->next = nilSlot;
		nilSlot->ptr = nilNode;
		nilNode->prev = nilNode->next = nilNode;

		// initialize hash table with sentinels
		hashTable = new Slot*[ n ];
		for ( unsigned int i = 0; i < capacity; i++ )
			hashTable[ i ] = nilSlot;

		initEnumState();
	}

	~hash3()
	{
		// finalize hash table
		for ( unsigned int i = 0 ; i < capacity ; i++ )
			clearSlots( hashTable[ i ] );
		delete[] hashTable;
	}

	/*
	 * Resets the hash table to be empty.
	 * All iterators of the hash table become invalid.
	 */
	void clear()
	{
		for ( unsigned int i = 0 ; i < capacity ; i++ ) {
			clearSlots( hashTable[i] );
			hashTable[i] = nilSlot;
		}
		initEnumState();
	}

	//
	// iterator class
	//
	struct iterator
	{
		iterator() {}
		iterator( Node* aNode ): ptr(aNode) {}
		iterator( Node* aNode, Slot* aSlot ): ptr(aNode), slot(aSlot) {}
		iterator( const iterator& it ): ptr(it.ptr), slot(it.slot) {}

		T&				   operator *()				  { return ptr->value; }
		const T&		   operator *() const		  { return ptr->value; }
		const iterator&	   operator ++() const		  { ptr = ptr->next; return *this; }
		const iterator&	   operator --() const		  { ptr = ptr->prev; return *this; }
		iterator		   operator ++(int) const	  { iterator _tmp = *this; ++*this; return _tmp; }
		iterator		   operator --(int) const	  { iterator _tmp = *this; --*this; return _tmp; }
		bool			   operator ==( iterator it ) { return ptr == it.ptr; }
		bool			   operator !=( iterator it ) { return ptr != it.ptr; }
		const HashKey&	   key() const				  { return slot->key; }

		mutable Node* ptr;
		mutable Slot* slot;
	};

	/*
	 * Returns the sentinel iterator.
	 */
	iterator nil() const { return iterator( nilNode ); }

	/*
	 * Searches the given key in the hash table and returns
	 * iterator indicating the 1st item with the given key.
	 * Returns nil() when no items are found.
	 */
	iterator search( HashKey key ) const
	{
		Slot* slot = hashTable[ HashFn::get_hash_index( key, capacity ) ];
		for (; slot != nilSlot; slot = slot->next ) {
			if ( HashFn::is_less( slot->key, key ) )
				continue;
			if ( HashFn::is_equal( key, slot->key ) )
				return iterator( slot->ptr, slot );
			break;
		}
		return nil();
	}

	/*
	 * Inserts the given (key,value) pair into the hash table
	 * and returns an iterator pointing to the newly added item.
	 *
	 * Note: New item is inserted in FRONT of all of the existing
	 *       items with the given key.
	 */
	iterator insert( HashKey key, const T& value )
	{
		Slot*& entry = hashTable[ HashFn::get_hash_index( key, capacity ) ];
		Slot* slot = entry;

		for (; slot != nilSlot; slot = slot->next ) {
			nilSlot->prev = slot;
			if ( HashFn::is_less( slot->key, key ) ) continue;

			if ( HashFn::is_equal( key, slot->key ) ) {
				Node* n = new Node( value, nilNode, slot->ptr );
				slot->ptr->prev = n;
				slot->ptr = n;
				nilSlot->prev = nilSlot;
				return iterator( n, slot );
			}
			break;
		}

		Node* n = new Node( value, nilNode, nilNode );
		Slot* s = new Slot( key, slot->prev, slot, n );

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;
		
		slot->prev = s;
		
		nilSlot->prev = nilSlot;
		return iterator( n, s );
	}

	/*
	 * Erases the node pointed by the given iterator from the hash.
	 *
	 * Note : Given iterator points to the next item after successful
	 *        deletion. If it points to the last item, it becomes nil().
	 */
	void erase( iterator& it )
	{
		if ( it == nil() ) return;
		if ( it.ptr == es_node ) 
			findNextValue();

		iterator it2 = it++;

		if ( it2.ptr->prev != nilNode ) {
			// trivial node deletion
			it2.ptr->prev->next = it2.ptr->next;
			it2.ptr->next->prev = it2.ptr->prev;
			delete it2.ptr;
			return;
		} 

		// deleting first node; slot entry must be updated
		it2.ptr->next->prev = nilNode;
		it2.slot->ptr = it2.ptr->next;
		delete it2.ptr;
		
		// quit if slot deletion is not needed
		if ( it2.slot->ptr != nilNode ) return;

		if ( it2.slot->prev != nilSlot ) {
			// trivial slot deletion
			it2.slot->prev->next = it2.slot->next;
			it2.slot->next->prev = it2.slot->prev;
			delete it2.slot;
			nilSlot->prev = nilSlot;
			return;
		}

		// deleting first slot; hash table entry must be updated
		it2.slot->next->prev = nilSlot;
		hashTable[ HashFn::get_hash_index( it2.slot->key, capacity ) ] = it2.slot->next;
		delete it2.slot;
		nilSlot->prev = nilSlot;
	}

	/*
	 * Inserts given (key,value) pair into the hash table while
	 * maintaining ascending order between items with same keys
	 * according to the given value compare function 'less',
	 * and returns iterator pointing to the inserted item.
	 *
	 * Note: You can put STL function object instead of function
	 *       pointer into argument 'less', which enables you to
	 *       pass inlined code block.
	 */
	template< class TLessFn >
	iterator insert( HashKey key, const T& value, TLessFn less )
	{
		Slot*& entry = hashTable[ HashFn::get_hash_index( key, capacity ) ];
		Slot* slot = entry;

		for (; slot != nilSlot; slot = slot->next ) {
			nilSlot->prev = slot;
			if ( HashFn::is_less( slot->key, key ) ) continue;

			if ( HashFn::is_equal( key, slot->key ) ) {

				Node* p = slot->ptr;
				for (; p != nilNode; p = p->next ) {
					nilNode->prev = p;
					if ( less( value, p->value ) ) break;
				}

				Node* n = new Node( value, p->prev, p );

				if ( p == slot->ptr ) slot->ptr = n;
				else				  p->prev->next = n;

				p->prev = n;

				nilNode->prev = nilNode;
				nilSlot->prev = nilSlot;
				return iterator(n,slot);
			}
			nilSlot->prev = nilSlot;
			break;
		}

		Node* n = new Node( value, nilNode, nilNode );
		Slot* s = new Slot( key, slot->prev, slot, n );

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;
		
		slot->prev = s;
		
		nilSlot->prev = nilSlot;
		return iterator(n,s);
	}

	/*
	 * Removes all items with the given key.
	 */
	void erase( HashKey key )
	{
		Slot*& entry = hashTable[ HashFn::get_hash_index( key, capacity ) ];
		Slot* slot = entry;

		while (true) {
			if ( slot == nilSlot ) return;
			if ( HashFn::is_less( key, slot->key ) ) return;
			if ( HashFn::is_equal( key, slot->key ) ) break;
			slot = slot->next;
		}

		while ( slot == es_slot )
			findNextValue();

		if ( slot == entry ) entry = slot->next;
		else				 slot->prev->next = slot->next;

		slot->next->prev = slot->prev;
		clearNodes(slot);
		delete slot;

		nilSlot->prev = nilSlot;
	}

	/*
	 * Changes the size of the hash table to the given number while
	 * retaining all the items and their relations. This operation
	 * is quite efficient. 
	 *
	 * Note: All of valid iterators will STILL REMAIN VALID after 
	 *       rehashing.
	 */
	void rehash( unsigned int n = 271 )
	{
		unsigned int i;

		if (n == capacity) return;

		Slot** oldTable = hashTable;
		hashTable = new Slot*[n];

		for ( i = 0; i < n; i++ ) 
			hashTable[i] = nilSlot;

		unsigned int oldCapacity = capacity;
		capacity = n;

		for ( i = 0; i < oldCapacity; i++ ) {
			for ( Slot* slot = oldTable[i]; slot != nilSlot; ) {
				Slot* n = slot->next;
				insertSlot( slot );
				slot = n;
			}
		}

		delete[] oldTable;
		
		initEnumState();
	};

	/*
	 * Checks given iterator whether it points to a valid position of
	 * the table. Useful for debugging.
	 */
	bool is_valid( iterator it ) const
	{
		if ( it == nil() ) return true;
		if ( it.slot == NULL ) return false;
		if ( it.ptr == NULL ) return false;

		for ( unsigned int i = 0; i < capacity; i++ ) {
			for ( Slot* slot = hashTable[i]; slot != nilSlot; slot = slot->next ) {
				if ( it.slot == slot ) {
					for ( Node* node = slot->ptr; node != nilNode; node = node->next ) {
						if ( it.ptr == node )
							return true;
					}
					return false;
				}
			}
		}
		return false;
	}

	/*
	 * Searches the hash table for incorrect links.
	 * Returns true when none is found. Useful for debugging.
	 */
	bool check_self_integrity()
	{
		for ( unsigned int i = 0; i < capacity; i++ ) {

			Slot* slot = hashTable[i];
			if ( slot->prev != nilSlot )
				return false;

			for (; slot != nilSlot; slot = slot->next ) {
				if ( slot->next != nilSlot )
					if ( slot->next->prev != slot ) 
						return false;

				Node* node = slot->ptr;
				if ( node->prev != nilNode ) 
					return false;

				for (; node != nilNode; node = node->next ) {
					if ( node->next != nilNode ) 
						if ( node->next->prev != node )
							return false;
				}
			}
		}
		return true;
	}

	/*
	 * Value enumerating methods. While not intrinsic hash operation, these can be 
	 * very handy at times. 
	 * get_first_value() and get_next_value() returns stored values by no particular
	 * order, and returns nil() when no more value is found.
	 *
	 * Note : Internal 'enum_state' resets after rehash() and clear().
	 */

	iterator get_first_value()
	{
		findFirstValue();
		iterator it( es_node, es_slot );
		findNextValue();
		return it;
	}

	iterator get_next_value()
	{
		iterator it( es_node, es_slot );
		findNextValue();
		return it;
	}
	
	/*
	 * Memory occupation calculation method. 
	 */

	unsigned long calc_memory() const 
	{
		unsigned long size = sizeof( this ) + m_capacity * sizeof( Slot* );

		for ( unsigned int i = 0; i < m_capacity; i++ ) {
			for ( Slot* slot = m_hashTable[ i ]; slot != m_nilSlot; slot = slot->next ) {
				size += sizeof( Slot );
				for ( Node* node = slot->ptr; node != m_nilNode; node = node->next ) {
					size += sizeof( Node );
				}
			}
		}

		return size;
	}

private:

	struct Slot {
		Slot* prev;
		Slot* next;
		Node* ptr;
		HashKey key;

		Slot() {}
		Slot( HashKey _key, Slot* _prev, Slot* _next, Node* _ptr )
			: key(_key), prev(_prev), next(_next), ptr(_ptr) {}
	};

	struct Node {
		Node* prev;
		Node* next;
		T value;

		Node() {}
		Node( const T& _value, Node* _prev, Node* _next ) 
			: value(_value), prev(_prev), next(_next) {}
	};

	void clearNodes( Slot* slot )
	{
		Node* p = slot->ptr; 
		while (p != nilNode) {
			Node* n = p->next;
			delete p;
			p = n;
		}
		slot->ptr = nilNode;
	}

	void clearSlots( Slot* slot )
	{
		while (slot != nilSlot) {
			Slot* n = slot->next;
			clearNodes( slot );
			delete slot;
			slot = n;
		}
	}

	void insertSlot( Slot* s )
	{
		Slot*& entry = hashTable[ HashFn::get_hash_index( s->key, capacity ) ];
		Slot* slot = entry;

		for (; slot != nilSlot; slot = slot->next ) {
			nilSlot->prev = slot;
			if ( HashFn::is_less( s->key, slot->key ) ) break;
		}

		s->prev = slot->prev;
		s->next = slot;

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;

		slot->prev = s;
		nilSlot->prev = nilSlot;
	}

	void initEnumState()
	{
		es_slot_index = 0;
		es_slot = nilSlot;
		es_node = nilNode;
	}

	void findFirstValue()
	{
		for ( es_slot_index = 0; es_slot_index < capacity; es_slot_index++ ) {
			for ( es_slot = hashTable[ es_slot_index ]; es_slot != nilSlot; es_slot = es_slot->next ) {
				for ( es_node = es_slot->ptr; es_node != nilNode; es_node = es_node->next ) {
					return;
				}
			}
		}
		es_node = nilNode;
		es_slot = nilSlot;
	}

	void findNextValue()
	{
		if ( es_node == nilNode ) 
			return;

		es_node = es_node->next;
		while ( true ) {
			while ( true ) {
				if ( es_node != nilNode )
					return;
				
				es_slot = es_slot->next; 
				if ( es_slot == nilSlot )
					break;
				es_node = es_slot->ptr;
			}

			if ( ++es_slot_index == capacity ) 
				break;
			es_slot = hashTable[ es_slot_index ];
			es_node = es_slot->ptr;
		}
		es_node = nilNode;
		es_slot = nilSlot;
	}

	unsigned int capacity;
	Slot** hashTable;

	struct NilNode {
		Node* prev;
		Node* next;
	};

	// sentinels

	Slot    nilSlotData;
	NilNode nilNodeData;

	Slot* nilSlot;
	Node* nilNode;

	// enumeration state

	unsigned int es_slot_index;
	Slot* es_slot;
	Node* es_node;
};

/*
 *  Implementation of pre-defined hash functions
 */

template < class IntNum >
struct mod_hash_fn
{
    typedef IntNum Key;

    static inline unsigned int get_hash_index( IntNum key, unsigned int capacity ) {
		key += (key << 12);
		key ^= (key >> 22);
        return (key > 0) ? key % capacity : (-key) % capacity;
    }

    static inline bool is_equal( IntNum key1, IntNum key2 ) {
        return key1 == key2;
    }

    static inline bool is_less( IntNum key1, IntNum key2 ) {
        return key1 < key2;
    }
};

struct Jenkins32_hash_fn
{
    typedef long Key;

    static inline unsigned int get_hash_index( long key, unsigned int capacity ) {
		key += (key << 12);
		key ^= (key >> 22);
		key += (key << 4);
		key ^= (key >> 9);
		key += (key << 10);
		key ^= (key >> 2);
		key += (key << 7);
		key ^= (key >> 12);
        return (key & 0x7fffffff) % capacity;
    }

    static inline bool is_equal( long key1, long key2 ) {
        return key1 == key2;
    }

    static inline bool is_less( long key1, long key2 ) {
        return key1 < key2;
    }
};

struct string_mod_hash_fn
{
    struct Key 
	{
        Key()							  { m_pStr = NULL; }
        Key( const Key& str )			  { const size_t pStr_len = strlen(str.m_pStr)+1; //aleksger - safe string 
											strcpy_s( m_pStr = new char[pStr_len], pStr_len, str.m_pStr ); }
        Key( const char* pStr )			  { const size_t pStr_len = strlen(pStr)+1; //aleksger - safe string 
											strcpy_s( m_pStr = new char[pStr_len], pStr_len, pStr ); }
        ~Key()							  { delete[] m_pStr; }
        operator char*()				  { return m_pStr; }
        operator const char*() const	  { return m_pStr; }
        void operator =( const Key& str ) { delete[] m_pStr;
											const size_t pStr_len = strlen(str.m_pStr)+1; //aleksger - safe string 
										    strcpy_s( m_pStr = new char[pStr_len], pStr_len, str.m_pStr ); }
        char* m_pStr;
    };

    static inline unsigned int get_hash_index( const Key& string, unsigned int capacity )
    {
        unsigned int key;
        char* pStr = string.m_pStr;
    	for ( key = 0; *pStr; pStr++ )
	    	key = key * 32 - key + *pStr;

	    return key % capacity;
    }

    static inline bool is_equal( const Key& key1, const Key& key2 )
    {
        char *pKey1 = key1.m_pStr, *pKey2 = key2.m_pStr;
        for (; *pKey1 == *pKey2; pKey1++, pKey2++ )
            if ( *pKey1 == 0 )
                return true;
        return false;
    }

    static inline bool is_less( const Key& key1, const Key& key2 )
    {
        char *pKey1 = key1.m_pStr, *pKey2 = key2.m_pStr;
        for (; *pKey1 == *pKey2; pKey1++, pKey2++ )
            if ( *pKey1 == 0 )
                return false;
        return *pKey1 < *pKey2;
    }
};

struct pstring_mod_hash_fn
{
	typedef const char* Key;

    static inline unsigned int get_hash_index( const char* pKey, unsigned int capacity )
    {
        unsigned int key;
    	for ( key = 0; *pKey; pKey++ )
	    	key = key * 32 - key + *pKey;

	    return key % capacity;
    }

    static inline bool is_equal( const char* pKey1, const char* pKey2 )
    {
        for (; *pKey1 == *pKey2; pKey1++, pKey2++ )
            if ( *pKey1 == 0 )
                return true;
        return false;
    }

    static inline bool is_less( const char* pKey1, const char* pKey2 )
    {
        for (; *pKey1 == *pKey2; pKey1++, pKey2++ )
            if ( *pKey1 == 0 )
                return false;
        return *pKey1 < *pKey2;
    }
};

template< class T >
struct pointer_mod_hash_fn
{
    typedef T* Key;

    static inline unsigned int get_hash_index( T* key, unsigned int capacity ) {
		unsigned index = unsigned(key);
		index += (index << 12);
		index ^= (index >> 22);
        return unsigned(index) % capacity;
    }

    static inline bool is_equal( T* key1, T* key2 ) {
        return key1 == key2;
    }

    static inline bool is_less( T* key1, T* key2 ) {
        return key1 < key2;
    }
};

#endif

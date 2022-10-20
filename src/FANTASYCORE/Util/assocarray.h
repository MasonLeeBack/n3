#ifndef __JOO_ASSOCIATIVE_ARRAY__
#define __JOO_ASSOCIATIVE_ARRAY__

/*
 *      An Associative Array Implementation
 *            based on chained hash 
 *
 *    Created on 2001/10/9
 *            by Young-Hyun Joo
 *
 *    Last Modified on 2003/3/11
 *                  by Young-Hyun Joo
 */

#include "hash3.h"

/*
 *    assoc_array
 *
 *  Note : Hash function definition form of assoc_array conforms to that of hash3.
 */

template < class T, class HashFn = mod_hash_fn< int >, bool bAutoRehash = true >
class assoc_array
{
	struct Slot;
public:

	typedef typename HashFn::Key HashKey;

	assoc_array( unsigned int n = 271 ): m_capacity(n) 
	{
		// make sentinels

		m_nilSlot = reinterpret_cast<Slot*>( &m_nilSlotData );
		m_nilSlot->prev = m_nilSlot->next = m_nilSlot;

		// initialize hash table with sentinels
		m_hashTable = new Slot*[ n ];
		for ( unsigned int i = 0; i < m_capacity; i++ )
			m_hashTable[ i ] = m_nilSlot;

		m_count = 0;

		initEnumState();
	}

	~assoc_array()
	{
		// finalize hash table
		for ( unsigned int i = 0 ; i < m_capacity ; i++ )
			clearSlots( m_hashTable[ i ] );
		delete[] m_hashTable;
	}

	/*
	 * Resets the hash table to be empty.
	 * All iterators of the hash table become invalid.
	 */
	void clear()
	{
		for ( unsigned int i = 0 ; i < m_capacity ; i++ ) {
			clearSlots( m_hashTable[ i ] );
			m_hashTable[ i ] = m_nilSlot;
		}

		m_count = 0;

		initEnumState();
	}

	/*
	 * Searches the given key in the hash table and returns
	 * pointer to corresponding item.
	 * Returns NULL when no item is found.
	 */
	const T* search( HashKey key ) const
	{
		Slot* slot = sub_search( key );
		return slot != NULL ? &slot->value : NULL;
	}

	T* search( HashKey key )
	{
		Slot* slot = sub_search( key );
		return slot != NULL ? &slot->value : NULL;
	}

	/*
	 * Searches the given key, and returns pointer to containing key and item. 
	 * Returns whether matching item is found or not.
	 */
	bool search( HashKey key, const HashKey*& pOutKey, const T*& pValue ) const
	{
		Slot* slot = sub_search( key );
		if ( slot != NULL ) {
			pOutKey = &slot->key, pValue = &slot->value;
			return true;
		}
		return false;
	}

	bool search( HashKey key, const HashKey*& pOutKey, T*& pValue )
	{
		Slot* slot = sub_search( key );
		if ( slot != NULL ) {
			pOutKey = &slot->key, pValue = &slot->value;
			return true;
		}
		return false;
	}

	/* 
	 * Search, operator version. Returns reference instead of pointer, and throws
	 * an exception(0) when no item is found.
	 */
	const T& operator [] ( HashKey key ) const
	{
		Slot* slot = sub_search( key );
		if ( slot == NULL )
			throw 0;
		return slot->value;
	}

	/*
	 * Inserts the given (key,value) pair into the hash table
	 * or replaces stored value by given value when given key exists.
	 */
	T* insert( HashKey key, const T& value )
	{
		return &sub_insert( key, value )->value;
	}

	/*
	 * Insert method, returning (key, value) pair version.
	 */
	void insert( HashKey key, const T& value, const HashKey*& pOutKey, T*& pOutValue )
	{
		Slot* slot = sub_insert( key, value );
		pOutKey = &slot->key, pValue = &slot->value;
	}

	/*
	 * A hybrid of search() and insert().
	 * Searches the given key and returns matched value reference.
	 * If no matching value exists, a new one is created with the given key.
	 */
	T& operator [] ( HashKey key )
	{
		Slot*& entry = m_hashTable[ HashFn::get_hash_index( key, m_capacity ) ];
		Slot* slot = entry;

		for (; slot != m_nilSlot; slot = slot->next ) {
			m_nilSlot->prev = slot;
			if ( HashFn::is_less( slot->key, key ) ) continue;

			if ( HashFn::is_equal( key, slot->key ) ) {
				m_nilSlot->prev = m_nilSlot;
				return slot->value;
			}
			break;
		}

		Slot* s = new Slot( key, slot->prev, slot );

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;
		
		slot->prev = s;
		m_nilSlot->prev = m_nilSlot;

		incCount();
		return s->value;
	}

	/*
	 * Removes item with the given key.
	 */
	void erase( HashKey key )
	{
		Slot*& entry = m_hashTable[ HashFn::get_hash_index( key, m_capacity ) ];
		Slot* slot = entry;

		while (true) {
			if ( slot == m_nilSlot ) return;
			if ( HashFn::is_less( key, slot->key ) ) return;
			if ( HashFn::is_equal( key, slot->key ) ) break;
			slot = slot->next;
		}

		while ( slot == m_esSlot )
			findNextKey();

		if ( slot == entry ) entry = slot->next;
		else				 slot->prev->next = slot->next;

		slot->next->prev = slot->prev;
		delete slot;

		m_nilSlot->prev = m_nilSlot;
		decCount();
	}

	/*
	 * Key counting method.
	 */
	unsigned int count()
	{
		return m_count;
	}

	/*
	 * Key & value enumerating methods. While not intrinsic hash operation, 
	 * these can be very handy at times. 
	 * get_first_key() and get_next_key() returns stored keys by no particular
	 * order, and returns NULL when no more value is found. 
	 * Likely, get_first_value() and get_next_value() returns stored values
	 * instead of keys.
	 *
	 * Note : Internal 'enum_state' resets after rehash() and clear().
	 */

	const HashKey* get_first_key() const
	{
		findFirstKey();
		HashKey* pKey = (m_esSlot == m_nilSlot) ? NULL : &m_esSlot->key;
		findNextKey();
		return pKey;
	}

	const HashKey* get_next_key() const
	{
		HashKey* pKey = (m_esSlot == m_nilSlot) ? NULL : &m_esSlot->key;
		findNextKey();
		return pKey;
	}

	const T* get_first_value() const
	{
		findFirstKey();
		T* pValue = (m_esSlot == m_nilSlot) ? NULL : &m_esSlot->value;
		findNextKey();
		return pValue;
	}

	const T* get_next_value() const
	{
		T* pValue = (m_esSlot == m_nilSlot) ? NULL : &m_esSlot->value;
		findNextKey();
		return pValue;
	}

	/*
	 * Searches the hash table for incorrect links.
	 * Returns true when none is found. Useful for debugging.
	 */
	bool check_self_integrity() const
	{
		unsigned int count = 0;
		for ( unsigned int i = 0; i < m_capacity; i++ ) {

			Slot* slot = m_hashTable[i];
			if ( slot->prev != m_nilSlot )
				return false;

			for (; slot != m_nilSlot; slot = slot->next, ++count ) {
				if ( slot->next != m_nilSlot )
					if ( slot->next->prev != slot ) 
						return false;
			}
		}
		return count == m_count;
	}

	/*
	 * Changes the size of the hash table to the given number while
	 * retaining all the items and their relations. This operation
	 * is quite efficient. 
	 * 
	 * Note : This is automatically called as needed. 
	 */
	void rehash( unsigned int n = 271 )
	{
		unsigned int i;

		if (n == m_capacity) return;

		Slot** oldTable = m_hashTable;
		m_hashTable = new Slot*[n];

		for ( i = 0; i < n; i++ ) 
			m_hashTable[i] = m_nilSlot;

		unsigned int old_capacity = m_capacity;
		m_capacity = n;

		for ( i = 0; i < old_capacity; i++ ) {
			for ( Slot* slot = oldTable[i]; slot != m_nilSlot; ) {
				Slot* tmp = slot->next;  //aleksger: prefix bug 830: Local declaration may confuse.
				insertSlot( slot );
				slot = tmp;
			}
		}

		delete[] oldTable;
		
		initEnumState();
	};

	/*
	 * Memory occupation calculation method. 
	 */

	unsigned long calc_memory() const 
	{
		unsigned long size = sizeof( this ) + m_capacity * sizeof( Slot* );

		for ( unsigned int i = 0; i < m_capacity; i++ ) {
			for ( Slot* slot = m_hashTable[ i ]; slot != m_nilSlot; slot = slot->next ) {
				size += sizeof( Slot );
			}
		}

		return size;
	}

private:

	struct Slot {
		Slot* prev;
		Slot* next;
		HashKey key;
		T value;

		Slot() {}
		Slot( HashKey _key, const T& _value, Slot* _prev, Slot* _next )
			: key(_key), value( _value ), prev(_prev), next(_next) {}
		Slot( HashKey _key, Slot* _prev, Slot* _next )
			: key(_key), value(), prev(_prev), next(_next) {}
	};

	Slot* sub_search( HashKey key ) const 
	{
		Slot* slot = m_hashTable[ HashFn::get_hash_index( key, m_capacity ) ];
		for (; slot != m_nilSlot; slot = slot->next ) {
			if ( HashFn::is_less( slot->key, key ) )
				continue;
			if ( HashFn::is_equal( key, slot->key ) ) {
				return slot;
			}
			break;
		}
		return NULL;
	}

	Slot* sub_insert( HashKey key, const T& value )
	{
		Slot*& entry = m_hashTable[ HashFn::get_hash_index( key, m_capacity ) ];
		Slot* slot = entry;

		for (; slot != m_nilSlot; slot = slot->next ) {
			m_nilSlot->prev = slot;
			if ( HashFn::is_less( slot->key, key ) ) continue;

			if ( HashFn::is_equal( key, slot->key ) ) {
				m_nilSlot->prev = m_nilSlot;
				slot->value = value;
			}
			break;
		}

		Slot* s = new Slot( key, value, slot->prev, slot );

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;
		
		slot->prev = s;
		m_nilSlot->prev = m_nilSlot;

		incCount();
		return s;
	}

	void clearSlots( Slot* slot )
	{
		while (slot != m_nilSlot) {
			Slot* n = slot->next;
			delete slot;
			slot = n;
		}
	}

	void insertSlot( Slot* s )
	{
		Slot*& entry = m_hashTable[ HashFn::get_hash_index( s->key, m_capacity ) ];
		Slot* slot = entry;

		for (; slot != m_nilSlot; slot = slot->next ) {
			m_nilSlot->prev = slot;
			if ( HashFn::is_less( s->key, slot->key ) ) break;
		}

		s->prev = slot->prev;
		s->next = slot;

		if ( slot == entry ) entry = s;
		else				 slot->prev->next = s;

		slot->prev = s;
		m_nilSlot->prev = m_nilSlot;
	}

	void initEnumState()
	{
		m_esSlotIndex = 0;
		m_esSlot = m_nilSlot;
	}

	void findFirstKey() const
	{
		for ( m_esSlotIndex = 0; m_esSlotIndex < m_capacity; m_esSlotIndex++ ) {
			for ( m_esSlot = m_hashTable[ m_esSlotIndex ]; m_esSlot != m_nilSlot; m_esSlot = m_esSlot->next ) {
				return;
			}
		}
		m_esSlot = m_nilSlot;
	}

	void findNextKey() const
	{
		if ( m_esSlot == m_nilSlot ) 
			return;

		m_esSlot = m_esSlot->next;
		while ( true ) {
			if ( m_esSlot != m_nilSlot )
				return;

			if ( ++m_esSlotIndex == m_capacity )
				break;
			m_esSlot = m_hashTable[ m_esSlotIndex ];
		}
		m_esSlot = m_nilSlot;
	}

	void incCount()
	{
		if ( bAutoRehash )
			if ( ++m_count == m_capacity * 2 )
				rehash( m_capacity * 3 + 1 );
	}

	void decCount()
	{
		if ( bAutoRehash )
			if ( --m_count * 4 < m_capacity )
				if ( m_capacity > 271 )
					rehash( m_capacity / 2 );
	}

	unsigned int m_count;

	unsigned int m_capacity;
	Slot** m_hashTable;

	struct NilSlot {
		Slot* prev;
		Slot* next;
	};

	// sentinels

	NilSlot m_nilSlotData;
	Slot* m_nilSlot;

	// enumeration state

	mutable unsigned int m_esSlotIndex;
	mutable Slot* m_esSlot;
};

#endif

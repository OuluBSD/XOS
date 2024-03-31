//
//  wclbase.h    Definitions for the base classes used by
//               the WATCOM Container List Class
//
//                          Open Watcom Project
//
//    Copyright (c) 2002-2008 Open Watcom Contributors. All Rights Reserved.
//    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
//
//    This file is automatically generated. Do not edit directly.
//
// =========================================================================
//
#ifndef _WCLBASE_H_INCLUDED
#define _WCLBASE_H_INCLUDED
#if !defined(_ENABLE_AUTODEPEND)
  #pragma read_only_file;
#endif


#ifndef __cplusplus
#error wclbase.h is for use with C++
#endif

#ifndef _WCDEFS_H_INCLUDED
 #include <wcdefs.h>
#endif

//
// This warning would be generated by taking the sizeof an object derived
// from WCSLink or WCDLink, required for user allocator and deallocators
//
// "Warning! W549: 'sizeof' operand contains compiler generated information"
#pragma warning 549 9

#if defined( new ) && defined( _WNEW_OPERATOR )
#  undef new
#endif
#if defined( delete ) && defined( _WDELETE_OPERATOR )
#  undef delete
#endif


//
//  The WCNIsvSLink class is used as a basis for non-intrusive singly
//  linked lists.
//

template<class Type>
class WCNIsvSLink : public WCSLink {
public:
    Type            data;

    inline WCNIsvSLink( const Type & datum ) : data( datum ) {};
    inline ~WCNIsvSLink() {};

    //
    // Just return ptr (passed by the new operator in WCListNew) without
    // allocating any memory.
    // Needed so we can call the constructor on user allocated memory.
    //
    inline void * operator new( size_t, void * ptr ) {
        return( ptr );
    }
};




//
//  The WCNIsvDLink class is used as a basis for non-intrusive doubly
//  linked lists.
//

template<class Type>
class WCNIsvDLink : public WCDLink {
public:
    Type            data;

    inline WCNIsvDLink( const Type & datum ) : data( datum ) {};
    inline ~WCNIsvDLink() {};

    //
    // Just return ptr (passed by the new operator in WCListNew) without
    // allocating any memory.
    // Needed so we can call the constructor on user allocated memory.
    //
    inline void * operator new( size_t, void * ptr ) {
        return( ptr );
    }
};




//
// macros to allow the user to find the size of the list elements allocated
// and freed by user allocator and deallocator functions
//

#define WCValSListItemSize( Type )      sizeof( WCNIsvSLink<Type> )
#define WCValDListItemSize( Type )      sizeof( WCNIsvDLink<Type> )
#define WCPtrSListItemSize( Type )      sizeof( WCNIsvSLink<void *> )
#define WCPtrDListItemSize( Type )      sizeof( WCNIsvDLink<void *> )




//
//  The WCIsvListBase class is used as a basis for intrusive lists.
//  It provides the common data and base functionality.  The WCExcept
//  class provides the common exception handling for all lists.
//
//  This class is also used by the value classes. The values
//  are encapsulated in non-intrusive links which are then stored
//  in the list intrusively.
//
//  It is an abstract base class to prevent objects of this class
//  from being created.
//

class WCIsvListBase : public WCExcept {
protected:
    WCSLink *       tail;
    int             entry_count;

    _WPRTLINK void           base_insert( WCSLink * );
    _WPRTLINK WCSLink *      base_next( const WCSLink *, WCbool ) const;
    _WPRTLINK int            base_index_check( int ) const;
    _WPRTLINK WCSLink *      base_sget( int );
    _WPRTLINK WCDLink *      base_dget( int );
    _WPRTLINK WCSLink *      base_bfind( int ) const;
    _WPRTLINK WCSLink *      base_sfind( int ) const;
    _WPRTLINK WCDLink *      base_dfind( int ) const;
    _WPRTLINK int            base_index( const WCSLink * ) const;
    inline WCbool            base_contains( const WCSLink * ) const;
    _WPRTLINK void           base_clear( void );
    _WPRTLINK void           base_destroy( void );
    virtual void             WCListDelete( WCSLink * datum ) = 0;

public:
    inline WCIsvListBase() : tail(0), entry_count(0) {};
    virtual ~WCIsvListBase() = 0;

    int operator==( const WCIsvListBase &rhs ) const {
        return( this == &rhs );
    };

    friend class WCListIterBase;
};

WCbool WCIsvListBase::base_contains( const WCSLink * elem ) const {
    return( base_index( elem ) != -1 );
};




//
//  The WCIsvSListBase class defines an intrusive singly linked list
//  base class.
//
//  It is an abstract base class to prevent objects of this class
//  from being created.
//

template<class Type>
class WCIsvSListBase : public WCIsvListBase {
protected:
    void            clear();
    void            clearAndDestroy();
    WCbool          insert( WCSLink * );
    WCbool          append( WCSLink * );
    WCSLink *       get( int = 0 );
    WCSLink *       find( int = 0 ) const;
    WCSLink *       findLast() const;
    int             index( int (*test_fn)( const Type *, void * )
                         , void * ) const;
    int             index( const WCSLink * ) const;
    WCbool          contains( const WCSLink * ) const;
    WCbool          isEmpty() const;
    int             entries() const;
    void            forAll( void (*)( Type *, void * ), void *) const;

    // used by clearAndDestroy
    virtual void    WCListDelete( WCSLink * datum ) {
        delete (Type *)datum;
    };

public:
    inline WCIsvSListBase( void * (*)( size_t )
                , void (*)( void *, size_t )
                ) { };

    inline WCIsvSListBase() {};
    inline virtual ~WCIsvSListBase() = 0;
};

template<class Type>
inline WCIsvSListBase<Type>::~WCIsvSListBase() {};

template<class Type>
inline void WCIsvSListBase<Type>::clear() {
    base_clear();
};

template<class Type>
inline void WCIsvSListBase<Type>::clearAndDestroy() {
    base_destroy();
};

//
// insert and append always return TRUE in an intrusive list, so that
// intrusive and pointer classes have the same prototypes for the
// WCPtrListBase class
//
template<class Type>
inline WCbool WCIsvSListBase<Type>::insert( WCSLink * datum ) {
    base_insert( datum );
    return( TRUE );
};

template<class Type>
inline WCbool WCIsvSListBase<Type>::append( WCSLink * datum ) {
    insert( datum );
    tail = datum;
    return( TRUE );
};

template<class Type>
inline WCSLink * WCIsvSListBase<Type>::get( int index ) {
    return( base_sget( index ) );
};

template<class Type>
inline WCSLink * WCIsvSListBase<Type>::find( int index ) const {
    return( base_sfind( index ) );
};

template<class Type>
inline WCSLink * WCIsvSListBase<Type>::findLast() const {
    if( tail == 0 ) {
        base_throw_empty_container();
    }
    return( tail );
};

//
// This index member function which is passed a test_fn is define for only
// the WCIsvSList classes.  Only a single index function is
// defined in WCValListBase and WCPtrSList to ensure this
// member function is unavailable to the value and pointer classes.
//
template<class Type>
inline int WCIsvSListBase<Type>::index(
                  int (*test_fn)( const Type *elem, void *datum )
                , void *datum ) const {
    int index = 0;

    Type * rover = (Type *)base_next( rover, TRUE );
    while( rover != 0 ) {
        if( (*test_fn)( rover, datum ) ) {
            return( index );
        }
        rover = (Type *)base_next( rover, FALSE );
        index++;
    }
    return( -1 );
};

template<class Type>
inline int WCIsvSListBase<Type>::index( const WCSLink * datum ) const {
    return( base_index( datum ) );
};

template<class Type>
inline WCbool WCIsvSListBase<Type>::contains( const WCSLink * datum ) const {
    return( base_contains( datum ) );
};

template<class Type>
inline WCbool WCIsvSListBase<Type>::isEmpty() const {
    return( tail == 0 );
};

template<class Type>
inline int WCIsvSListBase<Type>::entries() const {
    return( entry_count );
};

template<class Type>
void WCIsvSListBase<Type>::forAll(
                void (* func)( Type *, void * ), void * datum ) const {
    Type * rover = (Type *)base_next( rover, TRUE );
    while( rover != 0 ) {
        (*func)( rover, datum );
        rover = (Type *)base_next( rover, FALSE );
    }
};




//
//  The WCIsvDListBase class defines an intrusive doubly linked list
//  base class.
//
//  It is an abstract base class to prevent objects of this class
//  from being created.
//

template<class Type>
class WCIsvDListBase : public WCIsvListBase {
protected:
    void            clear();
    void            clearAndDestroy();
    WCbool          insert( WCDLink * );
    WCbool          append( WCDLink * );
    WCDLink *       get( int = 0 );
    WCDLink *       find( int = 0 ) const;
    WCDLink *       findLast() const;
    int             index( int (*test_fn)( const Type *, void * )
                         , void * ) const;
    int             index( const WCDLink * ) const;
    WCbool          contains( const WCDLink * ) const;
    WCbool          isEmpty() const;
    int             entries() const;
    void            forAll( void (*)( Type *, void * ), void *) const;

    // used by clearAndDestroy
    virtual void    WCListDelete( WCSLink * datum ) {
        delete (Type *)datum;
    };

public:
    inline WCIsvDListBase( void * (*)( size_t )
                , void (*)( void *, size_t )
                ) {};
    inline WCIsvDListBase() {};

    inline virtual ~WCIsvDListBase() = 0;
};

template<class Type>
inline WCIsvDListBase<Type>::~WCIsvDListBase() {};

template<class Type>
inline void WCIsvDListBase<Type>::clear() {
    base_clear();
};

template<class Type>
inline void WCIsvDListBase<Type>::clearAndDestroy() {
    base_destroy();
};

//
// insert and append always return TRUE in an intrusive list, so that
// intrusive and pointer classes have the same prototypes for the
// WCPtrListBase class
//
template<class Type>
WCbool WCIsvDListBase<Type>::insert( WCDLink * datum ) {
    WCbool empty_list = (tail == 0);
    base_insert( datum );
    if( empty_list ) {
        datum->prev.link = datum;
    } else {
        datum->prev.link = tail;
        ((WCDLink *)datum->link)->prev.link = datum;
    }
    return( TRUE );
};

template<class Type>
WCbool WCIsvDListBase<Type>::append( WCDLink * datum ) {
    insert( datum );
    tail = datum;
    return( TRUE );
};


template<class Type>
inline int WCIsvDListBase<Type>::entries() const {
    return( entry_count );
};

template<class Type>
inline WCDLink * WCIsvDListBase<Type>::get( int index ) {
    return( base_dget( index ) );
};

template<class Type>
inline WCDLink * WCIsvDListBase<Type>::find( int index ) const {
    return( base_dfind( index ) );
};

template<class Type>
inline WCDLink * WCIsvDListBase<Type>::findLast() const {
    if( tail == 0 ) {
        base_throw_empty_container();
    }
    return( (WCDLink *)tail );
};

//
// This index member function which is passed a test_fn is define for only
// the WCIsvDList class.  Only a single index function is
// defined in WCValListBase and WCPtrDList to ensure this
// member function is unavailable to the value and pointer classes.
//
template<class Type>
inline int WCIsvDListBase<Type>::index(
                  int (*test_fn)( const Type *elem, void *datum )
                , void *datum ) const {
    int index = 0;

    Type * rover = (Type *)base_next( rover, TRUE );
    while( rover != 0 ) {
        if( (*test_fn)( rover, datum ) ) {
            return( index );
        }
        rover = (Type *)base_next( rover, FALSE );
        index++;
    }
    return( -1 );
};

template<class Type>
inline int WCIsvDListBase<Type>::index( const WCDLink * datum ) const {
    return( base_index( datum ) );
};

template<class Type>
inline WCbool WCIsvDListBase<Type>::contains( const WCDLink * datum ) const {
    return( base_contains( datum ) );
};

template<class Type>
inline WCbool WCIsvDListBase<Type>::isEmpty() const {
    return( tail == 0 );
};

template<class Type>
void WCIsvDListBase<Type>::forAll(
                void (* func)( Type *, void * ), void * datum ) const {
    Type * rover = (Type *)base_next( rover, TRUE );
    while( rover != 0 ) {
        (*func)( rover, datum );
        rover = (Type *)base_next( rover, FALSE );
    }
};




//
//  The WCValListBase template class is used as a basis for non-intrusive
//  (value) lists.  The template supplies the type of the list data,
//  the methods for manipulating the list, and the link type of the list.
//
//  The member functions clear() and clearAndDestroy() perform the same
//  action since this is a value list (the list elements were created by
//  the list to contain the values, so we must destroy them.  But we can't
//  get rid of the list elements without getting rid of the values).
//
//  It is an abstract base class to prevent objects of this class
//  from being created.
//

template<class Type, class FType, class LType >
class WCValListBase : public FType {
private:
    //
    // These store the user allocator and deallocator functions for getting
    // and freeing raw memory for list links.
    // If NULL, then new and delete are used to get and destroy raw memory.
    // (see WCListNew and WCListDelete)
    //
    void * (* alloc_fn)( size_t );
    void (* dealloc_fn)( void *, size_t );
    //
    // For the value list classes, this virtual function does equivalence
    // based on Type's equivalence operator, and for pointer list classes,
    // this does equivalence based on the equivalence operator of the type
    // pointed to.  (Type's equivalence operator for WCValSList<Type>,
    // WCValDList<Type> AND WCPtrSList<Type> and WCPtrDList<Type>)
    //
    virtual int base_equivalent( const Type&, const Type& ) const = 0;
protected:
    //
    // new and delete a link, where user_alloc and user_alloc functions
    // will be called if the user supplied them.  If the user did not
    // supply the user_alloc and user_dealloc functions, then these call
    // the standard new and delete operators.
    //
    LType * WCListNew( const Type& datum );
    virtual void WCListDelete( WCSLink * );

    // value and pointer list classes copy constructor base
    void base_construct( const WCValListBase * );
    // value and pointer list classes assignment operator base
    void base_assign( const WCValListBase * );

public:
    inline WCValListBase( void * (*user_alloc)( size_t )
                , void (*user_dealloc)( void *, size_t )
                ) : alloc_fn( user_alloc ), dealloc_fn( user_dealloc ) {};
    inline WCValListBase() : alloc_fn( 0 ), dealloc_fn( 0 ) {};
    virtual ~WCValListBase() = 0;

    inline void clear()  {
        FType::clearAndDestroy();
    };

    inline void clearAndDestroy()  {
        FType::clearAndDestroy();
    };

    inline WCbool isEmpty() const {
        return( FType::isEmpty() );
    };

    inline int entries() const {
        return( FType::entries() );
    };

    inline WCbool contains( const Type &elem ) const {
        return( index( elem ) != -1 );
    };

    int index( const Type & ) const;

    Type get( int = 0 );
    Type find( int = 0 ) const;
    Type findLast() const;
    void forAll( void (*)( Type, void * ), void *) const;
    WCbool insert( const Type& datum );
    WCbool append( const Type& datum );
};


template<class Type, class FType, class LType>
LType * WCValListBase<Type,FType,LType>::WCListNew( const Type& datum ){
    LType *new_link;

    if( alloc_fn ) {
        new_link = (LType *)alloc_fn( sizeof( LType ) );
    } else {
        // assuming sizeof( char ) == 1
        new_link = (LType *)new char[ sizeof( LType ) ];
    }
    //
    // call constructor on the uninitialized memory new_link using
    // WCNIsvLinkAlloc's operator new
    //
    if( new_link ) {
        new( new_link ) LType( datum );
    }
    return( new_link );
}


template<class Type, class FType, class LType>
void WCValListBase<Type,FType,LType>::WCListDelete( WCSLink * old_link ){
    if( old_link ) {
        ( (LType *)old_link )->~LType();
        if( dealloc_fn ) {
            dealloc_fn( old_link, sizeof( LType ) );
        } else {
            delete [] (char *)old_link;
        }
    }
}


template<class Type, class FType, class LType>
void WCValListBase<Type,FType,LType>::base_construct(
                             const WCValListBase<Type,FType,LType> * olist ) {

    WCExcept::base_construct( olist );
    alloc_fn = olist->alloc_fn;
    dealloc_fn = olist->dealloc_fn;
    LType * rover = (LType *)( olist->base_next( rover, TRUE ) );
    while( rover != 0 ) {
        append( rover->data );
        rover = (LType *)( olist->base_next( rover, FALSE ) );
    }
};


template<class Type, class FType, class LType>
void WCValListBase<Type,FType,LType>::base_assign(
                              const WCValListBase<Type,FType,LType> * olist ) {
    if( this != (WCValListBase * const)olist ) {
        base_destroy();
        base_construct( olist );
    }
};


template<class Type, class FType, class LType>
WCValListBase<Type,FType,LType>::~WCValListBase() {
    if( tail != 0 ) {
        base_throw_not_empty();
        // we can destroy the list, since we allocated it
        base_destroy();
    }
};


template<class Type, class FType, class LType>
WCbool WCValListBase<Type,FType,LType>::insert( const Type& datum ) {
    LType * new_link = WCListNew( datum );
    if( new_link != 0 ) {
        return( FType::insert( new_link ) );
    } else {
        base_throw_out_of_memory();
        return( FALSE );
    }
};


template<class Type, class FType, class LType>
WCbool WCValListBase<Type,FType,LType>::append( const Type& datum ) {
    LType * new_link = WCListNew( datum );
    if( new_link != 0 ) {
        return( FType::append( new_link ) );
    } else {
        base_throw_out_of_memory();
        return( FALSE );
    }
};


template<class Type, class FType, class LType>
int WCValListBase<Type,FType,LType>::index(
                                        const Type & lookup_data ) const {
    int index = 0;
    LType * rover = (LType *)base_next( rover, TRUE );

    while( rover != 0 ) {
        if( base_equivalent( rover->data, lookup_data ) ) {
            return( index );
        }
        rover = (LType *)base_next( rover, FALSE );
        index++;
    }
    return( -1 );
};


template<class Type, class FType, class LType>
Type WCValListBase<Type,FType,LType>::get( int index ) {
    LType * ret_obj = (LType *)FType::get( index );
    if( ret_obj == 0 ) {
        Type ret_val;
        return( ret_val );
    } else {
        Type ret_val = ret_obj->data;
        WCListDelete( ret_obj );
        return( ret_val );
    }
};


template<class Type, class FType, class LType>
Type WCValListBase<Type,FType,LType>::find( int index ) const {
    LType * ret_obj = (LType *)FType::find( index );
    if( ret_obj == 0 ) {
        Type ret_val;
        return( ret_val );
    } else {
        Type ret_val = ret_obj->data;
        return( ret_val );
    }
};


template<class Type, class FType, class LType>
Type WCValListBase<Type,FType,LType>::findLast() const {
    LType * ret_obj = (LType *)FType::findLast();
    if( ret_obj == 0 ) {
        Type ret_val;
        return( ret_val );
    } else {
        Type ret_val = ret_obj->data;
        return( ret_val );
    }
};


template<class Type, class FType, class LType>
void WCValListBase<Type,FType,LType>::forAll(
                  void (* func)( Type, void * ), void * datum ) const {
    LType * rover = (LType *)base_next( rover, TRUE );
    while( rover != 0 ) {
        (*func)( rover->data, datum );
        rover = (LType *)base_next( rover, FALSE );
    }
};




//
//  The WCPtrListBase template class is used as a basis for the WCPtrSList,
//  WCPtrDList, WCIsvSList and WCIsvDList classes. The template supplies
//  the type of the list data and the methods for manipulating the list.
//
//  It is an abstract base class to prevent objects of this class
//  from being created.
//

template<class Type, class FType>
class WCPtrListBase : public FType {
public:
    inline WCPtrListBase( void * (*user_alloc)( size_t )
                , void (*user_dealloc)( void *, size_t )
                ) : FType ( user_alloc, user_dealloc ) {};

    inline WCPtrListBase() : FType( 0, 0 ) {};

    inline virtual ~WCPtrListBase() = 0;

    inline void clear() {
        FType::clear();
    };

    inline WCbool insert( Type * ptr )  {
        return( FType::insert( ptr ) );
    };

    inline WCbool append( Type * ptr )  {
        return( FType::append( ptr ) );
    };

    inline WCbool contains( const Type * ptr ) const {
        return( FType::contains( (Type *)ptr ) );
    };

    //
    // the index member function is not inlined here since intrusive
    // and pointer classes inherit the index member functions with different
    // prototypes
    //

    inline WCbool isEmpty() const {
        return( FType::isEmpty() );
    };

    inline int entries() const {
        return( FType::entries() );
    };

    inline Type * get( int index = 0 ) {
        return( (Type *)FType::get( index ) );
    };

    inline Type * find( int index = 0 ) const {
        return( (Type *)FType::find( index ) );
    };

    inline Type * findLast() const {
        return( (Type *)FType::findLast() );
    };

    inline void forAll( void (* func)( Type *, void * ), void * datum ) const {
        FType::forAll( func, datum );
    };

    void clearAndDestroy();
};

template<class Type, class FType>
inline WCPtrListBase<Type,FType>::~WCPtrListBase() {};


template<class Type, class FType>
void WCPtrListBase<Type,FType>::clearAndDestroy() {
    Type *  ptr_val;
    while( !isEmpty() ) {
        ptr_val = get();
        delete ptr_val;
    }
    clear();
}

#if defined( _WNEW_OPERATOR )
#  define new _WNEW_OPERATOR
#endif
#if defined( _WDELETE_OPERATOR )
#  define delete _WDELETE_OPERATOR
#endif

#endif

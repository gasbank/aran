#pragma once

template <typename T> class Singleton
{
	static T* s_instance;

public:
    Singleton( void )
    {
        assert( !s_instance );
        s_instance = static_cast<T*>(this);
    }
   ~Singleton( void )
        {  assert( s_instance );  s_instance = 0;  }
    static T& getSingleton( void )
        {  assert( s_instance );  return ( *s_instance );  }
    static T* getSingletonPtr( void )
        {  return ( s_instance );  }
};


#define IMPLEMENT_SINGLETON(type)	\
	template<> type* Singleton<type>::s_instance	= 0;
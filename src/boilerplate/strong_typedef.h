#ifndef STRONG_TYPEDEF_H
#define STRONG_TYPEDEF_H
#include <QUuid>
#include <QSharedPointer>

namespace Boilerplate {

template< typename VALUE, typename SFINAE = void >
struct StrongTypedefValidator;

template<>
struct StrongTypedefValidator< std::string, std::enable_if< true >::type > final
{
    bool operator ()( const std::string & value ) const noexcept
    {
        return !value.empty();
    }
};

template<>
struct StrongTypedefValidator< QString, std::enable_if< true >::type > final
{
    bool operator ()( const QString & value ) const noexcept
    {
        return !value.isNull() && !value.isEmpty();
    }
};

template<>
struct StrongTypedefValidator< QUuid, std::enable_if< true >::type > final
{
    bool operator ()( const QUuid & value ) const noexcept
    {
        return !value.isNull();
    }
};

template< typename VALUE >
struct StrongTypedefValidator< VALUE, typename std::enable_if< std::is_arithmetic< VALUE >::value >::type > final
{
    bool operator ()( const VALUE & value ) const noexcept
    {
        return value > 0;
    }
};

template< typename VALUE >
struct StrongTypedefComparator final
{
    bool operator ()( const VALUE & lh, const VALUE & rh ) const noexcept
    {
        return std::less< VALUE >()( lh, rh );
    }
};

template<>
struct StrongTypedefComparator< QUuid > final
{
    bool operator ()( const QUuid & lh, const QUuid & rh ) const noexcept
    {
        return lh.toString() < rh.toString();
    }
};

template < typename VALUE, typename SFINAE = void >
struct StrongTypedefDefaulter final
{
    VALUE operator()() const noexcept
    {
        return {};
    }
};

template < typename VALUE >
struct StrongTypedefDefaulter< VALUE, typename std::enable_if< !std::is_arithmetic< VALUE >::value >::type > final
{
    VALUE operator()() const noexcept
    {
        return 0;
    }
};

template<
    typename VALUE,
    typename CONVERTER = void,
    typename VALIDATOR = StrongTypedefValidator< VALUE >,
    typename COMPARATOR = StrongTypedefComparator< VALUE >,
    typename DEFAULTER = StrongTypedefDefaulter< VALUE >
>
class StrongTypedef final
{
public:
    using Self = StrongTypedef< VALUE, CONVERTER, VALIDATOR, COMPARATOR, DEFAULTER >;
    using Ptr = QSharedPointer< Self >;
    using ConstPtr = QSharedPointer< const Self >;

public:
    StrongTypedef() noexcept : value{ DEFAULTER{}() } {}

    StrongTypedef( const StrongTypedef & other ) = default;
    StrongTypedef & operator =( const StrongTypedef & other ) = default;

    StrongTypedef( StrongTypedef && other ) noexcept = default;
    StrongTypedef & operator =( StrongTypedef && other ) noexcept = default;

    explicit StrongTypedef( const VALUE & value ) noexcept
            : value( value )
    {

    }

    explicit StrongTypedef( VALUE && value ) noexcept
            : value( std::forward< VALUE >( value ) )
    {

    }

//    template< typename Other >
//    explicit StrongTypedef( const Other & value ) noexcept
//        : value( CONVERTER ()( value ) )
//    {

//    }

    bool isValid() const noexcept
    {
        return VALIDATOR()( value );
    }

    const VALUE & get() const noexcept
    {
        return value;
    }

    bool operator <( const Self & other ) const noexcept
    {
        return COMPARATOR()( value, other.value );
    }

    bool operator >( const Self & other ) const noexcept
    {
        return other < *this;
    }

    bool operator <=( const Self & other ) const noexcept
    {
        return !( other < *this );
    }

    bool operator >=( const Self & other ) const noexcept
    {
        return !( *this < other );
    }

    bool operator ==( const Self & other ) const noexcept
    {
        return value == other.value;
    }

    bool operator !=( const Self & other ) const noexcept
    {
        return !( other == *this );
    }

    Self & operator ++() noexcept
    {
        ++value;
        return *this;
    }

    Self & operator --() noexcept
    {
        --value;
        return *this;
    }

private:
    VALUE value;
};

template<
    typename VALUE,
    typename CONVERTER = void,
    typename VALIDATOR = StrongTypedefValidator< VALUE >,
    typename COMPARATOR = StrongTypedefComparator< VALUE >,
    typename DEFAULTER = StrongTypedefDefaulter< VALUE >
>
uint qHash( const StrongTypedef< VALUE, CONVERTER, VALIDATOR, COMPARATOR, DEFAULTER > & id, uint seed = 0 ) noexcept
{
    return ::qHash( id.get(), seed );
}

}
#endif // STRONG_TYPEDEF_H

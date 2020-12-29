#ifndef SINGLY_LINKED_STACK_H
#define SINGLY_LINKED_STACK_H
#include <QStack>
#include <QSharedPointer>

template<typename Y>
class SinglyLinkedElement
{
public:
    SinglyLinkedElement() = default;
    virtual ~SinglyLinkedElement() = default;
    void setNext(const Y &next);
    inline Y getNext() const
    {
        return m_next;
    }
private:
    Y m_next = nullptr;
};

template< typename Y >
void SinglyLinkedElement< Y >::setNext(const Y &next)
{
    m_next = next;
}

template<typename T>
class SinglyLinkedStack final : public QStack<T >
{
public:
    SinglyLinkedStack();
    ~SinglyLinkedStack();

    void insert_back(const T &t);
    void remove();
    void setHead(const T &head);
private:
    T m_head = nullptr;
};

template< typename T >
SinglyLinkedStack< T >::SinglyLinkedStack() : m_head( nullptr ) {
}

template< typename T >
SinglyLinkedStack< T >::~SinglyLinkedStack() {
    while( m_head ) {
        remove();
    }
}

template< typename T >
void SinglyLinkedStack< T >::insert_back( const T &t ) {
    // Новый узел привязывается к старому головному элементу
    if( !m_head.isNull() ) {
        t->setNext(m_head->getNext());
        m_head->setNext(t);
    }
    // Новый узел сам становится головным элементом
    setHead(t);
    QStack<T>::push(t);
}

template< typename T >
void SinglyLinkedStack< T >::remove() {
    if( !m_head.isNull() ) { // Если список не пуст:
        // Сохраняем указатель на второй узел, который станет новым головным элементом
        auto newHead = m_head->getNext();
        QStack<T>::pop();
        m_head.clear();

        // Назначаем новый головной элемент
        m_head = newHead;
    } // Иначе могли бы возбудить исключение
}

template< typename T >
void SinglyLinkedStack< T >::setHead(const T &head)
{
    m_head = head;
}

#endif // SINGLY_LINKED_STACK_H

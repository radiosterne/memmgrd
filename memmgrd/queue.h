/*
 *@project memmgrd
 *@file queue.h
 *@brief Definition and implementation of non-blocking thread-safe queue
 *@author Andrew Kurnoskin
 *@date 06/12/2012
 */

#ifndef QUEUE_H
#define QUEUE_H


#include <stdexcept>


/*
 *@brief Provides thread-safe non-blocking queue
 */
template <typename T> class queue
{
public:
    queue();
    ~queue();
    void push(T& data);
    void pop();
    T front();
    bool is_empty();
private:

    /*
     *@brief Nested class to use as queue element
     */
    struct queue_node
    {
        queue_node* next;
        T data;
    };

    queue_node head;
    queue_node* tail;
};

/*
 *@brief Does pointer magic
 */
template <typename T> queue<T>::queue()
{
    this->head.next = 0;
    this->tail = &this->head;
}

/*
 *@brief Unwinds the queue
 */
template <typename T> queue<T>::~queue()
{
    while(!this->is_empty())
        this->pop();
}


/*
 *@brief Puts new object in the queue
 *@param T& data an object to push
 */
template <typename T> void queue<T>::push(T& data)
{
    queue_node * ptr = new queue_node;
    ptr->data = data;
    ptr->next = 0;
    queue_node* t_ptr = 0;
    while(1)
    {
        t_ptr = this->tail;
        if(__sync_val_compare_and_swap(&this->tail, t_ptr, ptr) == t_ptr)
        {
            t_ptr->next = ptr;
            break;
        }
    }
}

/*
 *@brief Removes firs element from the queue
 *@exception std::logic_error, if the queue is empty
 */
template <typename T> void queue<T>::pop()
{
    if(this->head.next == 0)
        throw std::logic_error(std::string("trying to unwind empty queue!"));

    bool ok = false;
    queue_node *ptr;
    while(!ok)
    {
        ptr = this->head.next;
        if(ptr == this->tail)
        {
            this->head.next = ptr->next;
            if(__sync_val_compare_and_swap(&this->tail, ptr, &this->head) == ptr)
                ok = true;

        }
        else
        {
            this->head.next = ptr->next;
            ok = true;
        }
    }
    delete ptr;
}


/*
 *@brief Returns first element in the queue
 *@exception std::logic_error, if the queue is empty
 */
template <typename T> T queue<T>::front()
{
    if(this->head.next == 0)
        throw std::logic_error(std::string("trying to unwind empty queue!"));
    return this->head.next->data;
}

/*
 *@brief Checks whether the queue is empty
 */
template <typename T> bool queue<T>::is_empty()
{
    return (this->head.next == 0);
}

#endif // QUEUE_H

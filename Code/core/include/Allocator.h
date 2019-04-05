#pragma once

#include "Meta.h"

class Allocator;
struct AllocatorCompatible
{
    void SetAllocator(Allocator* apAllocator) { m_pAllocator = apAllocator; }
    Allocator* GetAllocator() { return m_pAllocator; }

private:

    Allocator* m_pAllocator;
};

namespace details
{
    template<typename T>
    using has_allocator_t = decltype(std::declval<T&>().GetAllocator());

    template<typename T>
    constexpr bool has_allocator = is_detected_v<has_allocator_t, T>;
}

class Allocator
{
public:

    virtual ~Allocator() {}
    virtual void* Allocate(size_t aSize) = 0;
    virtual void Free(void* apData) = 0;
    virtual size_t Size(void* apData) = 0;

    template<class T>
    T* New()
    {
        auto pData = (T*)Allocate(sizeof(T));
        if (pData)
        {
            if constexpr (details::has_allocator<T>)
                pData->SetAllocator(this);

            return new (pData) T();
        }

        return nullptr;
    }

    template<class T, class... Args>
    T* New(Args... args)
    {
        auto pData = (T*)Allocate(sizeof(T));
        if (pData)
        {
            if constexpr (details::has_allocator<T>)
                pData->SetAllocator(this);

            return new (pData) T(std::forward<Args...>(args...));
        }

        return nullptr;
    }

    template<class T>
    void Delete(T* apData)
    {
        if (apData == nullptr) 
            return;

        apData->~T();
        Free(apData);
    }

    static void Push(Allocator* apAllocator);
    static Allocator* Pop();
    static Allocator* Get();
    static Allocator* GetDefault();

private:

    enum
    {
        kMaxAllocatorCount = 1024
    };

    static thread_local Allocator* s_allocatorStack[kMaxAllocatorCount];
    static thread_local int s_currentAllocator;
};


struct ScopedAllocator
{
    ScopedAllocator(Allocator* apAllocator);
    ~ScopedAllocator();

private:
    Allocator* m_pAllocator;
};

template<class T>
T* New()
{
    if constexpr(details::has_allocator<T>)
        return Allocator::Get()->New<T>();
    else
        return Allocator::GetDefault()->New<T>();
}

template<class T, class... Args>
T* New(Args... args)
{
    if constexpr (details::has_allocator<T>)
        return Allocator::Get()->New<T>(std::forward<Args...>(args...));
    else
        return Allocator::GetDefault()->New<T>(std::forward<Args...>(args...));
}

template<class T>
void Delete(T* apEntry)
{
    if constexpr (details::has_allocator<T>)
    {
        apEntry->GetAllocator()->Delete(apEntry);
    }
    else
    {
        Allocator::GetDefault()->Delete(apEntry);
    }
}
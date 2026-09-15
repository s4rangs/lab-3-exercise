#ifndef SHARED_PTR_HEADER
#define SHARED_PTR_HEADER

#include <cassert>
#include <utility>

class ControlBlockBase {
public:
    ControlBlockBase() : refcount(1) {} // TODO: implement the default constructor.

    // dtor is virtual, so that we can call derived class's dtor from a ptr to this base class.
    virtual ~ControlBlockBase() {} // TODO: implement the destructor.

    // pure virtual function; must be overriden by derived classes
    virtual void* managedAddress() = 0;

    // Delete copies, which also implicitly deletes moves.
    ControlBlockBase(const ControlBlockBase&) = delete;
    ControlBlockBase& operator=(const ControlBlockBase&) = delete;

    long increment()
    {
        // TODO: increment refcount by 1 and return result.
        return ++refcount;
    }

    long decrement()
    {
        // TODO: decrement refcount by 1 and return result.
        assert(refcount > 0); // cant decrement if 0 or less
        return --refcount;
    }

    long refCount() const
    {
        // TODO: just return the refcount.
        return refcount;
    }

private:
    // TODO: add field(s) which both control block types need to have
    long refcount;
};

template <typename T>
class ControlBlock : public ControlBlockBase {
public:
    ControlBlock(T* ptr) : managedPtr(ptr) {} // constructor and takes ownership passed in raw ptr
    ~ControlBlock() override{
        delete managedPtr;
    }
    void* managedAddress() override {
        return managedPtr; // managedPtr is address and ptr is ptr eheh
    }
private:
    T* managedPtr;
};

template <typename T>
class SharedPtr {
private:
    T* storedPtr;
    ControlBlockBase* controlBlock;

public:
    //initialization:
    SharedPtr() : storedPtr(nullptr), controlBlock(nullptr) {}
    SharedPtr(T* ptr) : storedPtr(ptr), controlBlock(new ControlBlock<T>(ptr)) {}

    //destruction
    ~SharedPtr() {
        if (controlBlock != nullptr && controlBlock->decrement() == 0) {
            delete controlBlock;
        }
    }
    //copy semantics
    SharedPtr(const SharedPtr& other) : storedPtr(other.storedPtr), controlBlock(other.controlBlock) { //shared ownership of same resource
        if (controlBlock != nullptr) { //need to share because default sharedPtr has controlblock == nullptr
            controlBlock->increment();
        }
    }
    // move semantics
    SharedPtr(SharedPtr&& other) noexcept : storedPtr(other.storedPtr), controlBlock(other.controlBlock){
        other.storedPtr = nullptr;
        other.controlBlock = nullptr;
    }
    //copy assignment operators
    SharedPtr& operator=(const SharedPtr&other) {
        if (this!= &other) {
            SharedPtr temp(other);
            swap(temp);
        }
        return *this;
    }
    //move assignment operators
    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other) {
            if (controlBlock != nullptr && controlBlock->decrement() == 0) {
                delete controlBlock;
            }

            storedPtr = other.storedPtr;
            controlBlock = other.controlBlock;

            other.storedPtr = nullptr;
            other.controlBlock = nullptr;
        }
        return *this;
    }
    //dereference operator
    T& operator*() const {
        assert(storedPtr != nullptr);
        return *storedPtr;
    }

    //arrow operator
    T* operator->() const {
        assert(storedPtr != nullptr);
        return storedPtr;
    }
    
    T* get() const{
        return storedPtr;
    }
    bool operator==(const SharedPtr<T>& other) const {
        return storedPtr == other.storedPtr;
    }
    explicit operator bool () const{
        return storedPtr != nullptr;
    }
    void swap(SharedPtr<T>& other) noexcept {
        // need to swap both pointers 
        T* tempS = storedPtr;
        storedPtr = other.storedPtr;
        other.storedPtr = tempS;

        ControlBlockBase* tempC = controlBlock;
        controlBlock = other.controlBlock;
        other.controlBlock = tempC;
    }
    void reset() {
        SharedPtr temp;
        swap(temp);
    }
    void reset(T* other) {
        SharedPtr temp(other);
        swap(temp);
    }
    long useCount () const {
        if (controlBlock == nullptr) {
            return 0;
        }
        return controlBlock->refCount();
    }

};

template <typename T, typename... Args> SharedPtr<T> makeSharedBasic(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    return SharedPtr<T>(ptr);
}

#endif

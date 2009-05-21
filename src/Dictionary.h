#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <QHash>

/*
 * The purpose of this class is to have an efficient bi-directional
 * hash map similar to QHash. Actually, the interface mimicks QHash
 * as close as possible, but all operations are efficient in both
 * directions. And storage requirements are reasonable, with each
 * object stored only once.
 * 
 * A sample use case would be translating color names to QColor
 * and using the same mapping for reverse translation.
 */

template <typename KeyL, typename KeyR>
class Dictionary
{
    protected:
        // The pointers are actually iterators over the other hash map
        typedef QHash<KeyL, void*> HashL;
        typedef QHash<KeyR, void*> HashR;
        
        HashL hashL;
        HashR hashR;
        
    protected:
        // These convert left iterators to right iterators and back
        static typename HashR::iterator left2right(typename HashL::iterator il)
            { return typename HashR::iterator(il.value()); }
        static typename HashR::const_iterator left2right(typename HashL::const_iterator il)
            { return typename HashR::const_iterator(il.value()); }
        static typename HashL::iterator right2left(typename HashR::iterator ir)
            { return typename HashL::iterator(ir.value()); }
        static typename HashL::const_iterator right2left(typename HashR::const_iterator ir)
            { return typename HashL::const_iterator(ir.value()); }

        template<typename I>
        class iterator_base
        {
            friend class Dictionary;
        
            protected:
                I i;
                
                iterator_base(const I& i): i(i) {}

            public:
                iterator_base() {}
                iterator_base(const iterator_base<I>& other): i(other.i) {}
                const KeyL& left() { return i.key(); }
                const KeyR& right() { return left2right(i).key(); }
                bool operator!= (const iterator_base<I>& other) const { return i!=other.i; }
                iterator_base<I> operator+(int j) const { return i+j; }
                iterator_base<I>& operator++() { return ++i; }
                iterator_base<I> operator++(int) { return i++; }
                iterator_base<I>& operator+=(int j) { return i+=j; }
                iterator_base<I> operator-(int j) const { return i-j; }
                iterator_base<I>& operator--() { return --i; }
                iterator_base<I> operator--(int) { return i--; }
                iterator_base<I>& operator-=(int j) { return i-=j; }
                bool operator==(const iterator_base<I>& other) const { return i==other.i; }
        };
        
    public:
        // It is sufficient to store only one iterator,
        // we arbitrarily choose the left one
        typedef iterator_base<typename HashL::iterator> iterator;
        typedef iterator_base<typename HashL::const_iterator> const_iterator;

    public:
        iterator begin() { return hashL.begin(); }
        const_iterator begin() const { return hashL.begin(); }
        int capacity() const { return hashL.capacity(); }
        void clear() { hashL.clear(); hashR.clear(); }
        const_iterator constBegin() const { return this->begin(); }
        const_iterator constEnd() const { return this->end(); }
        const_iterator constFind(const KeyL& keyL) const { return findL(keyL); }
        const_iterator constFind(const KeyR& keyR) const { return findR(keyR); }
        bool contains(const KeyL& keyL) const { return containsL(keyL); }
        bool contains(const KeyR& keyR) const { return containsR(keyR); }
        bool containsL(const KeyL& keyL) const { return hashL.contains(keyL); }
        bool containsR(const KeyR& keyR) const { return hashR.contains(keyR); }
        int count(const KeyL& keyL) const { return countL(keyL); }
        int count(const KeyR& keyR) const { return countR(keyR); }
        int countL(const KeyL& keyL) const { return hashL.count(keyL); }
        int countR(const KeyR& keyR) const { return hashR.count(keyR); }
        int count() const { return hashL.count(); }
        bool empty() const { return hashL.empty(); }
        iterator end() { return hashL.end(); }
        const_iterator end() const { return hashL.end(); }
        iterator erase(iterator pos)
        {
            hashR.erase(left2right(pos.i));
            return hashL.erase(pos.i);
        }
        iterator find(const KeyL& keyL) { return findL(keyL); }
        iterator find(const KeyR& keyR) { return findR(keyR); }
        const_iterator find(const KeyL& keyL) const { return findL(keyL); }
        const_iterator find(const KeyR& keyR) const { return findR(keyR); }
        iterator findL(const KeyL& keyL) { return hashL.find(keyL); }
        iterator findR(const KeyR& keyR)
        {
            typename HashR::iterator ir = hashR.find(keyR);
            if (ir == hashR.end()) return hashL.end();
            return right2left(ir);
        }
        const_iterator findL(const KeyL& keyL) const { return hashL.find(keyL); }
        const_iterator findR(const KeyR& keyR) const
        {
            typename HashR::const_iterator ir = hashR.find(keyR);
            if (ir == hashR.end()) return hashL.end();
            return right2left(ir);
        }
        iterator insert(const KeyL& keyL, const KeyR& keyR)
        {
            typename HashL::iterator il = hashL.insert(keyL, 0);
            typename HashR::iterator ir = hashR.insert(keyR, il);
            il.value() = ir;
            return il;
        }
        iterator insertMulti(const KeyL& keyL, const KeyR& keyR)
        {
            typename HashL::iterator il = hashL.insertMulti(keyL, 0);
            typename HashR::iterator ir = hashR.insertMulti(keyR, il);
            il.value() = ir;
            return il;
        }
        bool isEmpty() const { return hashL.isEmpty(); }
        const KeyL left(const KeyR& keyR) const
        {
            typename HashR::const_iterator ir = hashR.find(keyR);
            if (ir == hashR.end()) return KeyL();
            return right2left(ir).key();
        }
        const KeyL left(const KeyR& keyR, const KeyL& defaultKeyL) const
        {
            typename HashR::const_iterator ir = hashR.find(keyR);
            if (ir == hashR.end()) return defaultKeyL;
            return right2left(ir).key();
        }
        QList<KeyL> lefts() const { return hashL.keys(); }
        QList<KeyL> lefts(const KeyR& keyR) const
        {
            QList<KeyL> ret;
            typename HashR::const_iterator ir = hashR.find(keyR);
            while (ir != hashR.end() && ir.key() == keyR)
            {
                ret.append(right2left(ir).key());
                ++ir;
            }
            return ret;
        }
        int remove(const KeyL& keyL) { return removeL(keyL); }
        int remove(const KeyR& keyR) { return removeR(keyR); }
        int removeL(const KeyL& keyL)
        {
            typename HashL::iterator il = hashL.find(keyL);
            while (il != hashL.end() && il.key() == keyL)
            {
                hashR.erase(left2right(il));
                ++il;
            }
            return hashL.remove(keyL);
        }
        int removeR(const KeyR& keyR)
        {
            typename HashR::iterator ir = hashR.find(keyR);
            while (ir != hashR.end() && ir.key() == keyR)
            {
                hashL.erase(right2left(ir));
                ++ir;
            }
            return hashR.remove(keyR);
        }
        void reserve(int size) { hashL.reserve(size); hashR.reserve(size); }
        const KeyR right(const KeyL& keyL) const
        {
            typename HashL::const_iterator il = hashL.find(keyL);
            if (il == hashL.end()) return KeyR();
            return left2right(il).key();
        }
        const KeyR right(const KeyL& keyL, const KeyR& defaultKeyR) const
        {
            typename HashL::const_iterator il = hashL.find(keyL);
            if (il == hashL.end()) return defaultKeyR;
            return left2right(il).key();
        }
        QList<KeyR> rights() const { return hashR.keys(); }
        QList<KeyR> rights(const KeyL& keyL) const
        {
            QList<KeyR> ret;
            typename HashL::const_iterator il = hashL.find(keyL);
            while (il != hashL.end() && il.key() == keyL)
            {
                ret.append(left2right(il).key());
                ++il;
            }
            return ret;
        }
        int size() const { return hashL.size(); }
        void squeeze() { hashL.squeeze(); hashR.squeeze(); }
        KeyR take(const KeyL& keyL) { return takeL(keyL); }
        KeyL take(const KeyR& keyR) { return takeR(keyR); }
        KeyR takeL(const KeyL& keyL)
        {
            typename HashL::iterator il = hashL.find(keyL);
            if (il == hashL.end()) return KeyR();
            KeyR ret = left2right(il).key();
            hashR.erase(left2right(il));
            hashL.erase(il);
            return ret;
        }
        KeyL takeR(const KeyR& keyR)
        {
            typename HashR::iterator ir = hashR.find(keyR);
            if (ir == hashR.end()) return KeyL();
            KeyL ret = right2left(ir).key();
            hashL.erase(right2left(ir));
            hashR.erase(ir);
            return ret;
        }
        QList<KeyL> uniqueKeysL() const { return hashL.uniqueKeys(); }
        QList<KeyR> uniqueKeysR() const { return hashR.uniqueKeys(); }

        /*
         * NOT IMPLEMENTED
         *
        QHash<Key, T> & unite ( const QHash<Key, T> & other )
        bool operator!= ( const QHash<Key, T> & other ) const
        bool operator== ( const QHash<Key, T> & other ) const
        */
};

#endif

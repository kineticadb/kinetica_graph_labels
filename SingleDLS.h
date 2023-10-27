#ifndef __SINGLEDLS_H__
#define __SINGLEDLS_H__

#include <vector>
#include <iostream>
#include <ostream>
///////////////////////////////////////////////////////////////////////////////////////////////////
/// Inspired and modified from the DLS Container introduced by the following citation:
/// Karamete BK., Aubry, R., Mestreau E., Dey S., ‘A Novel Double Link Structure (DLS) with Application to
/// Computational Engineering and Design’, 54th AIAA- Aerospace Sciences Meeting, Jan 4-8 2016 San
/// Diego, AIAA 2016-1301.
/// Single in-place linked list impl between the nodes/edges ids --> pair indexes (label)
/// Each graph entity has a label pair index, next entity that has the same index - m_list's one element
/// Each pair index has a cached entity index;
/// The purpose of this class is to have relationships of all labels belonging to each item w/o having to use a multimap
/// Author: Karamete - Aug, 2023
/// //////////////////////////////////////////////////////////////////////////////////////////////
class SingleDLS {
    
protected:
	std::vector<std::size_t> m_list;  ///- ith element has two values; the pair index for the ith entity and the next entity id
	std::vector<std::size_t> m_cache; ///- pair index's cached entity index to start unraveling process
    
public:
    /// C'tor
    SingleDLS() { clear(); }
    
    /// D'tor
    ~SingleDLS(){}
    
    /// Gets the linked list vector
    std::vector<std::size_t> &get() {return m_list;}

    /// Gets the linked list vector - Const Variety
    const std::vector<std::size_t> &get() const {return m_list;}

    /// Clears all
    void clear() { m_list.clear(); m_cache.clear(); }

    /// returns the pair index (label) of an entity item
    std::size_t get_label(std::size_t item) const
    {
        return (2*item >= m_list.size() ? 0 : m_list[2*item]);
    }
    
    /// inserts a pair index (label) to an item
    bool insert(std::size_t item, std::size_t label)
    {
        // get the label's cache
        if(label >= m_cache.size())
            m_cache.resize(label+1, 0);
        
        std::size_t cached = m_cache[label];
        
        // set the cached as the previous of the item
        std::size_t twoitem = 2*item;
        
        if(twoitem >= m_list.size())
            m_list.resize(twoitem+1);
        
        std::size_t olabel = get_label(item);
        if(olabel == label)
            return false;
        
        if(olabel)
            del_item(item);
        
        m_list[twoitem]   = label;
        m_list[twoitem-1] = cached;
        
        m_cache[label] = item;
        return true;
    }
    
    /// returns the number of items
    std::size_t size_items() const 
    {
        return m_list.size()/2;
    }
    
    /// returns the number of labels
    std::size_t size_labels() const 
    {
        return m_cache.size()-1;
    }

    /// resize labels
    void resize_labels(std::size_t old_index)
    {
        m_cache.resize(old_index);
    }

    /// returns the items whose labels all deleted
    std::size_t size_deleted() const
    {
        std::size_t cnt = 0;
        for(std::size_t i = 1; i <= m_list.size()/2; ++i)
        {
           if(is_deleted(i)) ++cnt;
        }
        return cnt;
    }
    
    /// deletes the item's all labels
    bool del_item(std::size_t item)
    {
        std::size_t label = get_label(item);
        if(!label)
            return false;
        std::size_t nextprev = m_list[2*item-1];        
        std::size_t cached = m_cache[label];
        
        if(cached == item)
            m_cache[label] = nextprev;        
        
        m_list[2*item] = 0;
        m_list[2*item-1] = 0;
        
        while(std::size_t prev = m_list[2*cached-1])
        {
            if(prev == item)
            {
                m_list[2*cached-1] = nextprev;
                return true;
            }
            cached = prev;
        }
        
        return true;
    }
       
    /// Returns true if the labels of an item is deleted
    bool is_deleted(std::size_t item) const
    {
        std::size_t twoitem = 2*item;
        return  (twoitem < m_list.size() ? (!m_list[twoitem] && !m_list[twoitem-1]) : false);
    }
    
    /// Returns true if the label has no associated item
    bool is_deleted_label(std::size_t label) const
    {
        return !m_cache[label];
    }
    
    /// Returns the items associated with a label
    std::size_t get(std::size_t label, std::vector<std::size_t> &items, bool clear = true) const
    {
        if(clear)
            items.clear();
        if(label >= m_cache.size())
            return 0;
        std::size_t cached = m_cache[label];
        
        if(!cached)
            return 0;
        items.push_back(cached);            
        while(std::size_t prev = m_list[2*cached-1])
        {
            items.push_back(prev);
            cached = prev;
        }
        return items.size();
    }
    
    /// Prints all
    void print(std::ostream &out = std::cout) const
    {
        out << "Singly L-List: " << std::endl;
        for(std::size_t i = 1; i < m_list.size(); i=i+2)
            out << (i+1)/2 << ": " << m_list[i] << " " << m_list[i+1] << std::endl;
        
        out << "Index Cached Node:" << std::endl;
        for(std::size_t i = 1; i < m_cache.size(); ++i)
            out << i << ": " << m_cache[i] << std::endl;
    }
    
    /// Prints the label's associated items
    void print_label(std::size_t label, std::ostream &out = std::cout) const
    {
        std::vector<std::size_t> items;
        get(label,items);        
        for(std::size_t item : items)
            out << item << std::endl;        
    }
    
    /// Prints the associated items for all the labels
    void print_labels(std::ostream &out = std::cout) const
    {       
        for(std::size_t label = 1; label < m_cache.size(); ++label)
        {
            out << "label=" << label << std::endl;
            print_label(label, out);
        }
    }

    /// Inserts the tuples of item,label pairs
    void populate(const std::vector<std::size_t> &pairs) 
    {
        clear();
        for(std::size_t i = 0; i < pairs.size(); i = i+2)
            insert(pairs[i], pairs[i+1]);        
    }
    
    /// Serialized write to a binary output stream
    void write(std::ostream &out) const
    {
        write(out, m_list);
        write(out, m_cache);
    }
    
    /// Serialized read from a binary input stream
    void read(std::istream &in)
    {
        clear();
        read(in, m_list);
        read(in, m_cache);
    }
    
    /// Returns the memory occupied
    std::size_t memory() const
    {
        return memory(m_list) + memory(m_cache);
    }
    
    /// Utils
    static std::size_t memory(const std::vector<std::size_t> &vec)
    {
        return sizeof(std::vector<std::size_t>) + vec.capacity()*sizeof(std::size_t);
    }

    static std::size_t memory(const std::vector<std::vector<std::size_t>> &vecs)
    {
        std::size_t total = sizeof(std::vector<std::vector<std::size_t>>) + vecs.capacity()*sizeof(std::vector<std::size_t>);
        for(auto vec : vecs)
            total += memory(vec);
        return total;
    }    
   
    static void read(std::istream &in, std::vector<std::size_t> &obj)
    {
        obj.clear();
        std::size_t vsize = 0;
        in.read((char*)&vsize, sizeof(std::size_t));
        if(vsize == 0)
            return;
        obj.resize(vsize);
        in.read((char*)&obj[0], vsize*sizeof(std::size_t));
    }
    
    static void write(std::ostream &out, const std::vector<std::size_t> &obj)
    {
        std::size_t vsize = obj.size();
        out.write((char*)&vsize, sizeof(std::size_t));
        if(vsize==0)
            return;
        out.write((char*)&obj[0], sizeof(std::size_t)*vsize);
    }


    void reserve(std::size_t num_items)
    {
        m_list.reserve(2*num_items+1);
    }
};

#endif

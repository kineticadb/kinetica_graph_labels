#ifndef __GRAPHLABELCONTAINER_H__
#define __GRAPHLABELCONTAINER_H__

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "SingleDLS.h"
#include <fstream>
#include <sstream>
/*!
///////////////////////////////////////////////////////////////////////////////////////////////////
/// This class is the main label container that can return associations of graph entities from/to labels
/// A map is used to keep track of any tuple combination of label index sets that are attached to the entities
/// E.g.:
/// If node 1 is attached to labels 1,2 then a pair index for {1,2} is created
/// If node 1 is added another label, say, 3, then the previous pair index is recyled
/// (unless there exists another node associated with these two labels)
/// and a new set {1,2,3} is created with another unique pair_index --> See m_labels2index
///
/// Index2Labels is the reverse of the map used above; i.e., for each pair_index it shows the set of labels it is created for
/// Label to indexes is updated so that we know which pair indexes a label is  part of.
/// E.g.:
/// If Label 1 is part of {1,2,3} tuple combination, which has the pair index of 10 and {1,4,5,6} is another tuple combo of index 12
/// Then, Label 1 --> {10,12} since it appears in pairs 10 and 12. This is created from the m_labels2index and automatically updated
///
/// A singleDLS container is used to keep the associations between the unique pair indexes and associated entities
/// See SingleDLS.h for details, but the unique index found from the maps of m_labels2index is used to uniquely associate it
/// with the graph entity ids.
///
/// Recycling is a FIFO vector - where when a pair index is removed then its id is recycled for the new possible labels combo set
/// m_maxid is incremented when the recycle que is empty as an id factory.
///
/// Author: Karamete - Aug, 2023
/// //////////////////////////////////////////////////////////////////////////////////////////////
*/
class GraphLabelContainer {
protected:
    
    std::map<std::vector<std::size_t>, std::size_t > m_labels2index; //- map btw labels set to a unique pair index
    std::vector<std::vector<std::size_t>>            m_index2labels; //- inverse of above
    std::vector<std::vector<std::size_t>>            m_label2indexes;//- book-keeping which indexes each label appears
    
    SingleDLS m_dls; //- associations between pair indexes and graph entities
    
    // Deque for recycling dead indexes
    std::vector<std::size_t>            m_recycle; //- A FIFO que for recycling the pair indexes
    std::size_t                         m_maxid;   //- Id factory to get fresh new index id when que is empty

 public:
        
    //! C'tor
    GraphLabelContainer()
    {
        clear();
    }

    //! Clears all
    void clear()
    {
        m_labels2index.clear();
        m_index2labels.clear();
        m_label2indexes.clear();            
        m_dls.clear();
        m_recycle.clear();
        m_maxid = 0;
    }

    ///! returns the number of items
    std::size_t size() const
    {
        return m_dls.size_items();
    }

    //! returns the index that might be used for the next pair (labels set)
    std::size_t next_index() const
    {
        //return m_labels2index.size()+1;
        //return m_maxid+1;
        return m_recycle.empty() ? m_maxid+1 : m_recycle.back();
    }
    
    //! Pops the index from the FIFO que or from the id factory
    void pop_index()
    {
        if(!m_recycle.empty())
            m_recycle.resize(m_recycle.size()-1);
        else
            m_maxid++;
    }
    
    //! Recycles the old_index
    bool recycle(std::size_t old_index)
    {
        if(m_dls.is_deleted_label(old_index))
        {
            // delete from the map
            if(old_index < m_index2labels.size())
            {                
                const std::vector<std::size_t> &labels = m_index2labels[old_index];
                m_labels2index.erase(labels);
                for(auto label : labels)
                {
                    if(label < m_label2indexes.size())
                    {
                        auto pr = std::find(m_label2indexes[label].begin(), m_label2indexes[label].end(), old_index);
                        if(pr != m_label2indexes[label].end())
                            m_label2indexes[label].erase(pr);                       
                    }                        
                }                
            }
            m_index2labels[old_index].clear();
            if(old_index+1 == m_index2labels.size())
                m_index2labels.resize(old_index);
            if(m_dls.size_labels() == old_index)
                m_dls.resize_labels(old_index);
            m_recycle.push_back(old_index);
            return true;
        }
        return false;
    }

    //! Adds a set of labels to the framework returns the pair index corresponding to this set
    std::size_t  addLabel(const std::vector<std::size_t> &newpair)
    {
        auto it = m_labels2index.insert({newpair, next_index()});
        std::size_t pair_index = it.first->second;
        if(it.second)
        {            
            pop_index();
            for(auto label_index : newpair)
            {
                if(label_index >= m_label2indexes.size())
                    m_label2indexes.resize(label_index+1);
                  m_label2indexes[label_index].insert(std::upper_bound(m_label2indexes[label_index].begin(),  
                                                                       m_label2indexes[label_index].end(),
                                                                       pair_index),pair_index);
                //-m_label2indexes[label_index].push_back(pair_index);
            }
            if(pair_index >= m_index2labels.size())
            {
                m_index2labels.resize(pair_index + 1);               
            }
             m_index2labels[pair_index] = newpair;
        }
        return pair_index;
    }
    
    //! Adds the label index with a unique pair index and associates the new index (if new) with the entity gv
    std::size_t addLabel(std::size_t gv, std::size_t label_index)
    {        
        std::size_t pair_index = m_dls.get_label(gv);        
        std::size_t old_index = pair_index;
        if(!pair_index)
            pair_index = addLabel(std::vector<std::size_t>(1,label_index));
        else
        {    
            
            const std::vector<std::size_t> &existing = m_index2labels[pair_index];
            if(!std::binary_search(existing.begin(), existing.end(), label_index))
            {                       
                std::vector<std::size_t> newpair(existing);            
                newpair.insert(std::upper_bound(newpair.begin(), newpair.end(), label_index),label_index);               
                pair_index = addLabel(newpair);
            }                          
        }
       
        m_dls.insert(gv,pair_index);
        
        if(old_index && old_index != pair_index)
        {
            recycle(old_index);
        } 
        return pair_index;
    }
    
    //! IMPORTANT: The label indexes have to be sorted - uses above method for each label in labels
    std::size_t addLabel(std::size_t gv, const std::vector<std::size_t> &labels)
    {        
        std::size_t pair_index = m_dls.get_label(gv);        
        std::size_t old_index = pair_index;
        if(!pair_index)
            pair_index = addLabel(labels);
        else
        {                
            const std::vector<std::size_t> &existing = m_index2labels[pair_index];
            bool nonexists = false;
            for(auto label : labels)
            {
                if(std::find(existing.begin(), existing.end(), label) == existing.end())
                {
                    nonexists = true;
                    break;
                }                    
            }
            if(nonexists)
            {                
                std::vector<std::size_t> newpair(existing);
                for(auto label : labels)
                {
                    newpair.insert(std::upper_bound(newpair.begin(), newpair.end(), label),label);
                }                             
                pair_index = addLabel(newpair);
            }                          
        }
        
        m_dls.insert(gv,pair_index);
        
        if(old_index && old_index != pair_index)
            recycle(old_index);
        
        return pair_index;
    }

    //! Removes the label index from an entity gv
    std::size_t delLabel(std::size_t gv, std::size_t label_index)
    {       
        // get the node's pair index.
        std::size_t pair_index = m_dls.get_label(gv);
         m_dls.del_item(gv);
        if(!pair_index)
        {
            std::cout << "node " << gv <<  " does not have any label " << std::endl;
            return 0;
        }
        std::size_t old_index = pair_index;                    
        const std::vector<std::size_t> &existing = m_index2labels[pair_index];
        std::vector<std::size_t> newpair(existing);
        auto it = std::find(newpair.begin(), newpair.end(), label_index);
        if(it != newpair.end())
        {    
            newpair.erase(it);            
            pair_index = (newpair.empty()) ? 0 : addLabel(newpair);
        }  
        else
        {
            std::cout << "node " << gv <<  " does not have label " << label_index << std::endl;
            return 0;
        }
        
        // deletes existing
        if(pair_index && old_index != pair_index)
        {           
            m_dls.insert(gv,pair_index);
        }
        
        if(old_index && old_index != pair_index)
        {
            recycle(old_index);           
        } 
        return pair_index;
    }
    
    //! Removes the label_index from all entities
    void delLabel(std::size_t label_index)
    {      
        std::vector<std::size_t> ents;
        if(getEntities(label_index, ents))
        {            
            for(auto ent : ents)
            {             
                delLabel(ent,label_index);              
            }
        }
    }
    
    //! Removes the entity from being associated to the labels; TO_DO: revise pair indexes
    void removeEntityFromLabels(std::size_t gv)
    {
        m_dls.del_item(gv);
    }

    bool hasLabel(std::size_t gv) const
    {
        return (gv > m_dls.size_items()) ? false : m_dls.get_label(gv);
    }

    bool hasLabel(std::size_t gv, std::size_t label_index) const
    {
        std::size_t pair_index = m_dls.get_label(gv);
        if(!pair_index)
            return false;
        return std::binary_search(m_index2labels[pair_index].begin(), m_index2labels[pair_index].end(), label_index);
    }

    //! Prints all
    void print(std::ostream &out = std::cout) const
    {
        out << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        out << "Max id=" << m_maxid << std::endl;
        out << "Recyled Ids: " << std::endl;
        for(auto id : m_recycle)
            out << id << std::endl;
        out << "Labels to Index" << std::endl;
        for(auto it = m_labels2index.begin(); it != m_labels2index.end(); ++it  )
        {
            const std::vector<std::size_t> &labels = it->first;
            if(labels.size() == 0)
                continue;
            for(auto label : labels)
            {
                out << label << " ";
            }
            out <<  ":: " << it->second << std::endl;
        }
        out << "Index to Labels" << std::endl;
        for(std::size_t i = 0; i < m_index2labels.size(); ++i)
        {
            const std::vector<std::size_t> &labels = m_index2labels[i];
            out << i << ":: ";
            for(auto label : labels)
            {
                out << label << " ";
            }
            out << std::endl;
        }
        out << "Label to Indexes: " << std::endl;
        for(std::size_t i = 0; i < m_label2indexes.size(); ++i)
        {
            const std::vector<std::size_t> &indexes = m_label2indexes[i];
            out << i << "::";
            for(auto index : indexes)
                out << index << " ";
            out << std::endl;
        }
        m_dls.print(out);       
        for(std::size_t i = 1; i < m_label2indexes.size(); ++i)
        {
            print(i,out);
        }
        out << "Node to Labels" << std::endl;
        std::vector<std::size_t> labels;
        for(std::size_t gv = 1; gv <= m_dls.size_items(); ++gv)
        {
            getLabels(gv, labels);
            out << gv << "::";
            for(auto label : labels)
            {
                out << label << " ";
            }
            out << std::endl;
        }
        out << "-------------------------------------------------------------" << std::endl;
    }

    //! Prints entities associated with a label index
    void print(std::size_t label_index, std::ostream &out= std::cout) const
    {
        out << "Nodes of label " << label_index << ":";
        std::vector<std::size_t> ents;
        getEntities(label_index, ents);
        for(auto ent : ents)
            out << ent << " ";
        out << std::endl;        
    }
    
    //! Returns all entities associated with a label index
    std::size_t getEntities(std::size_t label_index, std::vector<std::size_t> &ents, bool clear = true) const
    {
        if(clear)
            ents.clear();
        bool dontclear = false;
        if(label_index < m_label2indexes.size())
        {
            const std::vector<std::size_t> &indexes = m_label2indexes[label_index];
                      
            for(auto index : indexes)
            {                
                m_dls.get(index, ents, dontclear);                
            }
        }
        return ents.size();
    }
    
    //! Returns labels associated with an entity
    std::size_t getLabels(std::size_t gv, std::vector<std::size_t> &labels) const
    {
        labels.clear();
        // get the pair index of the gv
        std::size_t pair_index = m_dls.get_label(gv);
        
        // now get the labels of the pair index
        if(pair_index && pair_index < m_index2labels.size())
        {
            labels = m_index2labels[pair_index];
        }
        return labels.size();        
    }
    
    //! Serialized write to a binary output stream
    void write(std::ostream &out) const
    {
        std::size_t vsize = m_index2labels.size();
        out.write((char*)&vsize, sizeof(std::size_t));
        if(vsize==0)
            return;
        for(std::size_t index = 0; index < m_index2labels.size(); ++index)
        {
            vsize = m_index2labels[index].size();
            out.write((char*)&vsize,sizeof(std::size_t));
            if(vsize ==  0)
                continue;            
            out.write((char*)&m_index2labels[index][0], sizeof(std::size_t)*vsize);
        }
        m_dls.write(out);
        out.write((char*)&m_maxid, sizeof(std::size_t));
        SingleDLS::write(out,m_recycle);
    }
      
    //! Serialized read from a binary input stream
    void read(std::istream &in)
    {
        clear();
        m_index2labels.clear();
        std::size_t vsize = 0;
        in.read((char*)&vsize, sizeof(std::size_t));
        if(vsize == 0)
            return;        
        m_index2labels.resize(vsize);
        for(std::size_t i = 0; i < m_index2labels.size(); ++i)
        {             
            in.read((char*)&vsize, sizeof(std::size_t));
            if(vsize == 0)
                continue;
            m_index2labels[i].resize(vsize);         
            in.read((char*)&m_index2labels[i][0], vsize*sizeof(std::size_t));
        }        
        // populate others
        for(std::size_t index = 1; index < m_index2labels.size(); ++index)
        {            
            m_labels2index.insert({m_index2labels[index],index});            
            for(auto label : m_index2labels[index])
            {                                
                if(label >= m_label2indexes.size())
                    m_label2indexes.resize(label+1);
                m_label2indexes[label].push_back(index);
            }            
        }  
        // read dls
        m_dls.read(in);
        in.read((char*)&m_maxid, sizeof(std::size_t));
        SingleDLS::read(in,m_recycle);
    }
    
    //! Returns the memory occupied (in bytes)
    std::size_t memory() const
    {
        // index2labels
        std::size_t total = SingleDLS::memory(m_index2labels);
        // labels2index; assume 1.5 times more than above
        total *= 1.5;
        // label2indexes
        total += SingleDLS::memory(m_label2indexes);
        total += m_dls.memory();  
        total += SingleDLS::memory(m_recycle);        
        return total + sizeof(std::size_t);                        
    }

    void reserve(std::size_t num_entities)
    {
        m_dls.reserve(num_entities);
    }

    //! Returns the entities associated with exactly this set of labels; to be run in the case of and_labels
    std::size_t getEntities(const std::vector<std::size_t> &labels, std::vector<std::size_t> &ents) const
    {
        ents.clear();
        // labels should be sorted
        auto it = m_labels2index.find(labels);
        if(it != m_labels2index.end())
        {
            std::size_t index = it->second;
            m_dls.get(index, ents);
        }
        return ents.size();
    }

    bool isAssociated(const std::vector<std::size_t> &labels) const
    {
        auto it = m_labels2index.find(labels);
        return (it != m_labels2index.end()) ? !m_dls.is_deleted_label(it->second) : false;
    }

};


#endif

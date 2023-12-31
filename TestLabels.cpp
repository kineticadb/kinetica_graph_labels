#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "GraphLabelContainer.h"
/*!
////////////////////////////////////////////////////////////////////////////////////////////
/// One graph entity to many labels - many Labels to any graph entities 
/// fixed size light weight scalable Graph Label Container data-structure
/// Main program that has a rudimentary testing framework and runs a list of unit-tests
/// Author: Bilge Kaan Karamete (Oct 9, 2023) 
/// Verstion: 1.0
/// Note: This is the labelling framework incorporated to the Kinetica-Graph distributed 
/// hybrid graph DB that works in tandem with Kinetica's relational DB with parallel
/// OLAP expression support...
////////////////////////////////////////////////////////////////////////////////////////////
*/ 
int test_mimic_graph(std::ostream &out)
{
    // 1. Assume Dict encoded indexes for unique label strings on graph side
    std::map<std::string,std::size_t> vlabels =
    { {"self",1}, {"ceo",2}, {"principal",3}, {"expert",4}, {"master",5}, {"vp",6} };
    
    // 2. Assume the node names as dict encoded string to graph node indexes on graph side
    std::map<std::string,std::size_t> nodes = 
    { {"kaan",1}, {"eli", 2}, {"shouvik", 3}, {"nima",4}, {"steve",5}, {"john",6} };
    
    // 2.1. Create inverse of above aps for output purposes
    std::vector<std::string> index2label;
    std::vector<std::string> index2vname;
    
    for(auto it = vlabels.begin(); it != vlabels.end(); ++it)
    {
        std::size_t index = it->second; 
        if(index2label.size() <= index)
            index2label.resize(index+1);          
        index2label[index] = it->first;
    }
    for(auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        std::size_t index = it->second; 
        if(index2vname.size() <= index)
            index2vname.resize(index+1);          
        index2vname[index] = it->first;
    }
    
    // 3. Associate node with the label
    GraphLabelContainer glc;
    
    glc.addLabel(nodes["shouvik"],vlabels["expert"]);
    glc.addLabel(nodes["shouvik"],vlabels["master"]);
    glc.addLabel(nodes["kaan"],vlabels["expert"]);
    glc.addLabel(nodes["kaan"],vlabels["self"]);
    glc.addLabel(nodes["eli"],vlabels["vp"]);
    glc.addLabel(nodes["eli"],vlabels["expert"]);
    glc.addLabel(nodes["nima"],vlabels["ceo"]);
    glc.addLabel(nodes["nima"],vlabels["expert"]);
    
    glc.addLabel(nodes["steve"],vlabels["principal"]);
    
    glc.addLabel(nodes["john"],vlabels["expert"]);
    glc.addLabel(nodes["john"],vlabels["vp"]);
    
    glc.print(out);
    
    // 4. List label associations for each node 
    for(auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        std::vector<std::size_t> vlabels;
        glc.getLabels(it->second,vlabels);
        out << it->first << ":";
        for(auto label_index : vlabels)
        {
            out << index2label[label_index] << " ";
        }
        out << std::endl;
    }
    
    // 5. Find the nodes that have the 'expert' label
    std::size_t label_expert = vlabels["expert"];
    std::vector<std::size_t> ents;
    std::vector<std::string> stents;
    out << "Label 'expert' : ";
    glc.getEntities(label_expert, ents);
    for(auto ent : ents)
    {
        out << index2vname[ent] << " ";
        stents.push_back(index2vname[ent]);
    }
    out << std::endl;
    std::sort(stents.begin(), stents.end());
    std::vector<std::string> trusted = {"eli", "john", "kaan", "nima", "shouvik"};
    return (stents == trusted);
}


int test_A241_label_container(std::ostream &out)
{
    GraphLabelContainer c;
    std::vector<std::size_t> gvlabels = { 1,1, 1,2, 2,2, 2,1, 3,1, 3,3, 4,1, 4,4, 4,2, 4,3, 1,4 }; // gv, label
    
    for(std::size_t i = 0; i < gvlabels.size(); i = i + 2)
    {
        out << "Adding : (" << gvlabels[i] << "," << gvlabels[i+1] << "):" << std::endl;
        c.addLabel(gvlabels[i], gvlabels[i+1]);
        c.print(out);
    }
    out << "Memory [kB] = " << c.memory()/1000 << std::endl;
    c.print(out);
    std::ostringstream ost;
    c.print(ost);
    std::string s1 = ost.str();
    
    out << "Writing ..." << std::endl;
    std::ofstream f("test_labels.out");
    c.write(f);
    f.close();
    out << "Reading ..." << std::endl;
    c.clear();
    std::ifstream f1("test_labels.out", std::ios::binary);
    c.read(f1);
    
    out << "Memory [kB] = " << c.memory()/1000 << std::endl;
    f.close();
    out << "***********************************************" << std::endl;
    c.print(out);
    
    std::ostringstream ost2;
    ost2.clear();
    c.print(ost2);
    std::string s2 = ost2.str();
    
    if(s1 != s2)
    {
        out << "s1%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        out << s1 << std::endl;
        out << "s2%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
        out << s2 << std::endl;
        out << "Failed ..." << std::endl;
        return 0;
    }
    out << "Passed ..." << std::endl;
    
    c.clear();
    std::vector<std::vector<std::size_t>> gvlabels2 = { {1,2}, {2,1}, {3,1}, {1,4,2,3} }; // labels per index gv
    for(std::size_t i = 0; i < gvlabels2.size(); ++i)
    {
        for(std::size_t j = 0; j < gvlabels2[i].size(); ++j)
        {
            out << "Adding : (" << i+1 << "," << gvlabels2[i][j] << "):" << std::endl;
        }
        
        std::sort(gvlabels2[i].begin(), gvlabels2[i].end());
        c.addLabel(i+1, gvlabels2[i]);
    }
    bool haslabel[2] = { c.hasLabel(3,4), c.hasLabel(3,3) };
    bool trusted2[2] = { false, true };
    for(std::size_t i = 0; i < 2; ++i)
    {
        if(trusted2[i] != haslabel[i])
        {
            out << "has label failed for " << i << std::endl;
            return 0;
        }
    }
    out << "Memory [kB] = " << c.memory()/1000 << std::endl;
    c.print(out);
    
    //c.delLabel(4,2);
    c.delLabel(4,2);
    c.print(out);
    c.delLabel(2);
    out << "after delete 2" << std::endl;
    c.print(out);
    c.delLabel(4);
    out << "after delete 4" << std::endl;
    c.print(out);
    c.delLabel(1);
    out << "after delete 1" << std::endl;
    c.print(out);
    c.delLabel(3);
    out << "after delete 3" << std::endl;
    c.print(out);
    if(c.hasLabel(4,2))
    {
        out << "has label failed" << std::endl;
        return 0;
    }
    return 1;
}

typedef std::map<std::string, int (*)(std::ostream&)> TestMapType;
TestMapType tmap;

#define REGISTER(T) \
    tmap[#T] = &T;


int run_tests(int c, char** v)
{
    std::ostream *m_out = &std::cout;
    int tnum = 0;
    bool list = false;
    for (int i = 1; i < c; ++i)
    {
        std::string st = v[i];
        if(st == "-v")
        {
            std::cout << "running in verbose mode..." << std::endl;
        }
        if(st == "-q")
        {
            m_out = new std::ostream(NULL);
            std::cout << "running in quite mode..." << std::endl;
        }
        if(st == "-t")
        {
            if(c < i+1)
            {
                std::cout << "See usage.." << std::endl;
                return 0;
            }
            tnum = atoi(v[i+1]);
        }
        if(st == "-l")
        {
            list = true;
        }	      
    }
    
    
    int cnt = 0;
    int nf = 0;
    for(auto it = tmap.begin(); it != tmap.end(); ++it)
    {
        
        ++cnt;
        if(list)
        {
            std::cout << "Test " << cnt << ":" << it->first << std::endl;
            continue;
        }	      
        
        if(tnum && cnt != tnum)
            continue;
        std::cout << "Testing " << cnt << ":" << it->first;
        int res = 1;
        try
        {
            res = (it->second)(*m_out);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            res = 0;
        }
        if(!res)
        {
            std::cout << " --> failed..." << std::endl;
            ++nf;
        }
        else
            std::cout << " --> passed..." << std::endl;
    }
    
    if(!tnum)
    {
        if(!list)
            std::cout << "Tests failed = " << nf << " out of " << cnt << "." << std::endl;
        else
            std::cout << "Tests available : " << cnt << "." << std::endl;
    }        
    return 1;
}

int main(int c, char** v)
{    
    REGISTER(test_A241_label_container)
    REGISTER(test_mimic_graph)
    
    if(c == 1)
    {
        std::cout <<"########################################################" << std::endl;
        std::cout << "TESTS for labelling containers" << std::endl;
        std::cout << "Usage: TestLabels.x <option>" << std::endl;
        std::cout << "Options: " << std::endl;
        std::cout << "     -l : lists all available tests..." << std::endl;         
        std::cout << "     -q : Runs all tests in quiet mode..." << std::endl;         
        std::cout << "     -t [0..n] : tests the nth test..." << std::endl;         
        std::cout << "     -v : Runs all tests in verbose mode..." << std::endl;
        std::cout <<"########################################################" << std::endl;
        return 1;
    }
    
    return run_tests(c,v);
}

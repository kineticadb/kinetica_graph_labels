#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "GraphLabelContainer.h"

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


int main(int c, char** v)
{
   return test_A241_label_container(std::cout);
}

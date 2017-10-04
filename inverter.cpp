#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <cstdlib>
#include <vector>


using namespace std;
enum {
    INV_BASE = 0,
    INV_INPUT_FILE_UNREADABLE,
    INV_INPUT_FILE_EMPTY,
    
}INV_erros;


/* Open filename
    * read every line to get the files names
    * go through each file
    * parse the word in every file into inverted_index
    * return 0 if it works, otherwise code error
    */
int build_index(const std::string& filename, map<string, set<int>>& inverted_index)
{
   
    int result = INV_BASE;
    std::ifstream input_file(filename);
    if (!input_file)
    {
        std::cerr <<"The input file is not openable, maybe not available or path issue"<<'\n';
        result = INV_INPUT_FILE_UNREADABLE;
    }
    std::vector<string> database_filenames;
    
    // read input file
    while(input_file)
    {
        std::string file ;
        getline(input_file, file);
        if (!file.empty())
        {
            database_filenames.push_back(file);
        }
    }
//     cout <<"size of database "<< database_filenames.size() << '\n';
    if (database_filenames.size() == 0 )
    {
        std::cerr <<"Empty input file" <<'\n';
        result = INV_INPUT_FILE_EMPTY;
    }
    input_file.close();
    // iterate through the files 
    int file_index = 0;
    vector<string>:: const_iterator it;
    it = database_filenames.begin();
    while (it != database_filenames.end())
    {
        ifstream input(*it);
        if (!input)
        {
            std::cerr <<"The input file is not openable, maybe not available or path issue"<<'\n';
        }
        else
        {
            while(input)
            {
                
                string word;
                input >> std::skipws>> word;
                if (!word.empty()){
                    
                    map<string, set<int>>:: iterator it_tmp;
                    set<int> temp;
                    temp.insert(file_index);
                    it_tmp = inverted_index.find(word);
                    if (it_tmp == inverted_index.end())
                    {
                        // words is seen for the first time
                        inverted_index.insert(make_pair(word, temp));
                    }
                    else 
                    {
                        //update set to add build_index
                        (it_tmp->second).insert(file_index);
                    }
                }
            }
         }
         file_index +=1;
         ++it;
    }
           
    return result;
    
}

int main()
{
    
    map<string, set<int>> index;
    string input ="inputs";
    int test = build_index(input, index);
    cout << "size of index table "<<index.size() << '\n';
    map<string, set<int>>::const_iterator it; // declare an iterator
    it = index.begin(); // assign it to the start of the vector
    while (it != index.end()) // while it hasn't reach the end
    {
        cout << (it->first) << ": ";
        set<int>::const_iterator set_it;
        for (set_it = it->second.begin(); set_it != it->second.end(); ++set_it)
        {
            cout << *set_it << " ";
        }
        cout << '\n';
        ++it; // and iterate to the next element
    }
 
    return 0;
}
    
    
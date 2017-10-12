#include <iostream>
#include <fstream>
#include <string.h>
#include <map>
#include <set>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <sstream>
#include <iterator>

using namespace std;
enum {
    INV_OK,
    INV_BASE = 100,
    INV_INPUT_FILE_UNREADABLE,
    INV_INPUT_FILE_EMPTY,
    INV_QUERY_STRING_EMPTY,
    INV_QUERY_NOT_PRESENT_TABLE,
    
}INV_erros;


static const int debug = 1;
/* Open filename
    * read every line to get the files names
    * go through each file
    * parse the word in every file into inverted_index
    * return 0 if it works, otherwise code error
    */
int build_index(const std::string& filename, map<string, set<int>>& inverted_index, vector<string> &database_filenames, const set<string>& stop_list)
{
   
    int r_status = INV_OK;
    std::ifstream input_file(filename);
    if (!input_file)
    {
        std::cerr <<"The input file is not openable, maybe not available or path issue"<<'\n';
        r_status = INV_INPUT_FILE_UNREADABLE;
        return r_status;
    }
//     std::vector<string> database_filenames;
    
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
    if (database_filenames.size() == 0 )
    {
        std::cerr <<"Empty input file or file does not exist" <<'\n';
        r_status = INV_INPUT_FILE_EMPTY;
        return r_status;
    }
    input_file.close();
    // iterate through the files 
    int file_index = 0;
    vector<string>:: const_iterator it;
    it = database_filenames.begin();
    while (it != database_filenames.end())
    {
        ifstream input(*it);
        // maybe trim also the file name to avoid having \n \t in names. with \n \t the file won't be parsed
        if (!input)
        {
            std::cerr <<"The file "<<*it<<"  is not openable, maybe not available or path issue"<<'\n';
        }
        else
        {
            while(input)
            {
                
                string word;
                input >> std::skipws>> word;
                // parse the string to remove , . /t /n and special characters and punctuations
                word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::isspace(c);}), word.end());
                word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::ispunct(c);}), word.end());
                word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::isdigit(c);}), word.end());
                // put it to lowercase
                transform(word.begin(), word.end(), word.begin(), ::tolower);
                // word not empty or is not present in stop_list
                if (!word.empty() && (stop_list.find(word) == stop_list.end()) )
                {
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
                        // it might be faster to insert just 0 and 1, but if we insert 0 and 1 we will need 
                        (it_tmp->second).insert(file_index);
                    }
                }
            }
            file_index +=1;
         }
         
         ++it;
    }
           
    return r_status;
    
}

/*
 * build stop word list
 * these are recurrent words and are not useful for searches
 */

int build_stop_index(const std::string& filename, set<string>& stop_list)
{
    int r_status = INV_OK;
    std::ifstream input_file(filename);
    if (!input_file)
    {
        cout <<"The input file is not openable, maybe not available or path issue"<<'\n';
        r_status = INV_INPUT_FILE_UNREADABLE;
    }
    while(input_file)
    {
        
        string word;
 
        input_file >> std::skipws>> word;
//         cout << word <<'\n';
        // parse the string to remove , . /t /n and special characters and punctuations
        word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::isspace(c);}), word.end());
        word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::ispunct(c);}), word.end());
        word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c){return std::isdigit(c);}), word.end());
        // put it to lowercase
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        stop_list.insert(word);
    }
    
    return r_status;
}

/*
 * Normalise input
 * Remove punctuations
 * Remove also stop word
 */

int normalize(string &input, vector<string> &query, const set<string>& stop_list)
{
    int r_status = INV_OK;
    vector<string> tmp;
    // remove punctuations and digits
    input.erase(std::remove_if(input.begin(), input.end(), [](unsigned char c){return std::ispunct(c);}), input.end());
    input.erase(std::remove_if(input.begin(), input.end(), [](unsigned char c){return std::isdigit(c);}), input.end());
    transform(input.begin(), input.end(), input.begin(), ::tolower);
    istringstream iss(input);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), std::back_inserter(tmp) );
    
    // remove and stop word
    for (int i = 0 ; i < tmp.size(); ++i)
    {
        if ( stop_list.find(tmp[i]) == stop_list.end()) 
        {
            // add it
            query.push_back(tmp[i]);
        }
    }
    return r_status;
}

/*
 * Implement elastic search
 * here we don't look at the order in which the query is formulated as our index table is
 * ordered alphapetically
 * an optimization could be to get the document that has every string of query with the same order in query as in the document
 * but in order to do so, we need a different way of creating the index table
 */
int search(const map<string, set<int>> &index, const vector<string> &database_filenames, const vector<string> &query)
{

    // TODO normalize input to without comma, to lowercase and without punctuations
    // save how many times a word appear in a document
    // add stop word
    int r_status = INV_OK;
    bool iteration = true;
   
    if (query.size() == 0 || index.size() == 0)
    {
        cout <<"Query string is empty or index is empty" <<'\n';
        return INV_QUERY_STRING_EMPTY;
    }
    vector<string>::const_iterator it_string_in_query; // declare an iterator
    map<string, set<int>>::const_iterator it_index_table; // declare an iterator
    it_string_in_query = query.begin();
    
    /*
     * get documents that contains all strings in query
     * Intersection of differents sets
     */
    
    int iter_index = 0;
    std::set<int> cur_inter;
    std::set<int> pred_inter;
    while (iteration)
    {
        // find query 
        it_index_table = index.find(*it_string_in_query);
        if (it_index_table != index.end())
        {
            // query exist
            cur_inter = (it_index_table->second);
            // if first iteration pred and curr are same
            if (iter_index == 0 )
            {
                pred_inter = cur_inter;
            }
            // find Intersection
            std::set<int> tmp;
            set_intersection(cur_inter.begin(),cur_inter.end(),pred_inter.begin(),pred_inter.end(),
                             std::inserter(tmp,tmp.begin()));
            if ( tmp.size() == 0)
            {
                iteration = false;
            }
            else {
                // continue
                pred_inter = tmp;
            }
            ++it_string_in_query;
            ++iter_index;
            if (it_string_in_query != query.end())
            {
                iteration = false;
            }
        }
        // string is not present
        else 
        {
            iteration = false;
        }   
        
    }
    // final set in pred_inter
    if (pred_inter.size() == 0 )
    {
        cout <<"String is not present in index " <<'\n';
        r_status = INV_QUERY_NOT_PRESENT_TABLE;
    }
    else 
    {
        set<int>::const_iterator it;
        cout <<"Result of search :" <<'\n';
        for (it=pred_inter.begin(); it != pred_inter.end(); ++it)
        {
            cout << database_filenames[*it] <<'\n';
        }
    }
    return r_status;
}   
        
void print_database(const map<string, set<int>> &index, const set<string>& stop_list)
{
    map<string, set<int>>::const_iterator it; // declare an iterator
    set<string>::const_iterator it_set; // declare an iterator
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
    cout << "size of index table "<<index.size() << '\n';
    for (it_set = stop_list.begin(); it_set != stop_list.end(); ++it_set)
    {
            cout << *it_set << "\n";
    }
}
int main(int argc, char **argv)
{
    string input ;
    string table_filename = "inputs";
    string stop_filename = "stopword";
    
    map<string, set<int>> index;
    vector<string> database_filenames;
    set<string> stop_list;
    vector<string> query;
    
    int r_status = build_stop_index(stop_filename, stop_list);
    if(r_status != INV_OK)
    {
        cout << "status " << r_status <<'\n';
        cout<<"stop word create index failed"<<'\n';
        return 0;
    }
    r_status = build_index(table_filename, index, database_filenames, stop_list);
    if(r_status != INV_OK)
    {
        cout <<" valeur de return  " <<r_status <<'\n';
        cout<<"Inverted index creation failed"<<'\n';
        return 0;
    }
    cout<<"stop list size  "<<stop_list.size() <<'\n';
    cout<<"database list size  "<<index.size() <<'\n';
//     if (debug) print_database(index, stop_list);

    // Test
    
    while(1)
    {
        cout <<"Write your search and then press enter " << '\n';
        getline(cin , input);
        normalize(input, query, stop_list);
        
//         for (int i = 0 ; i < query.size(); ++i)
//         {
//             cout << query[i] <<'\n';
//         }
        
        r_status = search(index, database_filenames, query);
        if(r_status != INV_OK)
        {
            cout<<"Search failed"<<'\n';
        }
//         clear previous search
        query.clear();
    }
    
    return 0;
}
    
    
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>


// Format checker just assumes you have Alarm.bif and Solved_Alarm.bif (your file) in current directory
using namespace std;

// Our graph consists of a list of nodes where each node is represented as follows:
class Graph_Node{

private:
	string Node_Name;  // Variable name 
	vector<int> Children; // Children of a particular node - these are index of nodes in graph.
	vector<string> Parents; // Parents of a particular node- note these are names of parents
	int nvalues;  // Number of categories a variable represented by this node can take
	vector<string> values; // Categories of possible values
	vector<float> indep_prob; //Individual Probabilities of values
	vector<float> CPT; // conditional probability table as a 1-d array . Look for BIF format to understand its meaning

public:
	// Constructor- a node is initialised with its name and its categories
    Graph_Node(string name,int n,vector<string> vals){
		Node_Name=name;
		nvalues=n;
		values=vals;

	}

	string get_name(){
		return Node_Name;
	}

	vector<int> get_children(){
		return Children;
	}

	vector<string> get_Parents(){
		return Parents;
	}

	vector<float> get_CPT(){
		return CPT;
	}

	int get_nvalues(){
		return nvalues;
	}

	vector<string> get_values(){
		return values;
	}

	vector<float> get_indep(){
		return indep_prob;
	}

	void set_indep(vector<float> indep){
		indep_prob = indep;
	}

	void set_CPT(vector<float> new_CPT){
		CPT.clear();
		CPT=new_CPT;
	}

    void set_Parents(vector<string> Parent_Nodes){
        Parents.clear();
        Parents=Parent_Nodes;
    }

    // add another node in a graph as a child of this node
    int add_child(int new_child_index ){
        for(int i=0;i<Children.size();i++){
            if(Children[i]==new_child_index)
                return 0;
        }
        Children.push_back(new_child_index);
        return 1;
    }
};


// The whole network represted as a list of nodes
class Network{
	list <Graph_Node> Pres_Graph;
public:
	int addNode(Graph_Node node){
		Pres_Graph.push_back(node);
		return 0;
	}
        
	int netSize(){
		return Pres_Graph.size();
	}

    // get the index of node with a given name
    int get_index(string val_name){
        list<Graph_Node>::iterator listIt;
        int count=0;
        for(listIt=Pres_Graph.begin();listIt!=Pres_Graph.end();listIt++){
            if(listIt->get_name().compare(val_name)==0)
                return count;
            count++;
        }
        return -1;
    }

// get the node at nth index
    list<Graph_Node>::iterator get_nth_node(int n){
       list<Graph_Node>::iterator listIt;
        int count=0;
        for(listIt=Pres_Graph.begin();listIt!=Pres_Graph.end();listIt++){
            if(count==n)
                return listIt;
            count++;
        }
        return listIt; 
    }

    //get the iterator of a node with a given name
    list<Graph_Node>::iterator search_node(string val_name){
        list<Graph_Node>::iterator listIt;
        for(listIt=Pres_Graph.begin();listIt!=Pres_Graph.end();listIt++){
            if(listIt->get_name().compare(val_name)==0)
                return listIt;
        }
    
            cout<<"node not found\n";
        return listIt;
    }

	void independent_probability(vector<vector<string>> data){
		vector<map<string,float>> val_count;
		int net_size = this->netSize();
		vector<string>::iterator it;
		map<string,float>::iterator mit;
		for(int i=0;i<net_size;i++){
			map<string,float> values_count;
			vector<string> values = this->get_nth_node(i)->get_values();
			for(it=values.begin();it!=values.end();it++){
				values_count.insert({*it,0});
			}
			val_count.push_back(values_count);
		}

		for(int i=0;i<data.size();i++){
			for(int j=0;j<net_size;j++){
				string entry = data[i][j];
				if(entry!="\"?\""){
					val_count[j][entry]++;
				}
			}
		}

		for(int i=0;i<net_size;i++){
			float total=0.0;
			for(mit=val_count[i].begin();mit!=val_count[i].end();mit++){
				total += (*mit).second;
			}
			vector<float> temp;
			for(mit=val_count[i].begin();mit!=val_count[i].end();mit++){
				float prob = (*mit).second/total;
				temp.push_back(prob);
			}
			this->get_nth_node(i)->set_indep(temp);
		}

	}

	void counter(vector<string> col_names,int col_num,vector<float> &count,int low,int high,vector<string> row,float multiplier){
		int index = get_index(col_names[col_num]);
		vector<string> values = get_nth_node(index)->get_values();
		vector<string>::iterator it = find(values.begin(), values.end(), row[index]);
		int block_size = (high-low+1)/values.size();
		if(block_size==1){
			if(it!=values.end()){
				count[low+distance(values.begin(), it)] += multiplier*1;
				return;
			}else{
				vector<float> indep_prob = get_nth_node(index)->get_indep();
				for(int i=0;i<indep_prob.size();i++){
					count[low+i] += multiplier*indep_prob[i];
				}
				return;
			}
		}else{
			if(it!=values.end()){
				int d = distance(values.begin(), it);
				counter(col_names,col_num+1,count,low+(d*block_size),low+((d+1)*block_size),row,multiplier);
			}else{
				vector<float> indep_prob = get_nth_node(index)->get_indep();
				for(int i=0;i<indep_prob.size();i++){
					counter(col_names,col_num+1,count,low+(i*block_size),low+((i+1)*block_size),row,multiplier*indep_prob[i]);
				}
			}
		}

	}

	void conditional_probability(vector<vector<string>> data){
		int net_size = this->netSize();
		for(int i=1;i<2;i++){
			list<Graph_Node>::iterator Node = this->get_nth_node(i);
			vector<float> CPT_count;
			int nvalues = Node->get_nvalues();
			float total[nvalues];
			int cpt_size = Node->get_CPT().size();
			for(int j=0;j<nvalues;j++){
				total[j]=0;
			}
			for(int j=0;j<cpt_size;j++){
				CPT_count.push_back(0);
			}
			vector<string> columns = Node->get_Parents();
			columns.insert(columns.begin(),Node->get_name());
			for(int j=0;j<data.size();j++){
				counter(columns,0,CPT_count,0,cpt_size-1,data[j],1);
			}
			for(int j=0;j<cpt_size;j++){
				total[j%nvalues] += CPT_count[j];
			}
			for(int j=0;j<cpt_size;j++){
				CPT_count[j] = CPT_count[j]/total[j%nvalues];
			}
			Node->set_CPT(CPT_count);

		}
		

	}		

};

Network read_network(){
	Network Alarm;
	string line;
	int find=0;
  	ifstream myfile("alarm.bif"); 
  	string temp;
  	string name;
  	vector<string> values;
  	
    if (myfile.is_open()){
    	while (! myfile.eof() ){
    		stringstream ss;
      		getline (myfile,line);
      		      		
      		ss.str(line);
     		ss>>temp;
     		     		
     		if(temp.compare("variable")==0){
				ss>>name;
				getline (myfile,line);
				
				stringstream ss2;
				ss2.str(line);
				for(int i=0;i<4;i++){     					
					ss2>>temp;     					
					
				}
				values.clear();
				while(temp.compare("};")!=0){
					values.push_back(temp);     					
					ss2>>temp;
				}
				Graph_Node new_node(name,values.size(),values);
				int pos=Alarm.addNode(new_node);
     				
     		}else if(temp.compare("probability")==0){
                    
				ss>>temp;
				ss>>temp;
				
				list<Graph_Node>::iterator listIt;
				list<Graph_Node>::iterator listIt1;
				listIt=Alarm.search_node(temp);
				int index=Alarm.get_index(temp);
				ss>>temp;
				values.clear();
				while(temp.compare(")")!=0){
					listIt1=Alarm.search_node(temp);
					listIt1->add_child(index);
					values.push_back(temp);
					ss>>temp;

				}
				listIt->set_Parents(values);
				getline (myfile,line);
				stringstream ss2;                    
				ss2.str(line);
				ss2>> temp;                    
				ss2>> temp;                    
				vector<float> curr_CPT;
				string::size_type sz;
				while(temp.compare(";")!=0){                        
					curr_CPT.push_back(atof(temp.c_str()));     					
					ss2>>temp;                   
				}                    
				listIt->set_CPT(curr_CPT);
     		} 		    		
    	}    	
    	if(find==1)
    	myfile.close();
  	}  	
  	return Alarm;
}

vector<vector<string>> read_data(){
	vector<vector<string>> data;
    string line;
    ifstream datafile("records.dat"); 
  	string temp;
    while(getline(datafile,line)){
        istringstream ss(line);
		vector<string> row;
        while(ss){
			ss >> temp;
			row.push_back(temp);
		}
		data.push_back(row);
	}
	return data;
}




int main()
{
	Network Alarm;
	Alarm=read_network();
	vector<vector<string>> data = read_data();
	Alarm.independent_probability(data);
	vector<float> prob = Alarm.get_nth_node(1)->get_CPT();
	for(auto i:prob){
		cout << i << " ";
	}
	cout << "\n";
	Alarm.conditional_probability(data);
	vector<float> nprob = Alarm.get_nth_node(1)->get_CPT();
	for(auto i:nprob){
		cout << i << " ";
	}
	cout << "\n";

	
}






# include <iostream>
# include <vector>
# include <string>
# include <set>
# include <queue>
# include <utility>
# include <algorithm>
using namespace std;

//Class Definitions--------------------------------------------------------------------------

class lan{
	public:
		char id;
		int DP;
		vector<int> adj_bridges;
		vector<int> hosts;
		lan()
		{
			id = '\0';
			DP = -1;
		}
};

class ftable
{
	public:
		int host;
		lan fport;
};

class bridge{
	public:
		int id;
		int root;
		int root_distance;
		vector<char> adj_lans;
		vector<ftable> forwarding_table;
		pair<char,int> RP;
		bridge()
		{
			id = -1;
			root = id;
			RP = make_pair('\0',-1);
			root_distance = -1;
		}
};



class message{
	public:
		int root;
		int dist;
		bridge source;
		int destination;
		char lan;
		message()
		{
			root = -1;
			dist = -1;
			destination = -1;
			lan = '\0';
		}
};

struct messageComparer
{
    bool operator()(message const &j1, message const &j2)
    {
    	if(j1.destination < j2.destination) return true;
    	else return false;
        //compare j1 and j2 and return true or false
    }
};

class traces{
	public:
		int time;
		int bridge;
		char status;
		message m;
};

class data_packet{
public:
	int source;
	int destination;
	char prev;
	int bridge;
};

//--------------------------------------------------------------------------------------------

//Helper Functions-----------------------------------------------------------------------------

void print_bridge_network(vector<bridge> network)
{
	for(int i=0; i<network.size();i++)
	{
		cout<<network[i].id<<endl;
		for(int j=0; j<network[i].adj_lans.size();j++)
			cout<<network[i].adj_lans[j]<<" ";
		cout<<endl;
	}
}

void print_lan_network(vector<lan> network)
{
	for(int i=0; i<network.size();i++)
	{
		cout<<network[i].id<<endl;
		for(int j=0; j<network[i].adj_bridges.size();j++)
			cout<<network[i].adj_bridges[j]<<" ";
		cout<<endl;
		cout<<network[i].DP<<endl;
	}
}

typedef set<message,messageComparer> messageSet;

messageSet SendMessage(message m,vector<bridge> bridge_network, vector<lan> lan_network)
{
	messageSet messages;
	int root = m.root;
	int d = m.dist;
	bridge source = m.source;
	for(int i=0; i<bridge_network.size();i++)
		if(source.id == bridge_network[i].id)
			for(int j=0; j<bridge_network[i].adj_lans.size(); j++)
				for(int k=0; k<lan_network.size(); k++)
					if(bridge_network[i].adj_lans[j] == lan_network[k].id)
						for(int p=0; p<lan_network[k].adj_bridges.size();p++)
							if(lan_network[k].adj_bridges[p] != source.id)
							{
								message ms;
								ms.root = root;
								ms.dist = d;
								ms.source = source;
								ms.destination = lan_network[k].adj_bridges[p];
								ms.lan = lan_network[k].id;
								messages.insert(ms);
							}
	return messages;
}

message UpdateConfig(message m,vector<bridge>& bridge_network)
{
	int root = m.root;
	int d = m.dist;
	bridge source = m.source;
	int destination = m.destination;
	char lan = m.lan;

	message return_message;

	for(int i=0; i<bridge_network.size();i++)
	{
		if(destination == bridge_network[i].id)
		{
			bridge b = bridge_network[i];
			if(root < b.root)
			{
				return_message.root = root;
				return_message.dist = d+1;
				return_message.source=bridge_network[i];
				bridge_network[i].root = root;
				bridge_network[i].RP = make_pair(lan,source.id);
				bridge_network[i].root_distance = d+1;
			}
			else if(root == b.root && d+1 < bridge_network[i].root_distance)
			{
				return_message.root = root;
				return_message.dist = d+1;
				return_message.source=bridge_network[i];
				bridge_network[i].root = root;
				bridge_network[i].RP = make_pair(lan,source.id);
				bridge_network[i].root_distance = d+1;
			}
			else if (root == b.root && d+1 == bridge_network[i].root_distance && source.id<bridge_network[i].RP.second)
			{
				return_message.root = root;
				return_message.dist = d+1;
				return_message.source=bridge_network[i];
				bridge_network[i].root = root;				
				bridge_network[i].RP = make_pair(lan,source.id);
				bridge_network[i].root_distance = d+1;
			}
			else
			{
				return_message.root = -1;
				return_message.dist = d+1;
				return_message.source=bridge_network[i];
			}
		}
	}

	return return_message;
}

lan lookIntoTable(vector<ftable> forwarding_table, int d)
{
	lan l;
	l.id='\0';
	for(int i=0; i<forwarding_table.size();i++)
	{
		ftable f = forwarding_table[i];
		if(f.host == d) l = f.fport;
	}
	return l;
}
//--------------------------------------------------------------------------------------------------------

int main(){
	int tr; cin>>tr;
	int n;cin >> n;
	vector<bridge> bridge_network;
	set<char> lan_set;
	for(int i=0; i<=n; i++)
	{
		bridge b;
		string line;
		getline(cin,line);
		if(i!=0)
		{
			for(int j=0; j<line.size(); j++)
			{
				if(j==1)
				{
					if(line[j+1]!=':')
					{
						b.id = 10*((int) line[j] -48) + ((int) line[j+1] -48);
						b.root = b.id;
						j++;
					}
					else
					{
						b.id = (int) line[j] - 48;
						b.root = b.id;
					}
				}
				if(j!=0 & j!=1 & j!=2)
				{
					if(line[j] != ' ' and line[j]!=':') 
					{
						b.adj_lans.push_back(line[j]);
						lan_set.insert(line[j]);
					}
				}
			}
			sort(b.adj_lans.begin(),b.adj_lans.end());
			bridge_network.push_back(b);
		}
	}

	//cout<<"Initial Bridge Adjacency"<<endl;
	//print_bridge_network(bridge_network);  

	//Inputting bridges done, now constructing vector of lans

	vector<lan> lan_network;
	while(!lan_set.empty())
	{
		char c = *lan_set.begin();
		lan l;
		l.id = c;
		for(int i=0; i<bridge_network.size();i++)
			for(int j=0; j<bridge_network[i].adj_lans.size(); j++)
				if (bridge_network[i].adj_lans[j] == c) l.adj_bridges.push_back(bridge_network[i].id);
		lan_network.push_back(l);
		lan_set.erase(lan_set.begin());
	}

	//print_lan_network(lan_network);

	// Implementing spanning tree protocol

	queue<message> spawned,received;
	vector<class traces> traces_queue;
	int timestamp = 0,initial=1;

	for(int i=0; i<bridge_network.size();i++)
	{
		message m;
		m.root = bridge_network[i].id;
		m.dist=0;
		m.source=bridge_network[i];
		spawned.push(m);
	}

	while(!spawned.empty())
	{
		if(initial != 1)
		{
			while(!spawned.empty())
			{
				spawned.pop();
				//cout<<"spawned emptied"<<endl;
			}
		}

		while(!received.empty())
		{
			message m = received.front();
			//cout<<"Plucked out from received"<<endl;
			message to_be_published = UpdateConfig(m, bridge_network);
			if(to_be_published.root != -1) 
			{
				spawned.push(to_be_published);
				//cout<<"pushed in spawned"<<endl;
			}
			traces t;
			t.time = timestamp;
			t.bridge = m.destination;
			t.status = 'r';
			t.m = m;
			if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" (B"<<t.m.root<<" "<<t.m.dist<<" B"<<t.m.source.id<<")\n";
			traces_queue.push_back(t);
			received.pop();
			//cout<<"receive trace added"<<endl;
		}

		queue<message> temp;
		while(!spawned.empty())
		{
			message m = spawned.front();
			//cout<<"Plucked out from spawned"<<endl;
			messageSet received_by_set = SendMessage(m, bridge_network, lan_network);
			spawned.pop();
			while(!received_by_set.empty())
			{
				received.push(*received_by_set.begin());
				received_by_set.erase(received_by_set.begin());
			}
			traces t;
			t.time = timestamp;
			t.bridge = m.source.id;
			t.status = 's';
			t.m = m;
			traces_queue.push_back(t);
			if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" (B"<<t.m.root<<" "<<t.m.dist<<" B"<<t.m.source.id<<")\n";
			temp.push(m);
		}

		while(!temp.empty())
		{
			spawned.push(temp.front());
			temp.pop();
		}

		timestamp++;
		initial = 0;

	}

	//Protocol Implemented

	/*cout<<"Root Ports for Bridges"<<endl;
	for (int i=0; i<bridge_network.size();i++)
	{
		cout<<bridge_network[i].id<<" "<<bridge_network[i].RP.first<<endl;
	}
	cout<<endl;*/


	for(int i=0; i<lan_network.size(); i++)
	{
		lan_network[i].DP = *min_element(begin(lan_network[i].adj_bridges),end(lan_network[i].adj_bridges));
	}

	//cout<<"DP for lans"<<endl;
	//print_lan_network(lan_network);

	for(int i=0; i<bridge_network.size(); i++)
	{
		cout<<"B"<<bridge_network[i].id<<":";
		for(int j=0; j<bridge_network[i].adj_lans.size(); j++)
		{
			int flag=0;
			char c = bridge_network[i].adj_lans[j];
			cout<<" "<<c<<"-";
			if(c == bridge_network[i].RP.first) {cout<<"RP";flag=1;}
			for(int k=0;k<lan_network.size();k++)
			{
				if(c==lan_network[k].id && bridge_network[i].id==lan_network[k].DP && flag==0)
					{cout<<"DP";flag=1;break;}
			}
			if(flag==0) 
			{
				cout<<"NP";
				bridge_network[i].adj_lans.erase(remove(bridge_network[i].adj_lans.begin(), bridge_network[i].adj_lans.end(), c), bridge_network[i].adj_lans.end());
				j--;
			}
		}
		cout<<endl;
	}

	//Updating Lan network

	for(int i=0; i<lan_network.size();i++)
	{
		char c = lan_network[i].id;
		vector<int> l;
		for(int i=0; i<bridge_network.size();i++)
			for(int j=0; j<bridge_network[i].adj_lans.size(); j++)
				if (bridge_network[i].adj_lans[j] == c) l.push_back(bridge_network[i].id);
		lan_network[i].adj_bridges = l;
	}
	//cout<<" Final bridge network"<<endl;
	//print_lan_network(lan_network);
	
	//Data transfer Simulation

	//Inputting hosts connected LANs

	for(int i=0; i<lan_network.size(); i++)
	{
		string line;
		getline(cin,line);
		for(int j=0;j<line.size();j++)
		{
			if(line[j]!= ' ' && line[j]!=':' && line[j]!='H' && line[j]!=lan_network[i].id)
			{
				if(line[j+1]!= ' ' && line[j+1]!=':' && line[j+1]!='H' && line[j+1]!='\0')
				{
					lan_network[i].hosts.push_back(10*((int)line[j] -48) + ((int)line[j+1] -48));
					j++;
				}
				else lan_network[i].hosts.push_back((int)line[j] - 48);
			}
		}
	}
	//print_lan_network(lan_network);

	//Inputting Endpoints

	vector<traces> trace_data;

	cin>>n;
	for(int k=0; k<=n; k++)
	{
		int s,d,flag=0;
		string line;
		getline(cin,line);
		if(k!=0)
		{
			for (int j = 0; j < line.size(); j++)
			{
				if(line[j]!= ' ' && line[j]!='H')
				{
					if(line[j+1]!= ' ' && line[j+1]!='H' && line[j+1]!='\0')
					{
						if (flag == 0)
						{
							s = 10*((int)line[j] -48) + ((int)line[j+1] -48);
							flag=1;
						}
						else d = 10*((int)line[j] -48) + ((int)line[j+1] -48);
						j++;
					}
					else
					{
						if(flag==0) 
						{
							s = (int)line[j] - 48;
							flag=1;
						}
						else d = (int)line[j] - 48;
					}
				}
			}

			//Processing area;
			lan hostlan;
			for(int i=0; i<lan_network.size();i++)
				for(int p=0; p<lan_network[i].hosts.size(); p++)
					if(lan_network[i].hosts[p]==s) hostlan=lan_network[i];

			
			queue<data_packet> sent,received;

			for (int i=0; i<hostlan.adj_bridges.size(); i++)
			{
				data_packet data;
				data.source = s;
				data.destination = d;
				data.prev = hostlan.id;
				data.bridge = hostlan.adj_bridges[i];
				received.push(data);
			}
			int fl=1;
			while(!sent.empty() or fl==1)
			{
				while(!sent.empty())
				{
					data_packet data = sent.front();
					sent.pop();
					if(data.bridge > 0)
					{
						received.push(data);
						//cout<<data.bridge<<endl;
					}
					else
					{
						int ds = -1 * data.bridge;
						//cout<<"Bridge lt 0 and "<<data.bridge<<endl;
						lan l;
						char la=data.prev;
						for(int j=0; j<lan_network.size();j++)
							if(lan_network[j].id == la ) l=lan_network[j];

						for(int j=0; j<l.adj_bridges.size(); j++)
						{
							if(ds != l.adj_bridges[j])
							{
								data.bridge = l.adj_bridges[j];
								received.push(data);
								//cout<<"Pushed "<<data.bridge<<endl;
							}
						}
					}
				}

				while(!received.empty())
				{
					data_packet data = received.front();
					received.pop();
					for(int i=0; i<bridge_network.size();i++)
					{
						if(data.bridge == bridge_network[i].id)
						{
							lan lookin = lookIntoTable(bridge_network[i].forwarding_table, d);
							if(lookin.id != '\0')
							{
								int f=0;
								for(int j=0; j<lookin.hosts.size();j++)
								{
									if(lookin.hosts[j]==data.destination) 
									{
										f=1;
										for(int p=0; p<lookin.adj_bridges.size(); p++)
										{
											for(int q=0; q<bridge_network.size(); q++)
											{
												if(bridge_network[i].id!= bridge_network[q].id and bridge_network[q].id == lookin.adj_bridges[p] and lookIntoTable(bridge_network[q].forwarding_table, s).id == '\0')
												{
													ftable e; e.host = s; 
													//for(int j=0; j<lan_network.size();j++)
													//	if(lan_network[j].id == data.prev ) e.fport=lan_network[j];
													e.fport = lookin;
													bridge_network[q].forwarding_table.push_back(e);
													if(tr==1) cout<<timestamp<<" B"<<bridge_network[q].id<<" r "<<s<<"->"<<d<<" "<<data.prev<<endl;
												}
											}	
										}
									}
								}
								if(f==0 and data.prev!=lookin.id)
								{
									traces t;
									t.time = timestamp;
									t.bridge = bridge_network[i].id;
									t.status = 'r';
									message m;
									t.m.root = s;
									t.m.dist = d;
									trace_data.push_back(t);
									if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" "<<t.m.root<<"->"<<t.m.dist<<" "<<data.prev<<endl;
									t.status = 's';
									trace_data.push_back(t);
									if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" "<<t.m.root<<"->"<<t.m.dist<<" "<<lookin.id<<endl;

									for(int j=0; j<lookin.adj_bridges.size();j++)
									{
										data_packet datasend;
										datasend.source = data.source;
										datasend.destination = data.destination;
										datasend.prev = lookin.id;
										datasend.bridge = lookin.adj_bridges[j];
										//cout<<lookin.adj_bridges[j]<<" "<<bridge_network[i].id<<endl;
										if(lookin.adj_bridges[j] != bridge_network[i].id) sent.push(datasend);			
									}
								}
								if(lookIntoTable(bridge_network[i].forwarding_table, s).id == '\0')
								{
									ftable e; e.host = s; 
									for(int j=0; j<lan_network.size();j++)
										if(lan_network[j].id == data.prev ) e.fport=lan_network[j];
									bridge_network[i].forwarding_table.push_back(e);
								}
							}
							else
							{
								ftable e; e.host = s; 
								for(int j=0; j<lan_network.size();j++)
									if(lan_network[j].id == data.prev ) e.fport=lan_network[j];

								bridge_network[i].forwarding_table.push_back(e);

								for(int j=0; j<bridge_network[i].adj_lans.size(); j++)
								{
									data_packet datasend;
									datasend.source = data.source;
									datasend.destination = data.destination;
									datasend.prev = bridge_network[i].adj_lans[j];
									datasend.bridge = -1 * bridge_network[i].id;
									if(data.prev != bridge_network[i].adj_lans[j]) sent.push(datasend);
								}

								traces t;
								t.time = timestamp;
								t.bridge = bridge_network[i].id;
								t.status = 'r';
								message m;
								t.m.root = s;
								t.m.dist = d;
								if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" "<<t.m.root<<"->"<<t.m.dist<<" "<<data.prev<<endl;
								trace_data.push_back(t);
								t.status = 's';
								if(tr==1) cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" "<<t.m.root<<"->"<<t.m.dist<<" - "<<endl;
								trace_data.push_back(t);
							}
						}
					}
				}
				timestamp++;
				fl=0;
			}
			
			for(int i=0; i<bridge_network.size(); i++)
			{
				cout<<"B"<<bridge_network[i].id<<":"<<endl;
				cout<<"HOST ID | FORWARDING PORT"<<endl;
				for(int j=0; j<bridge_network[i].forwarding_table.size(); j++)
				{
					ftable f = bridge_network[i].forwarding_table[j];
					cout<<"H"<<f.host<<" | "<<f.fport.id<<endl;
				}
			}
			cout<<endl;




		}
	}

	if(tr==2)
	{
		for(int i=0;i<traces_queue.size();i++)
		{
			traces t = traces_queue[i];
			cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" (B"<<t.m.root<<", "<<t.m.dist<<", B"<<t.m.source.id<<")\n";
		}
		for(int i=0;i<trace_data.size();i++)
		{
			traces t = trace_data[i];
			cout<<t.time<<" B"<<t.bridge<<" "<<t.status<<" "<<t.m.root<<"->"<<t.m.dist<<endl;
		}
	}

}

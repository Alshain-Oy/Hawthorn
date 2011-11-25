#ifndef _LIBGRAPH_HPP_
#define _LIBGRAPH_HPP_

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <deque>
#include <stdint.h>

#include <time.h>

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <sys/time.h>

#include <leveldb/db.h>


class Node {
		static uint64_t _next_id;
	public:
		Node();
		Node( std::string );
		
		void set_id( uint64_t );
		void set_name( std::string );
		
		uint64_t get_id();
		std::string get_name();
		
		void connect( uint64_t, double );
		void disconnect( uint64_t );
		
		void back_connect( uint64_t );
		void back_disconnect( uint64_t );
		
		double get_weight( uint64_t );
		
		std::vector<uint64_t> get_connections();
		std::vector<uint64_t> get_back_connections();
	
		void add_property( std::string, std::string );
		void remove_property( std::string );
		
		std::string get_property( std::string );

		std::string serialize();
		
		uint64_t get_size();

		bool is_connected( uint64_t );

		static void _seed_id( uint64_t );
		static uint64_t generate_id();
	
		static Node* deserialize( std::string );
	
	private:
		std::string name;
		uint64_t id;
		
		std::map<uint64_t, double> conns;
		std::set<uint64_t> bconns;
		
		std::map<std::string, std::string> properties;
	

	};


class NodeStorage {
	
	public:
		NodeStorage();
		NodeStorage( uint64_t );
		~NodeStorage();
		
		bool open_storage( std::string );
		
		Node* fetch_node( uint64_t );
		void suppress_node( uint64_t );
		void insert_node( Node* );
		void remove_node( uint64_t );
		
		void set_max_size( uint64_t );

		uint64_t get_current_size();
		void compute_current_size();

		void compress();

		std::vector<std::string> get_values();

	private:
		std::vector< std::map<uint64_t, Node*> > nodes;
		uint64_t branch_mask;
		std::set<uint64_t> alive_nodes;
		std::deque<uint64_t> cache;
		
		uint64_t max_size;
		int64_t current_size;
		
		std::map<uint64_t, std::string> tmp_storage;
		
		leveldb::DB* db;
	
	};


class Graph {
	public:
		Graph();
		Graph( NodeStorage* );

		void set_storage( NodeStorage * );
	
		uint64_t create_node( std::string );
		
		void connect_nodes( std::string, std::string, double );
		
		void connect_nodes( uint64_t, uint64_t, double );
		
		void disconnect_nodes( std::string, std::string );
		
		void disconnect_nodes( uint64_t, uint64_t );
		
		Node* get_node( std::string );
		Node* get_node( uint64_t );
	
		void remove_node( std::string );
		void remove_node( uint64_t );
		
		NodeStorage* storage();
		
		std::vector<uint64_t> get_nodes();
		std::vector<std::pair< double, std::pair<uint64_t, uint64_t> > > get_edges();
		
		bool is_connected( std::string, std::string );
		bool is_connected( uint64_t, uint64_t );
		
		std::vector<uint64_t> find_shortest_path( std::string, std::string );
		std::vector<uint64_t> find_shortest_path( uint64_t, uint64_t );
		
		std::vector<uint64_t> find_longest_path( std::string, std::string );
		std::vector<uint64_t> find_longest_path( uint64_t, uint64_t );
		
		std::vector<uint64_t> find_nearest_nodes( std::string, double );
		std::vector<uint64_t> find_nearest_nodes( uint64_t, double );
		
		std::vector<uint64_t> find_connected_nodes( std::string );
		std::vector<uint64_t> find_connected_nodes( uint64_t );
		
		
		std::vector<uint64_t> filter_by_property( std::vector<uint64_t>, std::string, std::string, std::string );
		
		static std::vector<uint64_t> compute_union( std::vector<uint64_t>, std::vector<uint64_t> ); 
		static std::vector<uint64_t> compute_intersection( std::vector<uint64_t>, std::vector<uint64_t> ); 
		static std::vector<uint64_t> compute_complement( std::vector<uint64_t>, std::vector<uint64_t> ); 
		
		void restore_from_storage();
		
		std::vector< std::pair<uint64_t, uint64_t> > colour_graph();

		double compute_path_lenght( std::vector<uint64_t> );
		
	private:
		NodeStorage *node_storage;
		std::map<std::string, uint64_t> node_map;
		
		bool _dfs_is_connected( uint64_t, uint64_t, std::set<uint64_t> * );
		std::pair< bool, std::pair< std::vector<uint64_t>, double > > _dfs_shortest( uint64_t, uint64_t, std::vector<uint64_t>&, double, std::set<uint64_t>* );
		std::pair< bool, std::pair< std::vector<uint64_t>, double > > _dfs_longest( uint64_t, uint64_t, std::vector<uint64_t>&, double, std::set<uint64_t>* );
		void _dfs_nearest( uint64_t, double, std::set<uint64_t> * );
		void _dfs_connected( uint64_t, std::set<uint64_t> * );
	
	};







#endif

#include <iostream>
#include <map>


#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#include <sys/epoll.h>
#include <fcntl.h>

#include <signal.h>


#include <libgraph.hpp>

#include <pthread.h>


#include <leveldb/db.h>


#define CREATE_NODE 			1
#define CONNECT_NODES 			2
#define DISCONNECT_NODES		3
#define ADD_PROPERTY			4
#define GET_PROPERTY 			5
#define REMOVE_PROPERTY 		6
#define IS_CONNECTED 			7
#define GET_NODE 				8
#define FIND_SHORTEST_PATH 		9
#define FIND_LONGEST_PATH 		10
#define FIND_NEAREST_NODES 		11
#define GET_CHILDREN 			12
#define GET_PARENTS 			13
#define FILTER 					14
#define UNION 					15
#define INTERSECT 				16
#define COMPLEMENT 				17
#define REMOVE_NODE 			18
#define GET_RESULTS 			19
#define GET_CONNECTED 			20
#define REMOVE_RESULTS 			21
#define COLOUR_GRAPH 			22
#define COMPUTE_PATH_LENGTH		23





std::map<int, std::string> connection_buffers;

/*
 * 
 *  Message format: |uint64 total_len| pstr graph | uint64 stmt_len | stmst...|
 *  
 *  pstr = |uint64 len| ... |
 * 
 *  stmt = | uint64 command | pstr result_set | uint64 param_len | pstr params.. |
 * 
 * */

class HawthornStatement {
	public:
		HawthornStatement();
		
		void set_command( uint64_t );
		void set_result_set( std::string );
		void add_param( std::string );
	
		uint64_t get_command();
		std::string get_result_set();
		std::vector<std::string> get_params();
		
		static HawthornStatement deserialize( std::string );
	
	private:
		uint64_t command;
		std::string result;
		std::vector<std::string> params;
	};

HawthornStatement :: HawthornStatement(){}

void HawthornStatement :: set_command( uint64_t cmd ){ command = cmd; }
void HawthornStatement :: set_result_set( std::string res ){ result = res; }
void HawthornStatement :: add_param( std::string param ){ params.push_back( param ); }

uint64_t HawthornStatement :: get_command(){ return command; }
std::string HawthornStatement :: get_result_set() { return result; }
std::vector<std::string> HawthornStatement :: get_params() { return params; }


HawthornStatement HawthornStatement :: deserialize( std::string str ){
	HawthornStatement stmt;
	uint64_t value, length, pos = 0;
	const char *data = str.c_str();
	
	memcpy( &value, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	
	stmt.set_command( value );
	
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	stmt.set_result_set( str.substr( pos, length ) );
	pos += length;
	
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	for( uint64_t i = 0 ; i < length ; ++i ){
		memcpy( &value, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		stmt.add_param( str.substr( pos, value ) );
		pos += value;
		}
	
	
	return stmt;
	}



class HawthornBatch {
	public:
		HawthornBatch();
		
		void set_graph( std::string );
		std::string get_graph();
		
		void add_statement( HawthornStatement );
		std::vector<HawthornStatement> get_statements();
//		std::string serialize();
	
		static std::string deserialize( std::string, HawthornBatch& );
		static bool test_for_content( std::string );
	
		private:
			std::string graph;
			std::vector<HawthornStatement> statements;
			
	
	};

HawthornBatch :: HawthornBatch(){}

void HawthornBatch :: set_graph( std::string aGraph ){ graph = aGraph; }
std::string HawthornBatch :: get_graph(){ return graph; }
void HawthornBatch :: add_statement( HawthornStatement statement ){ statements.push_back( statement ); }
std::vector<HawthornStatement> HawthornBatch :: get_statements(){ return statements; }

std::string HawthornBatch :: deserialize( std::string str, HawthornBatch &batch ){
	const char* data = str.c_str();
	uint64_t length, value, pos;
	
	pos = 0;
	
	//std::cerr << "str.size() = " << str.size() << std::endl;
	pos += sizeof( uint64_t ); // skip total len
	//std::cerr << "pos: " << pos << std::endl;
	
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	//std::cerr << "pos: " << pos << std::endl;
	//std::cerr << "length: " << length << std::endl;
	
	batch.set_graph( str.substr( pos, length ) );
	pos += length;
	
	//std::cerr << "pos: " << pos << std::endl;
	
	
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	
	//std::cerr << "length: " << length << std::endl;
	
	for( uint64_t i = 0 ;  i < length ; ++i ){
		memcpy( &value, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		//std::cerr << "value = " << value << std::endl;
		batch.add_statement( HawthornStatement::deserialize( str.substr( pos, value ) ) );
		pos += value;
		//std::cerr << "pos = " << pos << std::endl;
		}
	
	return str.substr( pos, str.size() - pos );
	}

bool HawthornBatch :: test_for_content( std::string data ){
	if( data.size() >= sizeof( uint64_t ) ){
		uint64_t value;
		memcpy( &value, data.c_str(), sizeof( uint64_t ) );
		if( data.size() >= value ){ return true; }
		}
	return false;
	}

class HawthornResponse{
	public:
		HawthornResponse();
		void set_status( uint64_t );
		void add_result( std::string );
		
		std::string serialize();
	private:
		uint64_t status;
		std::vector<std::string> results;
	};

HawthornResponse :: HawthornResponse(){}

void HawthornResponse :: set_status( uint64_t s ){ status = s; }
void HawthornResponse :: add_result( std::string r ){ results.push_back( r ); }

std::string HawthornResponse :: serialize(){
	std::string out= "";
	char buf[32];
	uint64_t value;
	
	memset( buf, 0, 32 );
	memcpy( buf, &status, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	memset( buf, 0, 32 );
	value = results.size();
	memcpy( buf, &value, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	for( size_t i = 0 ; i < results.size() ; ++i ){
		memset( buf, 0, 32 );
		value = results[i].size();
		memcpy( buf, &value, sizeof( uint64_t ) );
		out += std::string( buf, sizeof( uint64_t ) );
		out += results[i];
		
		}
	
	memset( buf, 0, 32 );
	value = out.size() + sizeof( uint64_t );
	memcpy( buf, &value, sizeof( uint64_t ) );
	
	return std::string( buf, sizeof( uint64_t ) ) + out;
	
	}


std::string get_value_type( std::string str ){
	return str.substr( 0, 2 );
	}

bool is_string( std::string str ){
	return get_value_type( str ) == "s:";
	}

bool is_integer( std::string str ){
	return get_value_type( str ) == "i:" || get_value_type( str ) == "u:";
	}

bool is_double( std::string str ){
	return get_value_type( str ) == "f:";
	}

bool is_node( std::string str ){
	return get_value_type( str ) == "n:";
	}

std::string decode_value_as_string( std::string str ){
	std::string type = get_value_type( str );
	if( type == "u:" ){
		uint64_t value;
		char buf[128];
		memcpy( &value, str.c_str() + 2, sizeof( uint64_t ) );
		memset( buf, 0, 128 );
		sprintf( buf, "%lu", value );
		return std::string( buf );
		
		}
	return str.substr( 2, str.size() - 2 );
	}

uint64_t decode_value_as_integer( std::string str ){
	std::string type = get_value_type( str );
	if( type == "u:" ){
		uint64_t value;
		memcpy( &value, str.c_str() + 2, sizeof( uint64_t ) );
		return value;
		}
	if( type == "i:" ){
		uint64_t value = atol( str.substr(2, str.size()-2).c_str() );
		return value;
		}
	
	if( type == "f:" ){
		uint64_t value = atof( str.substr(2, str.size()-2).c_str() );
		return value;
		}
	
	return 0;
	}

double decode_value_as_double( std::string str ){
	std::string type = get_value_type( str );
	if( type == "u:" ){
		uint64_t value;
		memcpy( &value, str.c_str() + 2, sizeof( uint64_t ) );
		return value;
		}
	if( type == "i:" ){
		uint64_t value = atol( str.substr(2, str.size()-2).c_str() );
		return value;
		}
	if( type == "f:" ){
		double value = atof( str.substr( 2, str.size()-2).c_str() );
		return value;
		}
	return 0;
	}


std::string encode_value( std::string str ){
	return std::string( "s:" ) + str;
	}

std::string encode_value( uint64_t value, bool human_readable ){
	
	if( human_readable ){
		char buf[128];
		memset( buf, 0, 128 );
		sprintf( buf, "%lu", value );
		return std::string( "i:" ) + std::string( buf );
		}
	else{
		char buf[32];
		memset( buf, 0, 32 );
		memcpy( buf, &value, sizeof( uint64_t ) );
		return std::string( "u:" ) + std::string( buf, sizeof( uint64_t ) );
		}
	
	}

std::string encode_value( double value ){
	char buf[32];
	memset( buf, 0, 32 );
	sprintf( buf, "%f", value );
	return std::string( "f:" ) + std::string( buf );
	}

std::string encode_value( Node *node ){
	return std::string( "n:" ) + node->serialize();
	}

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}

uint64_t grab_node_id( std::string value ){
	uint64_t out;
	memcpy( &out, value.c_str() + 2, sizeof(uint64_t) );
	return out;
	}



std::map< std::string, Graph* > graphs;
std::map< std::string, NodeStorage* > storages;

std::map<std::string, pthread_mutex_t > write_locks;


std::map< std::string, std::vector<std::string> > result_sets;

void spawn_graph( std::string name ){
	storages[ name ] = new NodeStorage( 0xFFF );
	graphs[ name ] = new Graph( storages[ name ] );
	
	storages[name]->open_storage( std::string( "db/" ) + name );
	graphs[ name ]->restore_from_storage();
	
	pthread_mutex_init( &write_locks[ name ], NULL );
	}

void kill_graph( std::string name ){
	
	delete graphs[name];
	
	graphs.erase( name );
	
	delete storages[ name ];
	storages.erase( name );
	
	pthread_mutex_destroy( &write_locks[name] );
	
	write_locks.erase( name );
	
	}


void send_string( int sock, std::string str ){
	const char *data = str.c_str();
	size_t L = 0;
	int n;
	std::cerr << "Sending " << str.size() << " bytes." << std::endl;
	while( L < str.size() ){
		n = send( sock, data + L, L - str.size(), 0 );
		std::cerr << "L: " << L << std::endl;
		if( n > 0 ){ L += n; }
		}
	
	}

void run_batch( int sock, HawthornBatch &batch ){
	if( graphs.find( batch.get_graph() ) == graphs.end() ){
		spawn_graph( batch.get_graph() );
		}
		
	Graph *graph = graphs[ batch.get_graph() ];
	std::vector<HawthornStatement> stmts = batch.get_statements();
	HawthornResponse response;
	
	response.set_status( 0 );
	
	
	for( size_t i = 0 ; i < stmts.size() ; ++i ){
		std::vector<std::string> params = stmts[i].get_params();
		std::string results;
		std::string result_key = batch.get_graph() + "::" + stmts[i].get_result_set();
		std::string prefix = batch.get_graph() + "::";
		
		switch( stmts[i].get_command() ){
			
			case CREATE_NODE:
				std::cerr << "CREATE_NODE: " << decode_value_as_string( params[0] ) << std::endl;
				results = encode_value( graph->create_node( decode_value_as_string( params[0] ) ) );
				result_sets[ result_key ].push_back( results );
			
			break;
			
			case CONNECT_NODES:
				std::cerr << "CONNECT_NODES: " << decode_value_as_string( params[0] ) << ", " << decode_value_as_string( params[1] ) << " -> " << decode_value_as_double( params[2] ) << std::endl;
				
				if( is_integer( params[0] ) && is_integer( params[1] ) ){
					uint64_t idA, idB;
					double w;
					idA = decode_value_as_integer( params[0] );
					idB = decode_value_as_integer( params[1] );
					w = decode_value_as_double( params[2] );
					
					graph->connect_nodes( idA, idB, w );
					}
				else {
					double w = decode_value_as_double( params[2] );
					std::string idA, idB;
					idA = decode_value_as_string( params[0] );
					idB = decode_value_as_string( params[1] );
					graph->connect_nodes( idA, idB, w );					
					}
				
			break;
			
			case DISCONNECT_NODES:
				std::cerr << "DISCONNECT_NODES: " << decode_value_as_string( params[0] ) << ", " << decode_value_as_string( params[1] ) << std::endl;
				if( is_integer( params[0] ) && is_integer( params[1] ) ){
					uint64_t idA, idB;
					idA = decode_value_as_integer( params[0] );
					idB = decode_value_as_integer( params[1] );
					graph->disconnect_nodes( idA, idB );
					}
				else {
					std::string idA, idB;
					idA = decode_value_as_string( params[0] );
					idB = decode_value_as_string( params[1] );
					graph->disconnect_nodes( idA, idB );					
					}
				
			break;
			
			case ADD_PROPERTY:
				std::cerr << "ADD_PROPERTY: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << " = " << decode_value_as_string( params[2] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					std::string key, value;
					key = decode_value_as_string( params[1] );
					value = decode_value_as_string( params[2] );
					graph->get_node( id )->add_property( key, value );
					}
				else{
					std::string id = decode_value_as_string( params[0] );
					std::string key, value;
					key = decode_value_as_string( params[1] );
					value = decode_value_as_string( params[2] );
					graph->get_node( id )->add_property( key, value );
					}
			break;
			
			case GET_PROPERTY:
				std::cerr << "GET_PROPERTY: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					std::string key = decode_value_as_string( params[1] );
					result_sets[ result_key ].push_back( encode_value( graph->get_node( id )->get_property( key ) ) );
					}
				else {
					std::string id = decode_value_as_string( params[0] );
					std::string key = decode_value_as_string( params[1] );
					result_sets[ result_key ].push_back( encode_value( graph->get_node( id )->get_property( key ) ) );
					}
			break;
			
			case REMOVE_PROPERTY:
				std::cerr << "REMOVE_PROPERTY: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					std::string key = decode_value_as_string( params[1] );
					graph->get_node( id )->remove_property( key );
					}
				else {
					std::string id = decode_value_as_string( params[0] );
					std::string key = decode_value_as_string( params[1] );
					graph->get_node( id )->remove_property( key );
					}
			break;
			
			case IS_CONNECTED:
				std::cerr << "IS_CONNECTED: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << std::endl;
				if( is_integer( params[0] ) && is_integer( params[1] ) ){
					uint64_t idA, idB;
					idA = decode_value_as_integer( params[0] );
					idB = decode_value_as_integer( params[1] );
					result_sets[ result_key ].push_back( encode_value( (int)graph->is_connected( idA, idB ) ) );
					}
				else {
					std::string idA, idB;
					idA = decode_value_as_string( params[0] );
					idB = decode_value_as_string( params[1] );
					result_sets[ result_key ].push_back( encode_value( (int)graph->is_connected( idA, idB ) ) );
					}
			break;

			case GET_NODE:
				std::cerr << "GET_NODE: " << decode_value_as_string( params[0] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					result_sets[ result_key ].push_back( encode_value( graph->get_node( id ) ) );
					}
				else {
					std::string id = decode_value_as_string( params[0] );
					result_sets[ result_key ].push_back( encode_value( graph->get_node( id ) ) );
					}
			break;
			
			case FIND_SHORTEST_PATH:
				std::cerr << "FIND_SHORTEST_PATH: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << " (" << result_key << ")" << std::endl;
				if( is_integer( params[0] ) && is_integer( params[1] ) ){
					uint64_t idA, idB;
					idA = decode_value_as_integer( params[0] );
					idB = decode_value_as_integer( params[1] );
					std::vector<uint64_t> path = graph->find_shortest_path( idA, idB );
					for( size_t i = 0 ;  i< path.size() ; ++i ){
						result_sets[ result_key ].push_back( graph->get_node( path[i] )->serialize() );
						}
					
					}
				else {
					std::string idA, idB;
					idA = decode_value_as_string( params[0] );
					idB = decode_value_as_string( params[1] );
					std::vector<uint64_t> path = graph->find_shortest_path( idA, idB );
					for( size_t i = 0 ;  i< path.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( path[i] ) ) );
						}
					}
			break;

			case FIND_LONGEST_PATH:
				std::cerr << "FIND_LONGEST_PATH: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_string( params[1] ) << std::endl;
				if( is_integer( params[0] ) && is_integer( params[1] ) ){
					uint64_t idA, idB;
					idA = decode_value_as_integer( params[0] );
					idB = decode_value_as_integer( params[1] );
					std::vector<uint64_t> path = graph->find_longest_path( idA, idB );
					for( size_t i = 0 ;  i< path.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( path[i] ) ) );
						}
					
					}
				else {
					std::string idA, idB;
					idA = decode_value_as_string( params[0] );
					idB = decode_value_as_string( params[1] );
					std::vector<uint64_t> path = graph->find_longest_path( idA, idB );
					for( size_t i = 0 ;  i< path.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( path[i] ) ) );
						}
					}
			break;
			
			case FIND_NEAREST_NODES:
				std::cerr << "FIND_NEAREST_NODES: " << decode_value_as_string( params[0] ) << " -> " << decode_value_as_double( params[1] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					double d = decode_value_as_double( params[1] );
					std::vector<uint64_t> nodes = graph->find_nearest_nodes( id, d );
					for( size_t i = 0 ;  i< nodes.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( nodes[i] ) ) );
						}
					}
				else {
					std::string id = decode_value_as_string( params[0] );
					double d = decode_value_as_double( params[1] );
					std::vector<uint64_t> nodes = graph->find_nearest_nodes( id, d );
					for( size_t i = 0 ;  i< nodes.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( nodes[i] ) ) );
						}
					}
			break;

			case GET_CHILDREN:
				std::cerr << "GET_CHILDREN: " << decode_value_as_string( params[0] ) << std::endl;
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string key = prefix + decode_value_as_string( params[0] );
					std::vector<uint64_t> nodes;
					std::set<uint64_t> out;
					for( size_t i = 0 ; i < result_sets[key].size() ; ++i ){
						if( is_node( result_sets[key][i] ) ){
							nodes.push_back( grab_node_id( result_sets[key][i]  ) );
							}
						}
					for( size_t i = 0 ; i < nodes.size() ; ++i ){
						std::vector<uint64_t> conns = graph->get_node( nodes[i] )->get_connections();
						for( size_t j = 0 ; j < conns.size() ; ++j ){
							out.insert( conns[i] );
							}
						
						}
					for( std::set<uint64_t>::iterator it = out.begin() ; it != out.end() ; it++ ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( *it ) ) );
						}
					}
			break;
			
			case GET_PARENTS:
				std::cerr << "GET_PARENTS: " << decode_value_as_string( params[0] ) << std::endl;
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string key = prefix + decode_value_as_string( params[0] );
					std::vector<uint64_t> nodes;
					std::set<uint64_t> out;
					for( size_t i = 0 ; i < result_sets[key].size() ; ++i ){
						if( is_node( result_sets[key][i] ) ){
							nodes.push_back( grab_node_id( result_sets[key][i]  ) );
							}
						}
					for( size_t i = 0 ; i < nodes.size() ; ++i ){
						std::vector<uint64_t> conns = graph->get_node( nodes[i] )->get_back_connections();
						for( size_t j = 0 ; j < conns.size() ; ++j ){
							out.insert( conns[i] );
							}
						
						}
					for( std::set<uint64_t>::iterator it = out.begin() ; it != out.end() ; it++ ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( *it ) ) );
						}
					}
			break;
			
			case FILTER:
				std::cerr << "FILTER: " << decode_value_as_string( params[0] )<< " -> " << decode_value_as_string( params[1] ) << " " << decode_value_as_string( params[2] ) << " " <<decode_value_as_string( params[0] )  << std::endl;
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){

					std::string rkey = prefix + decode_value_as_string( params[0] );

					std::string key = decode_value_as_string( params[1] );
					std::string value = decode_value_as_string( params[2] );
					std::string op = decode_value_as_string( params[3] );

					std::vector<uint64_t> nodes;
					std::vector<uint64_t> out;
					for( size_t i = 0 ; i < result_sets[rkey].size() ; ++i ){
						if( is_node( result_sets[rkey][i] ) ){
							nodes.push_back( grab_node_id( result_sets[rkey][i]  ) );
							}
						}
					out = graph->filter_by_property( nodes, key, value, op );
					for( size_t i = 0 ; i < out.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( out[i] ) ) );
						}
					
					}
			break;
			case UNION:
				std::cerr << "UNION: " << decode_value_as_string( params[0] ) << " " << decode_value_as_string( params[1] ) << std::endl;
				
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string rkey0 = prefix + decode_value_as_string( params[0] );
					std::string rkey1 = prefix + decode_value_as_string( params[1] );
					std::vector<uint64_t> nodesA, nodesB, out;
					for( size_t i = 0 ; i < result_sets[rkey0].size() ; ++i ){
						if( is_node( result_sets[rkey0][i] ) ){
							nodesA.push_back( grab_node_id( result_sets[rkey0][i]  ) );
							}
						}
					for( size_t i = 0 ; i < result_sets[rkey1].size() ; ++i ){
						if( is_node( result_sets[rkey1][i] ) ){
							nodesB.push_back( grab_node_id( result_sets[rkey1][i]  ) );
							}
						}
					
					out = graph->compute_union( nodesA, nodesB );
					for( size_t i = 0 ; i < out.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( out[i] ) ) );
						}					
					}
			break;
			case INTERSECT:
				std::cerr << "INTERSECT: " << decode_value_as_string( params[0] ) << " " << decode_value_as_string( params[1] ) << std::endl;
				
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string rkey0 = prefix + decode_value_as_string( params[0] );
					std::string rkey1 = prefix + decode_value_as_string( params[1] );
					std::vector<uint64_t> nodesA, nodesB, out;
					for( size_t i = 0 ; i < result_sets[rkey0].size() ; ++i ){
						if( is_node( result_sets[rkey0][i] ) ){
							nodesA.push_back( grab_node_id( result_sets[rkey0][i]  ) );
							}
						}
					for( size_t i = 0 ; i < result_sets[rkey1].size() ; ++i ){
						if( is_node( result_sets[rkey1][i] ) ){
							nodesB.push_back( grab_node_id( result_sets[rkey1][i]  ) );
							}
						}
					
					out = graph->compute_intersection( nodesA, nodesB );
					for( size_t i = 0 ; i < out.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( out[i] ) ) );
						}					
					}
			break;
			case COMPLEMENT:
				std::cerr << "COMPLEMENT: " << decode_value_as_string( params[0] ) << " " << decode_value_as_string( params[1] ) << std::endl;
				
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string rkey0 = prefix + decode_value_as_string( params[0] );
					std::string rkey1 = prefix + decode_value_as_string( params[1] );
					std::vector<uint64_t> nodesA, nodesB, out;
					for( size_t i = 0 ; i < result_sets[rkey0].size() ; ++i ){
						if( is_node( result_sets[rkey0][i] ) ){
							nodesA.push_back( grab_node_id( result_sets[rkey0][i]  ) );
							}
						}
					for( size_t i = 0 ; i < result_sets[rkey1].size() ; ++i ){
						if( is_node( result_sets[rkey1][i] ) ){
							nodesB.push_back( grab_node_id( result_sets[rkey1][i]  ) );
							}
						}
					
					out = graph->compute_complement( nodesA, nodesB );
					for( size_t i = 0 ; i < out.size() ; ++i ){
						result_sets[ result_key ].push_back( encode_value( graph->get_node( out[i] ) ) );
						}					
					}
			break;
			
			case REMOVE_NODE:
				std::cerr << "REMOVE_NODE: " << decode_value_as_string( params[0] ) << std::endl;
				if( is_integer( params[0] ) ){
					uint64_t id = decode_value_as_integer( params[0] );
					graph->remove_node( id );
					}
				else{
					std::string id = decode_value_as_string( params[0] );
					graph->remove_node( id );
					}
			break;
			
			case GET_RESULTS:
				std::cerr << "GET_RESULTS: " << result_key << std::endl;
				for( size_t i = 0 ; i < result_sets[ result_key ].size() ; ++i ){
					response.add_result( result_sets[ result_key ][i] );
					}
			break;
			
			case REMOVE_RESULTS:
				std::cerr << "REMOVE_RESULTS: " << result_key << std::endl;
				result_sets.erase( result_key );
			break;
			
			case COLOUR_GRAPH:
				std::cerr << "COLOUR_GRAPH" << std::endl;
				
				{
				std::vector< std::pair<uint64_t, uint64_t> > pairs = graph->colour_graph();
				for( size_t i = 0 ; i < pairs.size() ; ++i ){
					std::string str = encode_value( encode_value( pairs[i].first, true ) + "," + encode_value( pairs[i].second, true ) );
					result_sets[result_key].push_back( str );
					}
					
				}
			
			break;

			case COMPUTE_PATH_LENGTH:
				std::cerr << "COMPUTE_PATH_LENGTH: " << decode_value_as_string( params[0] ) << std::endl;
				if( result_sets.find( prefix + decode_value_as_string( params[0] ) ) != result_sets.end() ){
					std::string key = prefix + decode_value_as_string( params[0] );
					std::vector<uint64_t> nodes;
					double len = 0;
					for( size_t i = 0 ; i < result_sets[ key ].size() ; ++i ){
						if( is_node( result_sets[key][i] ) ){
							nodes.push_back( grab_node_id( result_sets[key][i] ) );
							}
						}
					result_sets[ result_key ].push_back( encode_value( graph->compute_path_lenght( nodes ) ) );
					}else{ std::cerr << "Result set not found!" << std::endl; }
			break;
			
			}
		
		
		}
	//std::cerr << std::endl;
	std::cerr << "Sending response.." << std::endl;
	send_string( sock, response.serialize() );
	
	}


int main( int argc, char **argv ){
	int serversock, clientsock;
	
	int yes = 1;
	
	//struct sockaddr_in address;
	struct sockaddr_un address;
	
	int epfd = epoll_create( 1 );
	
	static struct epoll_event ev, events[32]; 
	
	int i, N, fd, addrlen, flags, n;
	
	char buffer[ 4096 ];
	
	bool done = false;
	
	
	serversock = socket( AF_UNIX, SOCK_STREAM, 0 );
	unlink( "/tmp/hawthorn.socket" );
	
	memset( &address, 9, sizeof( struct sockaddr_un ) );
	address.sun_family = AF_UNIX;
	sprintf( address.sun_path, "/tmp/hawthorn.socket" );
	
	
	bind( serversock, (struct sockaddr *)&address, sizeof( address ) );
	
	listen( serversock, 17 );
	
	ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	ev.data.fd = serversock;
	
	epoll_ctl( epfd, EPOLL_CTL_ADD, serversock, &ev );
	
	std::cout << "Server started." << std::endl;
	
	signal( SIGPIPE, SIG_IGN );
	
	while( !done ){
		N = epoll_wait( epfd, events, 32, -1 );
		for( i = 0 ; i < N  ; ++i ){
			
			if( events[i].data.fd == serversock ){
				
				addrlen = sizeof( struct sockaddr_in );
				clientsock = accept( serversock, (struct sockaddr*)&address, (socklen_t *)&addrlen );
				flags = fcntl( clientsock, F_GETFL, 0 );
				fcntl( clientsock, F_SETFL, flags | O_NONBLOCK );
				
				//setsockopt( clientsock, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof( int ) );
	
				ev.events = EPOLLIN /*| EPOLLET */| EPOLLERR | EPOLLRDHUP;
				ev.data.fd = clientsock;
				n = epoll_ctl( epfd, EPOLL_CTL_ADD, clientsock, &ev );
				connection_buffers[ clientsock ] = std::string();
				std::cerr << "Client connected." << std::endl;
				}
			else {
				if( events[i].events & EPOLLIN ){
					memset( buffer, 0, 4096 );
					n = recv( events[i].data.fd, buffer, 4090, 0 );
					if( n > 0 ){
						//std::cerr << "Data received!" << std::endl;
						connection_buffers[events[i].data.fd] += std::string( buffer, n );
						if( HawthornBatch::test_for_content( connection_buffers[events[i].data.fd] ) ){
							HawthornBatch batch;
							connection_buffers[events[i].data.fd] = HawthornBatch::deserialize( connection_buffers[events[i].data.fd], batch );
							run_batch( events[i].data.fd, batch ); 
							}
						
						}
					
					}
	
				if( events[i].events & EPOLLRDHUP ){
					std::cerr << "Client disconnected." << std::endl;
					connection_buffers.erase( events[i].data.fd );
					close( events[i].data.fd );
					
					
					}
				
				
				}
			
			
			}
		
		
		}
	
	
	
	
	return 0;
	}

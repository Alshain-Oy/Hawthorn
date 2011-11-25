#include "libgraph.hpp"

uint64_t Node::_next_id = 0;


uint64_t Node :: generate_id(){
	return Node::_next_id++;
	}

void Node :: _seed_id( uint64_t new_id ){
	Node::_next_id = new_id;
	}

Node :: Node (){
	id = Node::generate_id();
	}

Node :: Node( std::string aName ){
	name = aName,
	id = Node::generate_id();
	}


void Node :: set_id( uint64_t aId ){ id = aId; }
void Node :: set_name( std::string aName ){ name = aName; }

uint64_t Node :: get_id(){ return id; }
std::string Node :: get_name(){ return name; }


void Node :: connect( uint64_t node_id, double weight ){
	conns[ node_id ] = weight;
	}

void Node :: disconnect( uint64_t node_id ){
	conns.erase( node_id );
	}

void Node :: back_connect( uint64_t node_id ){
	bconns.insert( node_id );
	}

void Node :: back_disconnect( uint64_t node_id ){
	bconns.erase( node_id );
	}

double Node :: get_weight( uint64_t node_id ){
	return conns[ node_id ];
	}

std::vector<uint64_t> Node :: get_connections(){
	std::vector<uint64_t> out;
	for( std::map<uint64_t,double>::iterator it = conns.begin() ; it != conns.end() ; it++ ){
		out.push_back( (*it).first );
		}
	return out;
	}

std::vector<uint64_t> Node :: get_back_connections(){
	std::vector<uint64_t> out;
	for( std::set<uint64_t>::iterator it = bconns.begin() ; it != bconns.end() ; it++ ){
		out.push_back( (*it) );
		}
	return out;
	}

void Node :: add_property( std::string key, std::string value ){
	properties[ key ] = value;
	}

void Node :: remove_property( std::string key ){
	properties.erase( key );
	}

std::string Node :: get_property( std::string key ){
	if( properties.find( key ) != properties.end() ){ return properties[ key ]; }
	
	return std::string();
	}


std::string Node :: serialize(){
	std::string out;
	char buf[32];
	uint64_t length;
	
	length = id;
	
	memset( buf, 0, 32 );
	memcpy( buf, &length, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	length = name.size();
	
	memset( buf, 0, 32 );
	memcpy( buf, &length, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	out += name;
	
	length = conns.size();
	
	memset( buf, 0, 32 );
	memcpy( buf, &length, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	
	for( std::map<uint64_t,double>::iterator it = conns.begin() ; it != conns.end() ; it++ ){
		length = it->first;
		memset( buf, 0, 32 );
		memcpy( buf, &length, sizeof( uint64_t ) );
		out += std::string( buf, sizeof( uint64_t ) );
	
		
		memset( buf, 0, 32 );
		memcpy( buf, &(it->second), sizeof( double ) );
		out += std::string( buf, sizeof( double ) );
	
		}
	
	length = bconns.size();
	
	memset( buf, 0, 32 );
	memcpy( buf, &length, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	for( std::set<uint64_t>::iterator it = bconns.begin() ; it != bconns.end() ; it++ ){
		length = (*it);
		memset( buf, 0, 32 );
		memcpy( buf, &length, sizeof( uint64_t ) );
		out += std::string( buf, sizeof( uint64_t ) );
		}
	
	
	length = properties.size();
	
	memset( buf, 0, 32 );
	memcpy( buf, &length, sizeof( uint64_t ) );
	out += std::string( buf, sizeof( uint64_t ) );
	
	
	for( std::map<std::string,std::string>::iterator it = properties.begin() ; it != properties.end() ; it++ ){
		length = (*it).first.size();
		memset( buf, 0, 32 );
		memcpy( buf, &length, sizeof( uint64_t ) );
		out += std::string( buf, sizeof( uint64_t ) );
		out += (*it).first;
		
		length = (*it).second.size();
		memset( buf, 0, 32 );
		memcpy( buf, &length, sizeof( uint64_t ) );
		out += std::string( buf, sizeof( uint64_t ) );
		out += (*it).second;
		}
	
	return out;
	}


Node* Node :: deserialize( std::string str ){
	Node* node = new Node();
	size_t pos = 0;
	uint64_t length, conn_length, bconn_length, prop_length;
	double w;
	const char *data = str.c_str();
	
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	node->set_id( length );
	memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	node->set_name( std::string( data + pos, length ) ); pos += length;
	
	memcpy( &conn_length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	for( size_t i = 0 ; i < conn_length ; ++i ){
		memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		memcpy( &w, data + pos, sizeof( double ) ); pos += sizeof( double );
		node->connect( length, w );
		} 
	
	memcpy( &bconn_length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	for( size_t i = 0 ; i < bconn_length ; ++i ){
		memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		node->back_connect( length );
		}
	
	memcpy( &prop_length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
	for( size_t i = 0 ; i < prop_length ; ++i ){
		std::string key, value;
		memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		key = std::string( data + pos, length ); pos += length;
		memcpy( &length, data + pos, sizeof( uint64_t ) ); pos += sizeof( uint64_t );
		value = std::string( data + pos, length ); pos += length;
		node->add_property( key, value );
		}
	
	
	return node;
	}

uint64_t Node :: get_size(){
	uint64_t out = 0;
	
	out += name.size();
	out += sizeof( uint64_t );
	
	for( std::map<uint64_t, double>::iterator it = conns.begin() ; it != conns.end() ; it++ ){
		out += sizeof( uint64_t ) + sizeof( double );
		}
	out += bconns.size() * sizeof( uint64_t );
	
	for( std::map<std::string, std::string>::iterator it = properties.begin() ; it != properties.end() ; it++ ){
		out += it->first.size() + it->second.size();
		}
	return out;
	}


bool Node :: is_connected( uint64_t node_id ){
	
	if( conns.find( node_id ) != conns.end() ){
		return true;
		}
	
	return false;
	}




NodeStorage :: NodeStorage(){
	branch_mask = 0;
	nodes.push_back( std::map<uint64_t,Node*>() );
	max_size = 4;
	max_size <<= 30;
	current_size = 0;
	db = NULL;
	}

NodeStorage :: NodeStorage( uint64_t mask ){
	branch_mask = mask;
	for( uint64_t i = 0 ; i < mask + 2 ; ++i ){
		nodes.push_back( std::map<uint64_t,Node*>() );
		}
	max_size = 4;
	max_size <<= 30;
	current_size = 0;
	db = NULL;
	}

NodeStorage :: ~NodeStorage(){
	delete db;
	}

bool NodeStorage :: open_storage( std::string fn ){

	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, fn, &db);

	return status.ok();
	
	}
		

uint64_t  NodeStorage :: get_current_size(){
	if( current_size < 0 ){ return 0; }
	return current_size;
	}

void NodeStorage :: set_max_size( uint64_t aSize ){
	max_size = aSize;
	}

void NodeStorage :: compute_current_size(){
	current_size = 0;
	for( std::set<uint64_t>::iterator it = alive_nodes.begin() ; it != alive_nodes.end() ; it++ ){
		current_size += fetch_node( *it )->get_size();
		}
	
	}

Node* NodeStorage :: fetch_node( uint64_t node_id ){
	
	uint64_t bucket = node_id & branch_mask;
	
	//std::cerr << "id = " << node_id << " bucket = " << bucket << " (# of buckets = "<< nodes.size() << " )" << std::endl;
	
	std::map<uint64_t,Node*>::iterator it = nodes[ bucket ].find( node_id );
	
	if( it == nodes[ bucket ].end() ){
		return NULL;
		}
	
	if( (*it).second == NULL ){
		/* Fetch from storage */ 
		Node *node;
		if( db == NULL ){
			node = Node::deserialize( tmp_storage[ node_id ] );
			}
		else{
			char buf[32];
			memset( buf, 0, 32 );
			sprintf( buf, "node%lu", node_id );
			std::string key, value;
			key = std::string( buf );
			leveldb::Status s = db->Get( leveldb::ReadOptions(), key, &value );
			node = Node::deserialize( value );
			}
		nodes[bucket][node_id] = node;
		alive_nodes.insert( node_id );
		cache.push_back( node_id );
	
		current_size += nodes[bucket][node_id]->get_size();
		}
	
	
	return nodes[bucket][node_id];
	
	}


void NodeStorage :: suppress_node( uint64_t id ){
	
	uint64_t bucket = id & branch_mask;
	
	alive_nodes.erase( id );
	
	std::map<uint64_t,Node*>::iterator it = nodes[ bucket ].find( id );
	
	if( it != nodes[ bucket ].end() ){
		
		if( it->second == NULL ){ return; }
		
		current_size -= it->second->get_size();
		
		if( db == NULL ){
			tmp_storage[ id ] = it->second->serialize();
			}
		else {
			char buf[32];
			memset( buf, 0, 32 );
			sprintf( buf, "node%lu", id );
			std::string key, value;
			key = std::string( buf );
			value = it->second->serialize();
			leveldb::Status s = db->Put( leveldb::WriteOptions(), key, value );
			
			}
			
		delete nodes[ bucket ][ id ];
		nodes[ bucket ][ id ] = NULL;
		
		}
	
	
	}


void NodeStorage :: insert_node( Node *node ){
	
	uint64_t bucket = node->get_id() & branch_mask;
	
	//std::cerr << "id = " << node->get_id() << " bucket = " << bucket << " (# of buckets = "<< nodes.size() << " )" << std::endl;
	
	nodes[ bucket ][ node->get_id() ] = node;
	
	current_size += node->get_size();
	
	
	alive_nodes.insert( node->get_id() );
	cache.push_back( node->get_id() );
	
	}

void NodeStorage :: remove_node( uint64_t id ){
	
	uint64_t bucket = id & branch_mask;
	
	if( db != NULL ){
		char buf[32];
		memset( buf, 0, 32 );
		sprintf( buf, "node%lu", id );
		std::string key, value;
		key = std::string( buf );
		leveldb::Status s = db->Delete( leveldb::WriteOptions(), key );
		}
	
	if( nodes[ bucket ][ id ] != NULL ){
		current_size -= nodes[ bucket ][ id ]->get_size();
		delete nodes[ bucket ][ id ];
		}
	nodes[bucket].erase( id );
	alive_nodes.erase( id );
	}

void NodeStorage :: compress(){
	
	compute_current_size();
	//std::cerr << "NodeStorage::compress, current size = " << current_size << std::endl;
	while( current_size > max_size && cache.size() > 0 ){
		uint64_t id = cache.front();
		cache.pop_front();
		if( alive_nodes.find( id ) != alive_nodes.end() ){
			suppress_node( id );
			}
		
		}
	//std::cerr << "NodeStorage::compress, current size = " << current_size << ", max_size = "<< max_size << std::endl;
	
	
	}

std::vector<std::string> NodeStorage :: get_values(){
	std::vector<std::string> out;
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for( it->SeekToFirst(); it->Valid(); it->Next() ) {
		//std::cout << it->key().ToString() << std::endl;
		out.push_back( it->value().ToString() );
		}
	
	delete it;
	
	return out;
	}



Graph :: Graph(){
	node_storage = NULL;
	}

Graph :: Graph( NodeStorage *aStorage ){
	node_storage = aStorage;
	}

void Graph :: set_storage( NodeStorage *aStorage ){
	node_storage = aStorage;
	}

uint64_t Graph :: create_node( std::string name ){
	if( node_map.find( name ) == node_map.end() ){
		Node *node = new Node( name );
	
		node_storage->insert_node( node );
	
		node_map[ name ] = node->get_id();
	
		return node->get_id();
		}
	else{
		return node_map[ name ];
		}
	}

void Graph :: connect_nodes( std::string nameA, std::string nameB, double w ){
	
	std::map<std::string, uint64_t>::iterator it0, it1;
	
	it0 = node_map.find( nameA );
	it1 = node_map.find( nameB );
	
	if( it0 == node_map.end() || it1 == node_map.end() ){ return; }
	
	connect_nodes( it0->second, it1->second, w );
	
	}

void Graph :: connect_nodes( uint64_t idA, uint64_t idB, double w ){
	
	Node *nodeA, *nodeB;
	
	nodeA = node_storage->fetch_node( idA );
	nodeB = node_storage->fetch_node( idB );
	
	if( nodeA == NULL || nodeB == NULL ){ return; }
	
	nodeA->connect( idB, w );
	nodeB->back_connect( idA );
	
	}

void Graph :: disconnect_nodes( std::string nameA, std::string nameB ){
	
	std::map<std::string, uint64_t>::iterator it0, it1;
	
	it0 = node_map.find( nameA );
	it1 = node_map.find( nameB );
	
	if( it0 == node_map.end() || it1 == node_map.end() ){ return; }
	
	disconnect_nodes( it0->second, it1->second );
	
	}

void Graph :: disconnect_nodes( uint64_t idA, uint64_t idB ){
	
	Node *nodeA, *nodeB;
	
	nodeA = node_storage->fetch_node( idA );
	nodeB = node_storage->fetch_node( idB );
	
	if( nodeA == NULL || nodeB == NULL ){ return; }
	
	nodeA->disconnect( idB );
	nodeB->back_disconnect( idA );
	
	}


Node* Graph :: get_node( std::string name ){
	std::map<std::string, uint64_t>::iterator it;
	
	it = node_map.find( name );
	
	if( it == node_map.end() ){ return NULL; }
	
	return get_node( it->second );
	
	}

Node* Graph :: get_node( uint64_t id ){
	
	return node_storage->fetch_node( id );
	
	}


void Graph :: remove_node( std::string name ){
	std::map<std::string, uint64_t>::iterator it;
	
	it = node_map.find( name );
	
	if( it == node_map.end() ){ return; }
	
	return remove_node( it->second );
	
	}

void Graph :: remove_node( uint64_t id ){
	
	Node *node = node_storage->fetch_node( id );
	
	if( node == NULL ){ return; }
	
	std::vector<uint64_t> conns = node->get_connections();
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		//node->disconnect( conns[i] );
		disconnect_nodes( node->get_id(), conns[i] );
		}
	
	node_storage->remove_node( node->get_id() );
	
	}

NodeStorage* Graph :: storage(){ return node_storage; }

std::vector<uint64_t> Graph :: get_nodes(){
	std::vector<uint64_t> out;
	
	for( std::map<std::string,uint64_t>::iterator it = node_map.begin() ; it != node_map.end() ; it++ ){
		out.push_back( it->second );
		}
	return out;
	}

std::vector<std::pair< double, std::pair<uint64_t, uint64_t> > > Graph :: get_edges(){
	std::vector<std::pair< double, std::pair<uint64_t, uint64_t> > > out;
	
	std::vector<uint64_t> nodes = get_nodes();
	
	for( size_t i = 0 ; i < nodes.size() ; ++i ){
		Node *node = node_storage->fetch_node( nodes[i] );
		std::vector<uint64_t> conns = node->get_connections();
		
		for( size_t j = 0 ; j < conns.size() ; ++j ){
			
			out.push_back( std::make_pair( node->get_weight( conns[j] ), std::make_pair( nodes[i], conns[j] ) ) );
			
			}
		
		}
	return out;
	
	}


bool Graph :: _dfs_is_connected( uint64_t current, uint64_t target, std::set<uint64_t> *visited ){
	
	
	if( visited->find( current ) != visited->end() ){
		return false;
		}
	
	visited->insert( current );
	
	
	Node *node = node_storage->fetch_node( current );
	

	if( node->is_connected( target ) ){
		return true;
		}
	
	std::vector<uint64_t> conns = node->get_connections();		
		
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		if( _dfs_is_connected( conns[i], target, visited ) ){
			return true;
			}
		}
	
	return false;
	}

bool Graph :: is_connected( std::string nameA, std::string nameB ){
	
	std::map<std::string, uint64_t>::iterator it0, it1;
	
	it0 = node_map.find( nameA );
	it1 = node_map.find( nameB );
	
	if( it0 == node_map.end() || it1 == node_map.end() ){ return false; }
	
	return is_connected( it0->second, it1->second );
	
	}

bool Graph :: is_connected( uint64_t source, uint64_t target ){
	
	std::set<uint64_t> *visited = new std::set<uint64_t>();
	
	if( node_storage->fetch_node( source )->is_connected( target ) ){ return true; }
	
	bool out = _dfs_is_connected( source, target, visited );
	
	delete visited;
	
	return out;
	}


std::pair< bool, std::pair< std::vector<uint64_t>, double > > Graph :: _dfs_shortest( 	uint64_t current, 
																						uint64_t target, 
																						std::vector<uint64_t> &path, 
																						double dist,
																						std::set<uint64_t> *visited ){
	
	if( visited->find( current ) != visited->end() ){
		return std::make_pair(false, std::make_pair( std::vector<uint64_t>(), -1 ) );
		}
	
	visited->insert( current );
	
	Node *node = node_storage->fetch_node( current );
	
	if( node->is_connected( target ) ){
		dist += node->get_weight( target );
		std::vector<uint64_t> new_path = path;
		new_path.push_back( target );
		return std::make_pair( true, std::make_pair( new_path, dist ) );
		}
	
	std::vector<uint64_t> conns = node->get_connections();
	
	double best = 1e99;
	
	std::pair< bool, std::pair< std::vector<uint64_t>, double > > out, test;
	
	out = std::make_pair( false, std::make_pair( std::vector<uint64_t>(), -1 ) );
	
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		if( dist + node->get_weight( conns[i] ) < best ){
			std::vector<uint64_t> new_path = path;
			new_path.push_back( conns[i] );
			test = _dfs_shortest( conns[i], target, new_path, dist + node->get_weight( conns[i] ), visited );
			
			if( test.first ){
				if( test.second.second < best ){
					best = test.second.second;
					out = test;
					}
				}
			
			}
		}
	return out;
	}

std::vector<uint64_t> Graph :: find_shortest_path( std::string nameA, std::string nameB ){
	
	std::map<std::string, uint64_t>::iterator it0, it1;
	
	it0 = node_map.find( nameA );
	it1 = node_map.find( nameB );
	
	if( it0 == node_map.end() || it1 == node_map.end() ){ return std::vector<uint64_t>(); }
	
	return find_shortest_path( it0->second, it1->second );
	
	}

std::vector<uint64_t> Graph :: find_shortest_path( uint64_t source, uint64_t target ){
	
	std::set<uint64_t> *visited = new std::set<uint64_t>();
	std::vector<uint64_t> path;
	
	if( node_storage->fetch_node( source )->is_connected( target ) ){
		path.push_back( source );
		path.push_back( target );
		return path;
		}
	
	std::pair< bool, std::pair< std::vector<uint64_t>, double > > result = _dfs_shortest( source, target, path, 0, visited );
	
	delete visited;
	
	if( result.first ){
		path.push_back( source );
		path.insert( path.end(), result.second.first.begin(), result.second.first.end() );
		return path;
		}
	
	return path;
	}


std::pair< bool, std::pair< std::vector<uint64_t>, double > > Graph :: _dfs_longest( 	uint64_t current, 
																						uint64_t target, 
																						std::vector<uint64_t> &path, 
																						double dist,
																						std::set<uint64_t> *visited ){
	

	if( visited->find( current ) != visited->end() ){
		return std::make_pair(false, std::make_pair( std::vector<uint64_t>(), -1 ) );
		}
	
	visited->insert( current );
	
	Node *node = node_storage->fetch_node( current );
	
	std::pair< bool, std::pair< std::vector<uint64_t>, double > > out, test;
	
	out = std::make_pair( false, std::make_pair( std::vector<uint64_t>(), -1 ) );
	
	double best = -1;
	
	if( node->is_connected( target ) ){
		//dist += node->get_weight( target );
		std::vector<uint64_t> new_path = path;
		new_path.push_back( target );
		out = std::make_pair( true, std::make_pair( new_path, dist + node->get_weight( target ) ) );
		best = dist + node->get_weight( target );
		}
	
	std::vector<uint64_t> conns = node->get_connections();
	
	
	
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		std::vector<uint64_t> new_path = path;
		new_path.push_back( conns[i] );
		test = _dfs_longest( conns[i], target, new_path, dist + node->get_weight( conns[i] ), visited );
		
		if( test.first ){
			if( test.second.second > best ){
				best = test.second.second;
				out = test;
				}
			}
		}
	return out;
	}

std::vector<uint64_t> Graph :: find_longest_path( std::string nameA, std::string nameB ){
	
	std::map<std::string, uint64_t>::iterator it0, it1;
	
	it0 = node_map.find( nameA );
	it1 = node_map.find( nameB );
	
	if( it0 == node_map.end() || it1 == node_map.end() ){ return std::vector<uint64_t>(); }
	
	return find_longest_path( it0->second, it1->second );
	
	}

std::vector<uint64_t> Graph :: find_longest_path( uint64_t source, uint64_t target ){
	
	std::set<uint64_t> *visited = new std::set<uint64_t>();
	std::vector<uint64_t> path;
	
	if( node_storage->fetch_node( source )->is_connected( target ) ){
		path.push_back( source );
		path.push_back( target );
		return path;
		}
	
	std::pair< bool, std::pair< std::vector<uint64_t>, double > > result = _dfs_longest( source, target, path, 0, visited );
	
	delete visited;
	
	if( result.first ){
		path.push_back( source );
		path.insert( path.end(), result.second.first.begin(), result.second.first.end() );
		return path;
		}
	
	return path;
	}


void Graph :: _dfs_nearest( uint64_t current, double dist, std::set<uint64_t> * visited ){
	
	if( visited->find( current ) != visited->end() ){
		return;
		}
	
	visited->insert( current );
	
	Node *node = node_storage->fetch_node( current );
	
	std::vector<uint64_t> conns = node->get_connections();
	
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		double d = dist - node->get_weight( conns[i] );
		if( d >= 0 ){
			_dfs_nearest( conns[i], d, visited );
			}
		
		}
	
	}
		

std::vector<uint64_t> Graph :: find_nearest_nodes( std::string nameA, double dist ){
	
	std::map<std::string, uint64_t>::iterator it0;
	
	it0 = node_map.find( nameA );
	
	if( it0 == node_map.end() ){ return std::vector<uint64_t>(); }
	
	return find_nearest_nodes( it0->second, dist );
	
	}

std::vector<uint64_t> Graph :: find_nearest_nodes( uint64_t source, double dist ){
	
	std::set<uint64_t> *visited = new std::set<uint64_t>();
	std::vector<uint64_t> out;
	
	_dfs_nearest( source, dist, visited );
	
	for( std::set<uint64_t>::iterator it = visited->begin(); it != visited->end() ; it++ ){
		out.push_back( *it );
		}
	
	delete visited;
	
	return out;
	}

void Graph :: _dfs_connected( uint64_t current, std::set<uint64_t> * visited ){
	
	if( visited->find( current ) != visited->end() ){
		return;
		}
	
	visited->insert( current );
	
	Node *node = node_storage->fetch_node( current );
	
	std::vector<uint64_t> conns = node->get_connections();
	
	for( size_t i = 0 ; i < conns.size() ; ++i ){
		_dfs_connected( conns[i], visited );
		}
	
	}

std::vector<uint64_t> Graph :: find_connected_nodes( std::string nameA ){
	
	std::map<std::string, uint64_t>::iterator it0;
	
	it0 = node_map.find( nameA );
	
	if( it0 == node_map.end() ){ return std::vector<uint64_t>(); }
	
	return find_connected_nodes( it0->second );
	
	}

std::vector<uint64_t> Graph :: find_connected_nodes( uint64_t source ){
	
	std::set<uint64_t> *visited = new std::set<uint64_t>();
	std::vector<uint64_t> out;
	
	_dfs_connected( source, visited );
	
	for( std::set<uint64_t>::iterator it = visited->begin(); it != visited->end() ; it++ ){
		out.push_back( *it );
		}
	
	delete visited;
	
	return out;
	}

std::vector<uint64_t> Graph :: filter_by_property( std::vector<uint64_t> nodes, std::string key, std::string value, std::string op ){
	
	std::vector<uint64_t> out;
	Node *node;
	std::string test;
	
	for( size_t i = 0 ; i < nodes.size() ; ++i ){
		node = node_storage->fetch_node( nodes[i] );
		test = node->get_property( key );
		
		if( op == "=" ){
			if( test == value ){ out.push_back( nodes[i] ); }
			}
		if( op == "!=" ){
			if( test != value ){ out.push_back( nodes[i] ); }
			}
		
		if( op == "in" ){
			if( test.find( value ) != std::string::npos ){
				out.push_back( nodes[i] );
				}
			} 
		if( op == "not in" ){
			if( test.find( value ) == std::string::npos ){
				out.push_back( nodes[i] );
				}
			}
		
		}
	return out;
	}
		

std::vector<uint64_t> Graph :: compute_union( std::vector<uint64_t> nodesA, std::vector<uint64_t> nodesB ){
	std::vector<uint64_t> out;
	std::set<uint64_t> out_set;
	
	for( size_t i = 0 ; i < nodesA.size() ; ++i ){
		out_set.insert( nodesA[i] );
		}
	
	for( size_t i = 0 ; i < nodesB.size() ; ++i ){
		out_set.insert( nodesB[i] );
		}
	
	for( std::set<uint64_t>::iterator it = out_set.begin() ; it != out_set.end() ; it++ ){
		out.push_back( *it );
		}
		
	return out;
	}

std::vector<uint64_t> Graph :: compute_intersection( std::vector<uint64_t> nodesA, std::vector<uint64_t> nodesB ){
	std::vector<uint64_t> out;
	std::set<uint64_t> out_set;
	
	for( size_t i = 0 ; i < nodesA.size() ; ++i ){
		out_set.insert( nodesA[i] );
		}
	
	for( size_t i = 0 ; i < nodesB.size() ; ++i ){
		if( out_set.find( nodesB[i] ) != out_set.end() ){
			out.push_back( nodesB[i] );
			}
		}
		
	return out;
	}

std::vector<uint64_t> Graph :: compute_complement( std::vector<uint64_t> nodesA, std::vector<uint64_t> nodesB ){
	std::vector<uint64_t> out;
	std::set<uint64_t> out_set;
	
	for( size_t i = 0 ; i < nodesA.size() ; ++i ){
		out_set.insert( nodesA[i] );
		}
	
	for( size_t i = 0 ; i < nodesB.size() ; ++i ){
		if( out_set.find( nodesB[i] ) == out_set.end() ){
			out.push_back( nodesB[i] );
			}
		}
	
		
	return out;
	}


void Graph :: restore_from_storage(){
	
	std::vector<std::string> values = node_storage->get_values();
	
	//std::cout << "values.size() = " << values.size() << std::endl;
	
	for( size_t i = 0 ; i < values.size() ; ++i ){
		Node *node = Node::deserialize( values[i] );
		node_storage->insert_node( node );
		node_map[ node->get_name() ] = node->get_id();
		}
	
	}


std::vector< std::pair<uint64_t, uint64_t> > Graph :: colour_graph(){
	std::map<uint64_t, uint64_t> colours;
	std::vector<uint64_t> nodes = get_nodes();
	
	std::vector< std::pair<uint64_t, uint64_t> > out;
	
	for( size_t i = 0 ; i < nodes.size() ; ++i ){
		colours[ nodes[i] ] = 0;
		}
	
	for( size_t i = 0 ; i < nodes.size() ; ++i ){
		Node *node = node_storage->fetch_node( nodes[i] );
		//std::cout << "Node " << node->get_name() << std::endl;
		std::vector<uint64_t> conns = node->get_connections();
		std::set<uint64_t> adjecent;
		for( size_t j = 0 ; j < conns.size() ; ++j ){
			//std::cout << "colours = " << colours[conns[j]] << std::endl;
			adjecent.insert( colours[ conns[j] ] ); 
			}
		
		uint64_t maxL = *(adjecent.rbegin()) + 2;
		//std::cout << "maxL " << maxL << std::endl;
		for( size_t j = 1 ; j < maxL ; ++j ){
			if( adjecent.find( j ) == adjecent.end() ){
				colours[ nodes[i] ] = j;
				break;
				}
			}
		//std::cout << std::endl << std::endl;
		}
	
	for( std::map<uint64_t,uint64_t>::iterator it = colours.begin() ; it != colours.end() ; it++ ){
		out.push_back( std::make_pair( it->first, it->second ) );
		}
	
	return out;
	}

double Graph :: compute_path_lenght( std::vector<uint64_t> path ){
	double out = 0;
	for( size_t i = 0 ; i < path.size()-1 ; ++i ){
		out += node_storage->fetch_node( path[i] )->get_weight( path[i+1] );
		}
	return out;
	}

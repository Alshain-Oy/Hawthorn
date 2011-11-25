#include <iostream>
#include <libgraph.hpp>

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}


int main( int argc, char **argv ){
	
	NodeStorage store( 0xFFFF );
	Graph graph( &store );
	

	graph.create_node( "A" );
	graph.create_node( "B" );
	graph.create_node( "C" );
	graph.create_node( "D" );
	graph.create_node( "E" );
	graph.create_node( "F" );
	graph.create_node( "G" );
	
	std::vector<std::pair<std::string, std::string> > conns;
	
	conns.push_back( std::make_pair( "A", "B" ) );
	conns.push_back( std::make_pair( "A", "C" ) );
	conns.push_back( std::make_pair( "B", "C" ) );
	
	conns.push_back( std::make_pair( "B", "E" ) );
	conns.push_back( std::make_pair( "B", "D" ) );
	
	conns.push_back( std::make_pair( "D", "E" ) );
	conns.push_back( std::make_pair( "D", "G" ) );
	
	conns.push_back( std::make_pair( "G", "E" ) );
	conns.push_back( std::make_pair( "G", "F" ) );

	conns.push_back( std::make_pair( "E", "F" ) );

	for( size_t i = 0 ; i < conns.size() ; ++i ){
		graph.connect_nodes( conns[i].first, conns[i].second, 1.0 );
		graph.connect_nodes( conns[i].second, conns[i].first, 1.0 );
		}


	std::vector< std::pair<uint64_t, uint64_t> > cols = graph.colour_graph();
	
	for( size_t i = 0 ; i < cols.size() ; ++i ){
		std::cout << graph.get_node( cols[i].first )->get_name() << ": " << cols[i].second << std::endl;
		}
	
	return 0;
	}

